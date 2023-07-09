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

refer   
1.《跟闪电侠学Netty：Netty即时聊天实战与底层原理》  
2.https://netty.io/wiki/user-guide-for-4.x.html     
3.https://netty.io/4.1/api/index.html 搜索`ServerBootstrap` 






