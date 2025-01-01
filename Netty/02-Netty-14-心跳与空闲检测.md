# Netty - 群聊消息的收发及Netty性能优化    

## 引言         
连接假死的现象是：在某一端（服务端或者客户端）看来，底层的TCP连接已经断开，但是应用程序并没有捕获到，因此会认为这条连接仍然是存在的。从TCP层面来说，只有收到四次握手数据包或者一个RST数据包，才表示连接的状态已断开。               
连接假死会带来以下两大问题。            
1.对于服务端来说，因为每个连接都会耗费CPU和内存资源，大量假死的连接会逐渐耗光服务器的资源，最终导致性能逐渐下降，程序崩溃。         
2.对于客户端来说，连接假死会造成发送数据超时，影响用户体验。        
通常，连接假死由以下几个原因造成。          
1.应用程序出现线程堵塞，无法进行数据的读写。        
2.客户端或者服务端网络相关的设备出现故障，比如网卡、机房故障。                  
3.公网丢包。公网环境相对内网而言，非常容易出现丢包、网络抖动等现象，如果在一段时间内用户接入的网络连续出现丢包现象，那么对客户端来说，数据一直发送不出去；而服务端也一直收不到客户端的数据，连接就一直耗着。            
如果应用程序是面向用户的，那么公网丢包这个问题出现的概率是非常高的。对于内网来说，内网丢包、抖动也会有一定概率发生。一旦出现此类问题，客户端和服务端都会受到影响。接下来，我们分别从服务端和客户端的角度来解决连接假死的问题。              


## 服务端空闲检测
对于服务端来说，客户端的连接如果出现假死，那么服务端将无法收到客户端的数据。也就是说，如果能一直收到客户端发来的数据，则说明这个连接还是活的。因此，服务端对于连接假死的应对策略就是空闲检测。          
何为空闲检测？空闲检测指的是每隔一段时间，检测这段时间内是否有数据读写。简化一下，服务端只需要检测一段时间内，是否收到过客户端发来的数据即可，Netty自带的IdleStateHandler就可以实现这个功能。               
首先，我们写一个继承自IdleStateHandler的类，来定义检测到假死连接之后的逻辑。                
**IMIdleStateHandler.java**         
```java
public class IMIdleStateHandler extends IdleStateHandler {

    private static final int READER_IDLE_TIME = 15;

    public IMIdleStateHandler() {
        super(READER_IDLE_TIME, 0, 0, TimeUnit.SECONDS);
    }

    @Override
    protected void channelIdle(ChannelHandlerContext ctx, IdleStateEvent evt) {
        System.out.println(READER_IDLE_TIME + "秒内未读到数据，关闭连接");
        ctx.channel().close();
    }
}
```

｝
1.我们观察一下IMIdleStateHandler的构造函数，它调用父类IdleStateHandler的构造函数，有四个参数，其中第一个参数是读空闲时间，指的是在这段时间内如果没有读到数据，就表示连接假死；第二个参数是写空闲时间，指的是在这段时间如果没有写数据，就表示连接假死；第三个参数是读写空闲时间，指的是在这段时间内如果没有产生数据读或者写，就表示连接假死，写空闲和读写空闲均为0；最后一个参数是时间单位，在这个例子中表示的是：如果15秒内没有读到数据，就表示连接假死。                   

2.连接假死之后会回调channelIdle()方法，我们在这个方法里打印消息，并手动关闭连接。               
然后，我们把这个Handler插到服务端Pipeline的最前面。         
**NettyServer**
```java
public class NettyServer {

    private static final int PORT = 8000;

    public static void main(String[] args) {
        NioEventLoopGroup boosGroup = new NioEventLoopGroup();
        NioEventLoopGroup workerGroup = new NioEventLoopGroup();

        final ServerBootstrap serverBootstrap = new ServerBootstrap();
        serverBootstrap
                .group(boosGroup, workerGroup)
                .channel(NioServerSocketChannel.class)
                .option(ChannelOption.SO_BACKLOG, 1024)
                .childOption(ChannelOption.SO_KEEPALIVE, true)
                .childOption(ChannelOption.TCP_NODELAY, true)
                .childHandler(new ChannelInitializer<NioSocketChannel>() {
                    protected void initChannel(NioSocketChannel ch) {
                        // 空闲检测
                        ch.pipeline().addLast(new IMIdleStateHandler());  // 新增
                        ch.pipeline().addLast(new Spliter());
                        ch.pipeline().addLast(PacketCodecHandler.INSTANCE);
                        ch.pipeline().addLast(LoginRequestHandler.INSTANCE);
                        ch.pipeline().addLast(HeartBeatRequestHandler.INSTANCE);
                        ch.pipeline().addLast(AuthHandler.INSTANCE);
                        ch.pipeline().addLast(IMHandler.INSTANCE);
                    }
                });


        bind(serverBootstrap, PORT);
    }

    private static void bind(final ServerBootstrap serverBootstrap, final int port) {
        serverBootstrap.bind(port).addListener(future -> {
            if (future.isSuccess()) {
                System.out.println(new Date() + ": 端口[" + port + "]绑定成功!");
            } else {
                System.err.println("端口[" + port + "]绑定失败!");
            }
        });
    }
}
```

为什么要插到最前面？是因为假如插到最后面，如果这个连接读到了数据，但是在inbound传播的过程中出错了或者数据处理完毕就不往后传递了（我们的应用程序属于这类），那么最终IMIdleStateHandler就不会读到数据，会导致误判。           
服务端的空闲检测完毕之后，我们再思考一下，在一段时间内没有读到客户端的数据，是否一定能判断连接假死呢？并不能，如果在这段时间内客户端确实没有发送数据过来，但是连接是正常的，那么这个时候服务端也不能关闭这个连接。为了防止服务端误判，我们还需要在客户端做点什么。          

## 客户端定时发心跳数据包
服务端在一段时间内没有收到客户端的数据，这个现象产生的原因可以分为以下两种。            
1.连接假死。            
2.非假死状态下确实没有发送数据。            
我们只需要排除第二种可能，那么连接自然就是假死的。要排查第二种情况，我们可以在客户端定期发送数据包到服务端，通常这个数据包被称为心跳数据包。我们定义一个Handler，定期发送心跳数据包给服务端。       
**HeartBeatTimerHandler.java**      
```java
public class HeartBeatTimerHandler extends ChannelInboundHandlerAdapter {

    private static final int HEARTBEAT_INTERVAL = 5;

    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        scheduleSendHeartBeat(ctx);

        super.channelActive(ctx);
    }

    private void scheduleSendHeartBeat(ChannelHandlerContext ctx) {
        ctx.executor().schedule(() -> {

            if (ctx.channel().isActive()) {
                ctx.writeAndFlush(new HeartBeatRequestPacket());
                scheduleSendHeartBeat(ctx);
            }

        }, HEARTBEAT_INTERVAL, TimeUnit.SECONDS);
    }
}
```

ctx.executor()方法返回的是当前Channel绑定的NIO线程。NIO线程有一个schedule()方法，类似JDK的延时任务机制，可以隔一段时间执行一个任务。这里实现了每隔5秒向服务端发送一个心跳数据包，这个间隔时间通常要比服务端的空闲检测时间的一半短一些，可以直接定义为空闲检测时间的三分之一，主要是为了排除公网偶发的秒级抖动。             
在实际生产环境中，发送心跳数据包间隔时间和空闲检测时间可以略长一些，设置为几分钟级别，具体应用可以具体对待，没有强制规定。          
上面我们其实解决了服务端的空闲检测问题，服务端这个时候能够在一定时间段内关掉假死的连接，释放连接的资源，但是对于客户端来说，我们也需要检测假死的连接。              

## 服务端回复心跳与客户端空闲检测
客户端的空闲检测其实和服务端一样，依旧是在客户端Pipeline的最前面插入IMIdleStateHandler。                
**NettyClient.java**            
```java
.handler(new ChannelInitializer<SocketChannel>() {
    @Override
    public void initChannel(SocketChannel ch) {
        // 空闲检测
        ch.pipeline().addLast(new IMIdleStateHandler());

        ch.pipeline().addLast(new Spliter());
        ch.pipeline().addLast(new PacketDecoder());
        // 登录响应处理器
        ch.pipeline().addLast(new LoginResponseHandler());
        // 收消息处理器
        ch.pipeline().addLast(new MessageResponseHandler());
        // 创建群响应处理器
        ch.pipeline().addLast(new CreateGroupResponseHandler());
        // 加群响应处理器
        ch.pipeline().addLast(new JoinGroupResponseHandler());
        // 退群响应处理器
        ch.pipeline().addLast(new QuitGroupResponseHandler());
        // 获取群成员响应处理器
        ch.pipeline().addLast(new ListGroupMembersResponseHandler());
        // 群消息响应
        ch.pipeline().addLast(new GroupMessageResponseHandler());
        // 登出响应处理器
        ch.pipeline().addLast(new LogoutResponseHandler());
        ch.pipeline().addLast(new PacketEncoder());

        // 心跳定时器
        ch.pipeline().addLast(new HeartBeatTimerHandler());
    }
});
```

为了排除因为服务端在非假死状态下确实没有发送数据的情况，服务端也要定期发送心跳数据包给客户端。              
其实在前面我们已经实现了客户端向服务端定期发送心跳数据包，服务端这边只要在收到心跳数据包之后回复客户端，给客户端发送一个心跳响应包即可。如果在一段时间内客户端没有收到服务端发来的数据包，则可以判定这个连接为假死状态。            
因此，服务端的Pipeline中需要再加上一个Handler—HeartBeatRequestHandler，由于这个Handler的处理是无须登录的，所以，我们将该Handler放置在AuthHandler前面。          

## 总结
1.首先讨论了连接假死相关的现象及产生的原因。                
2.要处理连接假死问题，首先要实现客户端与服务端定期发送心跳数-据包。在这里，其实服务端只需要对客户端的定时心跳数据包进行回复即可。               
3.客户端与服务端如果都需要检测连接假死，那么直接在Pipeline的最前面插入一个自定义IdleStateHandler，在channelIdle()方法里自定义连接假死之后的逻辑即可。           
4.通常空闲检测时间比发送心跳数据包的间隔时间的两倍要长一些，这也是为了排除偶发的公网抖动，防止误判。                