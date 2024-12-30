# Netty 开发 Simple 服务端&客户端应用程序      

>该案例完成的是客户端与服务端建立通信           
![connect01](http://img.xinzhuxiansheng.com/blogimgs/netty/connect01.png)       

## 使用Netty来启动一个服务端应用程序    

### 服务端启动最小化代码    
```java
public class NettyServer {
    public static void main(String[] args) {
        NioEventLoopGroup bossGroup = new NioEventLoopGroup();
        NioEventLoopGroup workerGroup = new NioEventLoopGroup();

        ServerBootstrap serverBootstrap = new ServerBootstrap();
        serverBootstrap
                .group(bossGroup,workerGroup)
                .channel(NioServerSocketChannel.class)
                .childHandler(new ChannelInitializer<NioSocketChannel>(){
                    @Override
                    protected void initChannel(NioSocketChannel nioSocketChannel) throws Exception {

                    }
                });

        serverBootstrap.bind(8000);
    }
}   
```

* 上述代码首先创建了`两个NioEventLoopGroup`，这两个对象可以看作传统IO编程模型的两大线程组，`bossGroup表示监听端口，接收新连接的线程组；workerGroup表示处理每一个连接的数据读写的线程组`。用生活中的例子来讲就是，一个工厂要运作，必然要有一个老板负责从外面接活，然后有很多员工，负责具体干活。老板就是bossGroup，员工们就是workerGroup，bossGroup接收完连接，交给workerGroup去处理。         

* 其次创建了一个引导类ServerBootstrap，这个类将引导服务端的启动工作。     

* 通过.group(bossGroup,workerGroup)给引导类配置两大线程组，这个引导类的线程模型也就定型了。     

* 然后指定服务端的IO模型为`NIO`，上述代码通过`.channel(NioServerSocketChannel.class)`来指定IO模型，也可以有其他选择。`如果你想指定IO模型为BIO，那么这里配置上OioServerSocketChannel.class类型即可`。当然通常我们也不会这么做，因为Netty的优势就在于NIO。    

* 接着调用childHandler()方法，给这个引导类创建一个ChannelInitializer，主要是定义后续每个连接的数据读写，对于业务处理逻辑，不理解也没关系，后面我们会详细分析。在ChannelInitializer这个类中，有一个泛型参数NioSocketChannel，这个类就是Netty对NIO类型连接的抽象，而前面的NioServerSocketChannel也是对NIO类型连接的抽象，NioServerSocketChannel和NioSocketChannel的概念可以与BIO编程模型中的ServerSocket和Socket两个概念对应。             

最小化参数配置到这里就完成了，总结一下就是，要启动一个Netty服务端，必须要指定三类属性，分别是线程模型、IO模型、连接读写处理逻辑。有了这三者，之后再调用bind(8000)，就可以在本地绑定一个8000端口启动服务端。以上这段代码，读者可以直接复制到自己的IDE中运行。              

### 自动绑定递增端口    
上面代码绑定了8000端口，接下来我们实现一个稍微复杂点的逻辑。我们指定一个起始端口号，比如1000；然后从1000端口往上找一个端口，直到这个端口能够绑定成功。比如1000端口不可用，我们就尝试绑定1001端口，然后1002端口，以此类推。  

serverBootstrap.bind(8000)方法是一个`异步方法`，调用之后是立即返回的，`它的返回值是一个ChannelFuture`。我们可以给这个ChannelFuture添加一个`监听器GenericFutureListener`，然后在GenericFutureListener的operationComplete方法里，监听端口是否绑定成功。下面是监听端口是否绑定成功的代码片段。     
```java
serverBootstrap.bind(8000).addListener(new GenericFutureListener<Future<? super Void>>() {
    @Override
    public void operationComplete(Future<? super Void> future) throws Exception {
        if (future.isSuccess()) {
            System.out.println("端口绑定成功！ ");
        } else {
            System.out.println("端口绑定失败！");
        }
    }
});
```

接下来就可以从1000端口开始，往上找端口号，直到端口绑定成功。我们要做的就是在if(future.isSuccess())的else逻辑里重新绑定一个递增的端口。我们从这段绑定逻辑中抽取出一个bind方法。  
```java
private static void bind(final ServerBootstrap serverBootstrap, final int port) {
    serverBootstrap.bind(port).addListener(new GenericFutureListener<Future<? super Void>>() {
        @Override
        public void operationComplete(Future<? super Void> future) throws Exception {
            if (future.isSuccess()) {
                System.out.println("端口[" + port + "]绑定成功！ ");
            } else {
                System.out.println("端口[" + port + "]绑定失败！ ");
                bind(serverBootstrap, port + 1);
            }
        }
    });
}
``` 
上述代码中最关键的就是在端口绑定失败之后，重新调用自身方法，并且把端口号加一，这样，在我们的主流程里面就可以直接调用。      
```java
bind(serverBootstrap, 1000)
```
读者可以自行修改代码，运行之后看到效果，最终会发现，端口成功绑定在了1024。从1000开始到1023，端口均绑定失败，这是因为在笔者的Mac系统下，1023以下的端口号都被系统保留了，需要ROOT权限才能绑定。   

以上就是自动绑定递增端口的逻辑。接下来，我们一起学习一下，服务端启动引导类ServerBootstrap除了指定线程模型、IO模型、连接读写处理逻辑，还可以做哪些事情？ 

### 服务端启动的其他方法    

#### handler()  
```java
serverBootstrap.handler(new ChannelInitializer<NioServerSocketChannel>() {
                    @Override
                    protected void initChannel(NioServerSocketChannel nioServerSocketChannel) throws Exception {
                        System.out.println("服务端启动中");
                    }
                });
```
handler()方法可以和前面分析的childHandler()方法对应起来：`childHandler()方法用于指定处理新连接数据的读写处理逻辑；handler()方法用于指定在服务端启动过程中的一些逻辑，通常情况下用不到这个方法`      

#### attr() 
```java
serverBootstrap.attr(AttributeKey.newInstance("serverName"), "nettyServer")
``` 
attr()方法可以给服务端Channel，也就是NioServerSocketChannel指定一些自定义属性，然后通过channel.attr()取出这个属性。比如，上面的代码可以指定服务端Channel的serverName属性，属性值为nettyServer，其实就是给NioServerSocketChannel维护一个Map而已，通常情况下也用不上这个方法。        

#### childAttr()    
除了可以给服务端Channel即NioServerSocketChannel指定一些自定义属性，我们还可以给每一个连接都指定自定义属性。 
```java
serverBootstrap.childAttr(AttributeKey.newInstance("clientKey"), "clientValue")
```
上面的childAttr()方法可以给每一个连接都指定自定义属性，后续我们可以通过channel.attr()方法取出该属性。   

#### option()   
option()方法可以给服务端Channel设置一些TCP参数，最常见的就是so_backlog，设置如下。  
```java
serverBootstrap.option(ChannelOption.SO_BACKLOG, 1024)  
``` 
这个设置表示系统用于临时存放已完成三次握手的请求的队列的最大长度，如果连接建立频繁，服务器处理创建新连接较慢，则可以适当调大这个参数。  

#### childOption()
childOption()方法可以给每个连接都设置一些TCP参数    
```java
serverBootstrap       
    .childOption(ChannelOption.SO_KEEPALIVE, true)       
    .childOption(ChannelOption.TCP_NODELAY, true)   
```
上述代码中设置了两种TCP参数，其中： 
* ChannelOption.SO_KEEPALIVE表示是否开启TCP底层心跳机制，true表示开启。 
* ChannelOption.TCP_NODELAY表示是否开启Nagle算法，true表示关闭，false表示开启。通俗地说，如果要求高实时性，有数据发送时就马上发送，就设置为关闭；如果需要减少发送次数，减少网络交互，就设置为开启。     

> 以上列举仅仅是一小部分。  


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
2.https://netty.io/wiki/user-guide-for-4.x.html     
3.https://netty.io/4.1/api/index.html 搜索`ServerBootstrap` 
