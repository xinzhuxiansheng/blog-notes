# Netty - RocketMQ - 探索异步转同步的实现细节    

>涉及到 RocketMQ version: 5.3.1 & version：4.9.5 

## 背景  
Java 语言开发的中间件组件，它们绝大多数使用 Netty 开发`NIO`网络通信模块，`异步事件驱动是它带来的高效特性`，在部分场景下希望业务代码调用 Netty Client 发送请求后，一直等待返回结果再继续处理后续逻辑。这对于 Netty 来说需要一些特殊处理，因为业务代码与 Netty 处理请求不在同一个线程。这点可以通过打印各自的线程名称来证实这点, 我们需要通过一些中间 cache 来关联他们。         

**等待返回结果**  
![async2sync01](http://img.xinzhuxiansheng.com/blogimgs/netty/async2sync01.png)     

![async2sync02](http://img.xinzhuxiansheng.com/blogimgs/netty/async2sync02.png)  

**打印业务线程与handler线程 名称**  
```bash
NettyClient 当前线程名称: main
Handler 当前线程名称: nioEventLoopGroup-2-1   
```

## 思考 & 案例实现    
我先拿案例来说明思路，后续会对比 RocketMQ version 4.9.5 与 version 5.3.1 的实现。    

>注意：下面的代码是`背景` 章节图片中的 `NettyClient` 源码：        
```java  
public void doRegistry() {
    RegistryReqDTO registryDTO = new RegistryReqDTO();
    try {
        System.out.println("NettyClient 当前线程名称: " + Thread.currentThread().getName());
        String msgId = UUID.randomUUID().toString();
        registryDTO.setBrokerIp(Inet4Address.getLocalHost().getHostAddress());
        registryDTO.setBrokerPort(5050);
        registryDTO.setUser("yzhou");
        registryDTO.setPassword("password");
        registryDTO.setMsgId(msgId);
        TcpMsg tcpMsg = new TcpMsg(ServerEventCode.REGISTRY.getCode(), JSON.toJSONBytes(registryDTO));

        TcpMsg respMsg = sendSyncMsg(tcpMsg, msgId);

        System.out.println("redistry resp: " + new String(respMsg.getBody()));
    } catch (UnknownHostException e) {
        throw new RuntimeException(e);
    }
}

public TcpMsg sendSyncMsg(TcpMsg tcpMsg, String msgId) {
    channel.writeAndFlush(tcpMsg);
    SyncFuture syncFuture = new SyncFuture();
    syncFuture.setMsgId(msgId);
    ServerSyncFutureManager.put(msgId, syncFuture);
    try {
        return (TcpMsg) syncFuture.get();
    } catch (Exception e) {
        throw new RuntimeException(e);
    }
}
```

`NettyClient#doRegistry()` 方法是我们注册节点信息的业务代码，RegistryReqDTO 对象是请求参数 `TcpMsg`中的 body 部分。 拼装好请求参数后，再调用 `NettyClient#sendSyncMsg()`方法发起 Netty 请求，剩下的就是等待 response 。    

`NettyClient#sendSyncMsg()`方法中 `channel.writeAndFlush(tcpMsg);` 负责 Netty Client 发起请求，它的所有处理都要经过 `Netty Channel Pipeline`, 下面是它的定义，`ServerRespChannelHandler` 会对 Response 进行处理。        
```java
bootstrap.handler(new ChannelInitializer<SocketChannel>() {
    @Override
    protected void initChannel(SocketChannel ch) throws Exception {
        ch.pipeline().addLast(new TcpMsgDecoder());
        ch.pipeline().addLast(new TcpMsgEncoder());
        ch.pipeline().addLast(new ServerRespChannelHandler());
    }
});
```

**ServerRespChannelHandler**            
```java
@ChannelHandler.Sharable
public class ServerRespChannelHandler extends SimpleChannelInboundHandler {

    @Override
    protected void channelRead0(ChannelHandlerContext channelHandlerContext, Object msg) throws Exception {

        System.out.println("Handler 当前线程名称: " + Thread.currentThread().getName());

        TcpMsg tcpMsg = (TcpMsg) msg;
        System.out.println("resp:" + JSON.toJSONString(tcpMsg));
        if (ServerResponseCode.REGISTRY_SUCCESS.getCode() == tcpMsg.getCode()) {
            RegistryResponseDTO registryResponseDTO = com.alibaba.fastjson2.JSON.parseObject(tcpMsg.getBody(), RegistryResponseDTO.class);
            SyncFuture syncFuture = ServerSyncFutureManager.get(registryResponseDTO.getMsgId());
            if (syncFuture != null) {
                syncFuture.setResponse(tcpMsg);
            }
        }
    }
}
```

在上面我们已说明 `NettyClient#doRegistry()` 与 `ServerRespChannelHandler#channelRead0()` 分别在不同的线程中执行。下面是我们要解决的。              

### 我们要解决的两个核心问题是：       
**1.** `NettyClient#doRegistry()`方法所在的线程如何获取 `ServerRespChannelHandler#channelRead0()`所在线程的返回值 ？？？    

**2.** `NettyClient#doRegistry()` 调用 `NettyClient#sendSyncMsg()` 方法后如何阻塞等待当前线程，又如何被唤醒 ？？？            


### 实现思路              
![async2sync03](http://img.xinzhuxiansheng.com/blogimgs/netty/async2sync03.png)   

**1.** 使用公共 Hash Table 存储 `ServerRespChannelHandler#channelRead0()`所在线程的返回值，那 `NettyClient#doRegistry()`方法所在的线程就可以通过 Value 值获取提前设置好的返回值。            

>这里需要特别注意，`msgId`既是请求参数的一部分，当 server 接收请求，处理业务再返回数据时，同样也需要将 msgId 赋值给 response，只有这样我们才能在 Hash Table 中获取到 Value 值部分。  

**2.** 使用 `CountDownLatch countDownLatch = new CountDownLatch(1)` 来阻塞业务线程， countDownLatch 对象是 Hash Table Value 值的一部分， `ServerRespChannelHandler#channelRead0()`所在线程处理完返回值，赋值给 Hash Table Value，在将 Value 值中的 countDownLatch 中的计数器 -1。这样就会唤醒 `NettyClient#doRegistry()`所在线程， 这样就实现了 `异步转同步`的效果。   

下面看下 SyncFuture.java 的具体实现。  
`SyncFuture` 实现 Future 接口，重写了 get() 方法, 在 java `CompletableFuture`异步类中 get() 是一个阻塞等待返回结果方法， 而 SyncFuture 为了实现 get() 阻塞，在创建对象时，会创建一个计数器为 1的 CountDownLatch 对象，在 get()方法中调用 `countDownLatch.await()`方法，那当前线程会处理等待状态。  

回到 `NettyClient#sendSyncMsg()` 方法，创建 SyncFuture 对象，以 msgId 为 key，syncFuture 为 value，放入全局 Hash Table 中， 当`ServerRespChannelHandler#channelRead0()`处理返回值，根据返回值中 msgId 从全局 Hash Table 获取 SyncFuture，再赋值给它的属性 `response`, 此时就完成了返回值的存储。  
```java
if (syncFuture != null) {
    syncFuture.setResponse(tcpMsg);
}
```   

>注意，SyncFuture#setResponse(tcpMsg) 方法并不是默认的 set()方法，其内部调用`countDownLatch.countDown()` 将计数器置位0，则业务线程中的 `return (TcpMsg) syncFuture.get(); `会被唤醒，则继续完成后续处理逻辑。 


**SyncFuture.java**
```java
public class SyncFuture implements Future {

    private Object response;
    private String msgId;

    private CountDownLatch countDownLatch = new CountDownLatch(1);

    public String getMsgId() {
        return msgId;
    }

    public void setMsgId(String msgId) {
        this.msgId = msgId;
    }

    public Object getResponse() {
        return response;
    }

    public void setResponse(Object response) {
        this.response = response;
        countDownLatch.countDown();
    }

    @Override
    public boolean cancel(boolean mayInterruptIfRunning) {
        return false;
    }

    @Override
    public boolean isCancelled() {
        return false;
    }

    @Override
    public boolean isDone() {
        return response != null;
    }

    @Override
    public Object get() throws InterruptedException, ExecutionException {
        countDownLatch.await();
        ServerSyncFutureManager.remove(msgId);
        return response;
    }

    @Override
    public Object get(long timeout, TimeUnit unit) throws InterruptedException, ExecutionException, TimeoutException {
        try {
            countDownLatch.await(timeout,unit);
        } catch (Exception e) {
            throw new RuntimeException(e);
        } finally {
            ServerSyncFutureManager.remove(msgId);
        }
        return response;
    }
}
```  

`ServerSyncFutureManager` 是 Hash Table 的管理类。 
**ServerSyncFutureManager**
```java
public class ServerSyncFutureManager {

    private static Map<String, SyncFuture> syncFutureMap = new ConcurrentHashMap<>();

    public static void put(String key, SyncFuture syncFuture) {
        syncFutureMap.put(key, syncFuture);
    }

    public static SyncFuture get(String key) {
        return syncFutureMap.get(key);
    }

    public static void remove(String key) {
        syncFutureMap.remove(key);
    }
}
```

>注意，syncFutureMap 是 Hash Table 结构，每次调用都会插入一条 K/V 数据，当请求处理完后，我们要时刻注意它是否 remove 掉，（这里的处理完，代表的是正常情况，异常情况等）。    

完整的案例代码： `https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-netty/src/main/java/com/javamain/netty/protocol`   

## RocketMQ 关于 Netty 异步转同步的实现细节  
RocketMQ 源码中的 remoting 是负责网络通信的模块，它几乎被其他所有需要网络通信的模块所依赖和引用。 为了实现客户端与服务器之间高效的数据请求与接收， RocketMQ 消息队列自定义了通信协议并在 Netty 的基础之上扩展了通信模块。今天我们要介绍的``NettyRemotingClient`,`NettyRemotingAbstract` 就出自 remoting 模块中。    

先从 RocketMQ version 4.9.5 开始介绍，这是因为在 RocketMQ `issue#7321`（https://github.com/apache/rocketmq/pull/7322）中 使用 `CompletableFuture` + `Callback Function` 方案替换了 `CountDownLatch`。   

有了上面案例代码的实现细节，在 RocketMQ 找到实现因素特别得心应手。   


## RocketMQ version 4.9.5 实现  

### NettyRemotingAbstract#invokeSyncImpl()  
在 `NettyRemotingAbstract#invokeSyncImpl()` 方法中，int opaque 是 msgId,  responseTable 是用来在两个不同线程之间传递值的全局 Hash Table， 它的结构如下：        
```java
/**
 * This map caches all on-going requests.
 */
protected final ConcurrentMap<Integer /* opaque */, ResponseFuture> responseTable =
    new ConcurrentHashMap<Integer, ResponseFuture>(256);
``` 

创建 ResponseFuture 对象，它包含 opaque 字段，调用 channel.writeAndFlush() 发送消息后，再调用 `RemotingCommand responseCommand = responseFuture.waitResponse(timeoutMillis);` 来阻塞当前线程，`waitResponse()`的代码如下, 它使用 CountDownLatch#await() 来挂起线程，这与我们的案例实现是一模一样的。        
```java
public RemotingCommand waitResponse(final long timeoutMillis) throws InterruptedException {
    this.countDownLatch.await(timeoutMillis, TimeUnit.MILLISECONDS);
    return this.responseCommand;
}
``` 

```java
public RemotingCommand invokeSyncImpl(final Channel channel, final RemotingCommand request,
    final long timeoutMillis)
    throws InterruptedException, RemotingSendRequestException, RemotingTimeoutException {
    final int opaque = request.getOpaque();  // 1. msgId 是 请求参数的一部分

    try {
        final ResponseFuture responseFuture = new ResponseFuture(channel, opaque, timeoutMillis, null, null);
        this.responseTable.put(opaque, responseFuture); // 2. 放入 Hash Table 中
        final SocketAddress addr = channel.remoteAddress();
        channel.writeAndFlush(request).addListener(new ChannelFutureListener() {
            @Override
            public void operationComplete(ChannelFuture f) throws Exception {
                if (f.isSuccess()) {
                    responseFuture.setSendRequestOK(true);
                    return;
                } else {
                    responseFuture.setSendRequestOK(false);
                }

                responseTable.remove(opaque);
                responseFuture.setCause(f.cause());
                responseFuture.putResponse(null);
                log.warn("send a request command to channel <" + addr + "> failed.");
            }
        });

        RemotingCommand responseCommand = responseFuture.waitResponse(timeoutMillis);  // 3. 调用 countDownLatch.await 阻塞当前线程
        if (null == responseCommand) {
            if (responseFuture.isSendRequestOK()) {
                throw new RemotingTimeoutException(RemotingHelper.parseSocketAddressAddr(addr), timeoutMillis,
                    responseFuture.getCause());
            } else {
                throw new RemotingSendRequestException(RemotingHelper.parseSocketAddressAddr(addr), responseFuture.getCause());
            }
        }

        return responseCommand;
    } finally {
        this.responseTable.remove(opaque);
    }
}
```

我们看 `NettyClientHandler` 又是如何处理返回值和唤起业务线程的， 它是 `NettyRemotingClient`的内部类, 因为 `NettyRemotingClient`继承 `NettyRemotingAbstract`，所以`NettyClientHandler`最终可以调用到`org.apache.rocketmq.remoting.netty.NettyRemotingAbstract#processResponseCommand()` 方法，（注意，我们处理的事返回值）。     

在 `NettyRemotingAbstract#invokeSyncImpl()`方法中，并没有对 `ResponseFuture responseFuture` 对象设置 回调函数，所以它调用的是 putResponse()。在`putResponse()`方法调用 countDownLatch.countDown() 方法将返回值赋值给 responseFuture ，再将计数器置为0后， 这两个条件满足了业务代码继续向下执行的条件，这样就完成了 `异步转同步的实现`。  在 version 4.9.5 的实现逻辑与案例是一模一样。   

```java
public void putResponse(final RemotingCommand responseCommand) {
    this.responseCommand = responseCommand;
    this.countDownLatch.countDown();
}
```

```java
/**
 * Process response from remote peer to the previous issued requests.
 *
 * @param ctx channel handler context.
 * @param cmd response command instance.
 */
public void processResponseCommand(ChannelHandlerContext ctx, RemotingCommand cmd) {
    final int opaque = cmd.getOpaque();
    final ResponseFuture responseFuture = responseTable.get(opaque);
    if (responseFuture != null) {
        responseFuture.setResponseCommand(cmd);

        responseTable.remove(opaque);

        if (responseFuture.getInvokeCallback() != null) {
            executeInvokeCallback(responseFuture);
        } else {
            responseFuture.putResponse(cmd);
            responseFuture.release();
        }
    } else {
        log.warn("receive response, but not matched any request, " + RemotingHelper.parseChannelRemoteAddr(ctx.channel()));
        log.warn(cmd.toString());
    }
}
```   
  
## RocketMQ version 5.3.1 实现       
![async2sync04](http://img.xinzhuxiansheng.com/blogimgs/netty/async2sync04.png)  
`NettyRemotingAbstract#invoke0()`方法返回的是`CompletableFuture<ResponseFuture> future = new CompletableFuture<>();`, `responseTable` 仍然是全局的 Hash Table，K，V 的取值都是没有变化，但是创建 ResponseFuture 对象时，它创建 InvokeCallback 匿名类，用于在 `NettyRemotingAbstract#processResponseCommand()` 执行它。在 InvokeCallback 匿名类中的 `operationSucceed()`方法，它将 future标记为完成。并且将 `responseFutureReference` 赋值给 future。 此时你需要在意的是以下 2点：            

**1.** `NettyRemotingAbstract#invoke0()` 返回值 future 与 responseFutureReference 是不同对象，而 responseTable Hash Table 存储的 responseFuture 对象会赋值给 `responseFutureReference`。 所以再执行它的回调 operationSucceed() 方法时会将 responseFuture 赋值给 future。     

**2.** `ResponseFuture#executeInvokeCallback()`的回调执行的内容如下, 那 future, responseFutureRefercence 又是如何在其他线程中引用它们的值呢。 这里引入 `effectively final` 概念， 若不清楚的话，必须增量。   

>在 Java 中，effectively final（有效最终）是一个变量特性的描述，指的是虽然变量没有用 final 关键字声明，但在它的生命周期内其值未被修改。也就是说，如果一个局部变量在初始化之后没有被赋予新的值，它就可以被视为“有效最终”的。     
 
在 Java 的 Lambda 表达式或匿名内部类中，变量必须是 effectively final（即 "行为上不变"）。但是如果你想 在 Lambda 里修改变量，你不能直接使用普通变量，而需要借助 AtomicReference。    

所以就解释了 `future`, `responseFutureRefercence` 之间的区别。     

```java
@Override
public void operationSucceed(RemotingCommand response) {
    future.complete(responseFutureReference.get());
}
```


```java
protected CompletableFuture<ResponseFuture> invoke0(final Channel channel, final RemotingCommand request,
    final long timeoutMillis) {
    CompletableFuture<ResponseFuture> future = new CompletableFuture<>();
    long beginStartTime = System.currentTimeMillis();
    final int opaque = request.getOpaque();

    boolean acquired;
    try {
        acquired = this.semaphoreAsync.tryAcquire(timeoutMillis, TimeUnit.MILLISECONDS);
    } catch (Throwable t) {
        future.completeExceptionally(t);
        return future;
    }
    if (acquired) {
        final SemaphoreReleaseOnlyOnce once = new SemaphoreReleaseOnlyOnce(this.semaphoreAsync);
        long costTime = System.currentTimeMillis() - beginStartTime;
        if (timeoutMillis < costTime) {
            once.release();
            future.completeExceptionally(new RemotingTimeoutException("invokeAsyncImpl call timeout"));
            return future;
        }

        AtomicReference<ResponseFuture> responseFutureReference = new AtomicReference<>();
        final ResponseFuture responseFuture = new ResponseFuture(channel, opaque, request, timeoutMillis - costTime,
            new InvokeCallback() {
                @Override
                public void operationComplete(ResponseFuture responseFuture) {

                }

                @Override
                public void operationSucceed(RemotingCommand response) {
                    future.complete(responseFutureReference.get());
                }

                @Override
                public void operationFail(Throwable throwable) {
                    future.completeExceptionally(throwable);
                }
            }, once);
        responseFutureReference.set(responseFuture);
        this.responseTable.put(opaque, responseFuture);
        try {
            channel.writeAndFlush(request).addListener((ChannelFutureListener) f -> {
                if (f.isSuccess()) {
                    responseFuture.setSendRequestOK(true);
                    return;
                }
                requestFail(opaque);
                log.warn("send a request command to channel <{}>, channelId={}, failed.", RemotingHelper.parseChannelRemoteAddr(channel), channel.id());
            });
            return future;
        } catch (Exception e) {
            responseTable.remove(opaque);
            responseFuture.release();
            log.warn("send a request command to channel <{}> channelId={} Exception", RemotingHelper.parseChannelRemoteAddr(channel), channel.id(), e);
            future.completeExceptionally(new RemotingSendRequestException(RemotingHelper.parseChannelRemoteAddr(channel), e));
            return future;
        }
    } else {
        if (timeoutMillis <= 0) {
            future.completeExceptionally(new RemotingTooMuchRequestException("invokeAsyncImpl invoke too fast"));
        } else {
            String info =
                String.format("invokeAsyncImpl tryAcquire semaphore timeout, %dms, waiting thread nums: %d semaphoreAsyncValue: %d",
                    timeoutMillis,
                    this.semaphoreAsync.getQueueLength(),
                    this.semaphoreAsync.availablePermits()
                );
            log.warn(info);
            future.completeExceptionally(new RemotingTimeoutException(info));
        }
        return future;
    }
}
```

针对 RocketMQ 中的异步转同步实现细节已介绍完成了，这部分最容易忽略的是 Hash Table的移除操作。所以这部分需要特别关注。   
