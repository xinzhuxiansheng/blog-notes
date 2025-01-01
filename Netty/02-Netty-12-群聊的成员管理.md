# Netty - 群聊的成员管理

## 引言         
已经学习了如何创建群聊并通知群聊的各位成员。本章中我们来实现群成员管理，包括群的加入、退出和获取群成员列表等功能。     

## 群的加入

### 在控制台添加群加入命令处理器            
**JoinGroupConsoleCommand.java**        
```java
public class JoinGroupConsoleCommand implements ConsoleCommand {
    @Override
    public void exec(Scanner scanner, Channel channel) {
        JoinGroupRequestPacket joinGroupRequestPacket = new JoinGroupRequestPacket();

        System.out.print("输入 groupId，加入群聊：");
        String groupId = scanner.next();

        joinGroupRequestPacket.setGroupId(groupId);
        channel.writeAndFlush(joinGroupRequestPacket);
    }
}
```
我们在控制台先添加群加入命令处理器JoinGroupConsoleCommand。在这个处理器中，我们创建一个指令对象JoinGroupRequestPacket，填上群ID之后，将数据包发送至服务端。之后，我们将该控制台指令添加到ConsoleCommandManager。       
**ConsoleCommandManager.java**     
```java
public ConsoleCommandManager() {
    consoleCommandMap = new HashMap<>();
    consoleCommandMap.put("sendToUser", new SendToUserConsoleCommand());
    consoleCommandMap.put("logout", new LogoutConsoleCommand());
    consoleCommandMap.put("createGroup", new CreateGroupConsoleCommand());
    consoleCommandMap.put("joinGroup", new JoinGroupConsoleCommand());
    consoleCommandMap.put("quitGroup", new QuitGroupConsoleCommand());
    consoleCommandMap.put("listGroupMembers", new ListGroupMembersConsoleCommand());
}
```

接下来，就轮到服务端来处理加群请求了。      

### 服务端处理加群请求
在服务端的Pipeline中添加对应的Handler—JoinGroupRequestHandler。                 
**NettyServer.java**   
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
                        ch.pipeline().addLast(new Spliter());
                        ch.pipeline().addLast(new PacketDecoder());
                        // 登录请求处理器
                        ch.pipeline().addLast(new LoginRequestHandler());
                        ch.pipeline().addLast(new AuthHandler());
                        // 单聊消息请求处理器
                        ch.pipeline().addLast(new MessageRequestHandler());
                        // 创建群请求处理器
                        ch.pipeline().addLast(new CreateGroupRequestHandler());
                        // 加群请求处理器
                        ch.pipeline().addLast(new JoinGroupRequestHandler());
                        // 退群请求处理器
                        ch.pipeline().addLast(new QuitGroupRequestHandler());
                        // 获取群成员请求处理器
                        ch.pipeline().addLast(new ListGroupMembersRequestHandler());
                        // 登出请求处理器
                        ch.pipeline().addLast(new LogoutRequestHandler());
                        ch.pipeline().addLast(new PacketEncoder());
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

JoinGroupRequestHandler的具体逻辑如下。
**JoinGroupRequestHandler.java**    
```java
public class JoinGroupRequestHandler extends SimpleChannelInboundHandler<JoinGroupRequestPacket> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, JoinGroupRequestPacket requestPacket) {
        // 1. 获取群对应的 channelGroup，然后将当前用户的 channel 添加进去
        String groupId = requestPacket.getGroupId();
        ChannelGroup channelGroup = SessionUtil.getChannelGroup(groupId);
        channelGroup.add(ctx.channel());

        // 2. 构造加群响应发送给客户端
        JoinGroupResponsePacket responsePacket = new JoinGroupResponsePacket();

        responsePacket.setSuccess(true);
        responsePacket.setGroupId(groupId);
        ctx.channel().writeAndFlush(responsePacket);
    }
}
```

1.在通过groupId拿到对应的ChannelGroup之后，只需要调用ChannelGroup.add()方法，将加入群聊的用户的Channel添加进去，服务端即完成了加入群聊的逻辑。              
2.构造一个加群响应，填入groupId之后，调用writeAndFlush()方法把加群响应发送给加入群聊的客户端。              

### 客户端处理加群响应
我们在客户端的Pipeline中添加对应的Handler—JoinGroupResponseHandler来处理加群之后的响应。                
**NettyClient.java**   
```java
public class NettyClient {
    private static final int MAX_RETRY = 5;
    private static final String HOST = "127.0.0.1";
    private static final int PORT = 8000;


    public static void main(String[] args) {
        NioEventLoopGroup workerGroup = new NioEventLoopGroup();

        Bootstrap bootstrap = new Bootstrap();
        bootstrap
                .group(workerGroup)
                .channel(NioSocketChannel.class)
                .option(ChannelOption.CONNECT_TIMEOUT_MILLIS, 5000)
                .option(ChannelOption.SO_KEEPALIVE, true)
                .option(ChannelOption.TCP_NODELAY, true)
                .handler(new ChannelInitializer<SocketChannel>() {
                    @Override
                    public void initChannel(SocketChannel ch) {
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
                        // 登出响应处理器
                        ch.pipeline().addLast(new LogoutResponseHandler());
                        ch.pipeline().addLast(new PacketEncoder());
                    }
                });

        connect(bootstrap, HOST, PORT, MAX_RETRY);
    }

    private static void connect(Bootstrap bootstrap, String host, int port, int retry) {
        bootstrap.connect(host, port).addListener(future -> {
            if (future.isSuccess()) {
                System.out.println(new Date() + ": 连接成功，启动控制台线程……");
                Channel channel = ((ChannelFuture) future).channel();
                startConsoleThread(channel);
            } else if (retry == 0) {
                System.err.println("重试次数已用完，放弃连接！");
            } else {
                // 第几次重连
                int order = (MAX_RETRY - retry) + 1;
                // 本次重连的间隔
                int delay = 1 << order;
                System.err.println(new Date() + ": 连接失败，第" + order + "次重连……");
                bootstrap.config().group().schedule(() -> connect(bootstrap, host, port, retry - 1), delay, TimeUnit
                        .SECONDS);
            }
        });
    }

    private static void startConsoleThread(Channel channel) {
        ConsoleCommandManager consoleCommandManager = new ConsoleCommandManager();
        LoginConsoleCommand loginConsoleCommand = new LoginConsoleCommand();
        Scanner scanner = new Scanner(System.in);

        new Thread(() -> {
            while (!Thread.interrupted()) {
                if (!SessionUtil.hasLogin(channel)) {
                    loginConsoleCommand.exec(scanner, channel);
                } else {
                    consoleCommandManager.exec(scanner, channel);
                }
            }
        }).start();
    }
}
```

JoinGroupResponseHandler对应的逻辑如下。                

**JoinGroupResponseHandler.java**           
```java
public class JoinGroupResponseHandler extends SimpleChannelInboundHandler<JoinGroupResponsePacket> {

    @Override
    protected void channelRead0(ChannelHandlerContext ctx, JoinGroupResponsePacket responsePacket) {
        if (responsePacket.isSuccess()) {
            System.out.println("加入群[" + responsePacket.getGroupId() + "]成功!");
        } else {
            System.err.println("加入群[" + responsePacket.getGroupId() + "]失败，原因为：" + responsePacket.getReason());
        }
    }
}
```
该处理器的逻辑很简单，只是简单地将加群的结果输出到控制台，实际生产环境的IM可能比这要复杂，但是修改起来也非常容易。至此，与加群相关的逻辑就讲解完成了。              

## 群的退出
关于群的退出逻辑与群的加入逻辑非常类似，这里展示一下关键代码。              
服务端退群的核心逻辑为QuitGroupRequestHandler。         
**QuitGroupRequestHandler.java**   
```java
public class QuitGroupRequestHandler extends SimpleChannelInboundHandler<QuitGroupRequestPacket> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, QuitGroupRequestPacket requestPacket) {
        // 1. 获取群对应的 channelGroup，然后将当前用户的 channel 移除
        String groupId = requestPacket.getGroupId();
        ChannelGroup channelGroup = SessionUtil.getChannelGroup(groupId);
        channelGroup.remove(ctx.channel());

        // 2. 构造退群响应发送给客户端
        QuitGroupResponsePacket responsePacket = new QuitGroupResponsePacket();

        responsePacket.setGroupId(requestPacket.getGroupId());
        responsePacket.setSuccess(true);
        ctx.channel().writeAndFlush(responsePacket);

    }
}
```
从上面的代码可以看到，QuitGroupRequestHandler和JoinGroupRequestHandler其实是一个逆向的过程。                
1.通过groupId拿到对应的ChannelGroup之后，只需要调用ChannelGroup.remove()方法，将当前用户的Channel删除，服务端即完成了退群的逻辑。               
2.构造一个退群响应，填入groupId之后，调用writeAndFlush()方法把退群响应发送给退群的客户端。          
至此，加群和退群的逻辑就讲解完成了。最后，我们来看一下获取群成员列表的逻辑。                

## 获取群成员列表
### 在控制台添加获取群成员列表命令处理器            
**ListGroupMembersConsoleCommand.java**         
```java
public class ListGroupMembersConsoleCommand implements ConsoleCommand {

    @Override
    public void exec(Scanner scanner, Channel channel) {
        ListGroupMembersRequestPacket listGroupMembersRequestPacket = new ListGroupMembersRequestPacket();

        System.out.print("输入 groupId，获取群成员列表：");
        String groupId = scanner.next();

        listGroupMembersRequestPacket.setGroupId(groupId);
        channel.writeAndFlush(listGroupMembersRequestPacket);
    }
}
```

依旧按照前面的套路，我们在控制台先添加获取群成员列表命令处理器ListGroupMembers- ConsoleCommand。在这个处理器中，我们创建一个指令对象ListGroupMembersRequestPacket，填上群ID之后，将数据包发送至服务端。之后，将该控制台指令添加到ConsoleCommandManager。                        

### 服务端处理获取群成员列表请求
在服务端的Pipeline中添加对应的Handler—ListGroupMembersRequestHandler。              

ListGroupMembersRequestHandler的具体逻辑如下。          
**ListGroupMembersRequestHandler.java**         
```java
public class ListGroupMembersRequestHandler extends SimpleChannelInboundHandler<ListGroupMembersRequestPacket> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, ListGroupMembersRequestPacket requestPacket) {
        // 1. 获取群的 ChannelGroup
        String groupId = requestPacket.getGroupId();
        ChannelGroup channelGroup = SessionUtil.getChannelGroup(groupId);

        // 2. 遍历群成员的 channel，对应的 session，构造群成员的信息
        List<Session> sessionList = new ArrayList<>();
        for (Channel channel : channelGroup) {
            Session session = SessionUtil.getSession(channel);
            sessionList.add(session);
        }

        // 3. 构建获取成员列表响应写回到客户端
        ListGroupMembersResponsePacket responsePacket = new ListGroupMembersResponsePacket();

        responsePacket.setGroupId(groupId);
        responsePacket.setSessionList(sessionList);
        ctx.channel().writeAndFlush(responsePacket);
    }
}
``` 

1.通过groupId拿到对应的ChannelGroup。           
2.创建一个sessionList用来装载群成员信息，遍历Channel的每个Session，把对应的用户信息都装到sessionList中。在实际生产环境中，这里可能会构造另外一个对象来装载用户信息而非Session。                 
3.构造一个获取群成员列表的响应指令数据包，填入groupId和群成员信息之后，调用writeAndFlush()方法把响应发送给发起获取群成员列表的客户端。                  
最后，就剩下客户端来处理获取群成员列表的响应了。         

## 运行演示         
**NettyServer**         
```bash
Thu Jan 02 00:16:03 CST 2025: 端口[8000]绑定成功!
[yzhou]登录成功
[jj]登录成功
群创建成功，id 为 4012705a, 群里面有：[yzhou, jj]
```

**NettyClient01**   
```bash
Thu Jan 02 00:16:27 CST 2025: 连接成功，启动控制台线程……
输入用户名登录: yzhou
[yzhou]登录成功，userId 为: 92f25f0d
createGroup
【拉人群聊】输入 userId 列表，userId 之间英文逗号隔开：92f25f0d,71c41900
群创建成功，id 为[4012705a], 群里面有：[yzhou, jj]
listGroupMembers
输入 groupId，获取群成员列表：4012705a
群[4012705a]中的人包括：[71c41900:jj, 92f25f0d:yzhou]
``` 

**NettyClient02**      
```bash
Thu Jan 02 00:16:42 CST 2025: 连接成功，启动控制台线程……
输入用户名登录: jj
[jj]登录成功，userId 为: 71c41900
群创建成功，id 为[4012705a], 群里面有：[yzhou, jj]
```

## 总结
添加一个服务端和客户端交互的新功能只需要遵循以下步骤。          
1.创建控制台指令对应的ConsoleCommand，并将其添加到ConsoleCommandManager。           
2.在控制台输入指令和数据之后，填入协议对应的指令数据包—xxxRequestPacket，将请求写到服务端。             
3.服务端创建对应的xxxRequestPacketHandler，并将其添加到服务端的Pipeline中，在xxxRequestPacketHandler处理完后构造对应的xxxResponsePacket发送给客户端。           
4.客户端创建对应的xxxResponsePacketHandler，并将其添加到客户端的Pipeline中，最后在xxxResponsePacketHandler中完成响应的处理。            
5.最容易忽略的是，新添加xxxPacket时别忘了完善编解码器PacketCodec中的packetTypeMap。     
