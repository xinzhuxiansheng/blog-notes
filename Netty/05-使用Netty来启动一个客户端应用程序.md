## 使用Netty来启动一个客户端应用程序    

### 客户端启动Demo  
对于客户端的启动来说，和服务端的启动类似，依然需要`线程模型、IO模型，以及IO业务处理逻辑三大参数`。下面我们来看一下客户端启动的标准流程。  
```java
public class NettyClient {
    public static void main(String[] args) {
        NioEventLoopGroup workerGroup = new NioEventLoopGroup();

        Bootstrap bootstrap = new Bootstrap();
        bootstrap
                // 指定线程模型
                .group(workerGroup)
                // 指定 IO 类型为 NIO
                .channel(NioSocketChannel.class)
                // IO 处理逻辑
                .handler(new ChannelInitializer<SocketChannel>() {
                    @Override
                    protected void initChannel(SocketChannel socketChannel) throws Exception {

                    }
                });

        bootstrap.connect("127.0.0.1",8000).addListener(future -> {
            if (future.isSuccess()){
                System.out.println("连接成功！");
            }else{
                System.out.println("连接失败！");
            }
        });
    }
}
```

从上面的代码可以看到，`客户端启动的引导类是Bootstrap`，负责启动客户端和连接服务端；而在`服务端启动的时候，这个引导类是ServerBootstrap`。引导类创建完成之后，客户端启动的流程如下。  

1.与服务端的启动一样，需要给它指定线程模型，驱动连接的数据读写，这个线程的概念可以和第1章中IOClient.java创建的线程联系起来。    

2.指定IO模型为NioSocketChannel，表示IO模型为NIO。当然，你可以设置IO模型为OioSocketChannel，但是通常不会这么做，因为Netty的优势在于NIO。 

3.给引导类指定一个Handler，主要定义连接的业务处理逻辑，不理解没关系，在后面会详细分析。 

4.配置完线程模型、IO模型、业务处理逻辑之后，调用connect方法进行连接，可以看到connect方法有两个参数，第一个参数可以填写IP或者域名，第二个参数填写端口号。由于connect方法返回的是一个Future，也就是说这个方法是异步的，通过addListener方法可以监听连接是否成功，进而打印连接信息。        

到了这里，一个客户端启动的Demo就完成了。    

### 失败重连    
在网络情况差的情况下，客户端第一次连接可能会连接失败，这个时候我们可能会尝试重连。重连的逻辑写在连接失败的逻辑块里。    
```java
bootstrap.connect("127.0.0.1",8000).addListener(future -> {
    if (future.isSuccess()){
        System.out.println("连接成功！");
    }else{
        System.out.println("连接失败！");
    }
});
```
在重连的时候，依然调用同样的逻辑。因此，我们把建立连接的逻辑先抽取出来，然后在重连的时候，递归调用自身。    
```java
private static void connect(Bootstrap bootstrap,String host,int port){
    bootstrap.connect("127.0.0.1",8000).addListener(future -> {
        if (future.isSuccess()){
            System.out.println("连接成功！");
        }else{
            System.out.println("连接失败！");
            connect(bootstrap,host,port);
        }
    });
}
```
上面这一段便是带有自动重连功能的逻辑，可以看到在连接失败的时候，会调用自身进行重连。    

**但是，在通常情况下，连接失败不会立即重连，而是通过一个指数退避的方式，比如每隔1秒、2秒、4秒、8秒，以2的幂次来建立连接，到达一定次数之后就放弃连接。接下来我们实现这段逻辑，默认重试5次。**    
```java
private static void connect(Bootstrap bootstrap, String host, int port, int retry) {
    bootstrap.connect(host, port).addListener(future -> {
        if (future.isSuccess()) {
            System.out.println("连接成功！");
        } else if (retry == 0) {
            System.out.println("重试次数已用完，放弃连接！");
        } else {
            // 第几次重连
            int order = (MAX_RETRY - retry) + 1;
            // 本次重连的间隔
            int delay = 1 << order;
            System.err.println(new Date() + ": 连接失败，第" + order + "次重连.....");
            bootstrap.config().group().schedule(() -> connect(bootstrap, host, port, retry - 1), delay, TimeUnit.SECONDS);
        }
    });
}
``` 

从上面的代码可以看到，通过判断连接是否成功及剩余的重试次数，分别执行不同的逻辑。

1.如果连接成功，则打印连接成功的消息。  
2.如果连接失败但重试次数已经用完，则放弃连接。  
3.如果连接失败但重试次数仍然没有用完，则计算下一次重连间隔delay，然后定期重连。 
在上面的代码中，我们看到，定时任务调用的是bootstrap.config().group().schedule()，其中bootstrap.config()这个方法返回的是BootstrapConfig，它是对Bootstrap配置参数的抽象，然后bootstrap.config().group()返回的就是我们在一开始配置的线程模型workerGroup，调用workerGroup的schedule方法即可实现定时任务逻辑。   

在schedule方法块里，前四个参数原封不动地传递，最后一个重试次数参数减掉1，就是下一次建立连接时的上下文信息。读者可以自行修改代码，更改到一个连接不上的服务端Host或者Port，查看控制台日志就可以看到5次重连日志。  

以上就是实现指数退避的客户端重连逻辑。接下来，我们一起学习一下，客户端启动过程中的引导类Bootstrap除了指定线程模型、IO模型、连接读写处理逻辑，还可以做哪些事情？     

### 客户端启动的其他方法    

#### attr() 
```java
bootstrap.attr(AttributeKey.newInstance("clientName"), "nettyClient")   
```
attr()方法可以为`客户端Channel也就是NioSocketChannel绑定自定义属性`，然后通过channel.attr()方法取出这个属性。比如，上面的代码可以指定客户端Channel的clientName属性，属性值为nettyClient，其实就是为NioSocketChannel维护一个Map而已。后续在NioSocketChannel通过参数传来传去的时候，就可以通过它来取出设置的属性，非常方便。    

#### option()
```java
Bootstrap.
  .option(ChannelOption.CONNECT_TIMEOUT_MILLIS, 5000)   
  .option(ChannelOption.SO_KEEPALIVE, true)  
  .option(ChannelOption.TCP_NODELAY, true)
```
option()方法可以为连接设置一些TCP底层相关的属性，比如上面的代码中，我们设置了3种TCP属性，其中：
* `ChannelOption.CONNECT_TIMEOUT_MILLIS`表示连接的超时时间，超过这个时间，如果仍未连接到服务端，则表示连接失败。    
* `ChannelOption.SO_KEEPALIVE`表示是否开启TCP底层心跳机制，true表示开启。   
* `ChannelOption.TCP_NODELAY`表示是否开始Nagle算法，true表示关闭，false表示开启。通俗地说，如果要求高实时性，有数据发送时就马上发送，就设置为true；如果需要减少发送次数，减少网络交互，就设置为false。  

其他参数这里就不一一讲解了  

refer   
1.《跟闪电侠学Netty：Netty即时聊天实战与底层原理》      
2.https://netty.io/4.1/api/index.html 搜索`Bootstrap`   
