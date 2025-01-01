# Netty - 使用ChannelHandler的热插拔实现客户端身份校验   

## 引言         
细心的读者可能会注意到，客户端连上服务端之后，即使没有进行登录校验，服务端在收到消息之后仍然会进行消息的处理，这个逻辑其实是有问题的。接下来介绍如何使用Pipeline及Handler强大的热插拔机制来实现客户端身份校验。                 

## 身份检验         
首先，我们在客户端登录成功之后，标记当前Channel的状态为已登录。         
**LoginRequestHandler.java**  
```java
public class LoginRequestHandler extends SimpleChannelInboundHandler<LoginRequestPacket> {
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, LoginRequestPacket loginRequestPacket) {
        System.out.println(new Date() + ": 收到客户端登录请求……");

        LoginResponsePacket loginResponsePacket = new LoginResponsePacket();
        loginResponsePacket.setVersion(loginRequestPacket.getVersion());
        if (valid(loginRequestPacket)) {
            loginResponsePacket.setSuccess(true);
            System.out.println(new Date() + ": 登录成功!");
            LoginUtil.markAsLogin(ctx.channel());
        } else {
            loginResponsePacket.setReason("账号密码校验失败");
            loginResponsePacket.setSuccess(false);
            System.out.println(new Date() + ": 登录失败!");
        }

        // 登录响应
        ctx.channel().writeAndFlush(loginResponsePacket);
    }

    private boolean valid(LoginRequestPacket loginRequestPacket) {
        return true;
    }
}

```

**LoginUtil.java**  
```java
public static void markAsLogin(Channel channel) {
    channel.attr(Attributes.LOGIN).set(true);
}
```
在登录成功之后，我们通过给Channel打属性标记的方式，标记这个Channel已成功登录。接下来，我们是不是需要在处理后续的每一种指令前，都判断一下用户是否登录？              
**LoginUtil.java**      
```java
public static boolean hasLogin(Channel channel) {    
    Attribute<Boolean> loginAttr = channel.attr(Attributes.LOGIN);    
    return loginAttr.get() ！= null;
}
```

判断一个用户是否登录很简单，只需要调用LoginUtil.hasLogin(channel)即可。但是，Netty的Pipeline机制帮我们省去了重复添加同一段逻辑的烦恼，我们只需要在后续所有的指令处理Handler之前插入一个用户认证Handler即可。            

**NettyServer.java**  
```java
.childHandler(new ChannelInitializer<NioSocketChannel>() {
    protected void initChannel(NioSocketChannel ch) {
        ch.pipeline().addLast(new LifeCyCleTestHandler());
        ch.pipeline().addLast(new PacketDecoder());
        ch.pipeline().addLast(new LoginRequestHandler());

        // 新增加用户认证handler
        ch.pipeline().addLast(new AuthHandler());
        ch.pipeline().addLast(new MessageRequestHandler());
        ch.pipeline().addLast(new PacketEncoder());
    }
});
```  

从上面的代码可以看出，我们在MessageRequestHandler之前插入了一个AuthHandler，因此MessageRequestHandler以及后续所有与指令相关的Handler（后面小节会逐个添加）的处理都会经过AuthHandler的一层过滤，只要在AuthHandler里处理完与身份认证相关的逻辑，后续所有的Handler都不用再操心身份认证这个逻辑，我们来看AuthHandler的具体实现。                    

**AuthHandler.java**            
```java
public class AuthHandler extends ChannelInboundHandlerAdapter {

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        if (!LoginUtil.hasLogin(ctx.channel())) {
            ctx.channel().close();
        } else {
            ctx.pipeline().remove(this);
            super.channelRead(ctx, msg);
        }
    }

    @Override
    public void handlerRemoved(ChannelHandlerContext ctx) {
        if (LoginUtil.hasLogin(ctx.channel())) {
            System.out.println("当前连接登录验证完毕，无需再次验证, AuthHandler 被移除");
        } else {
            System.out.println("无登录验证，强制关闭连接!");
        }
    }
}
```

1.AuthHandler继承自 ChannelInboundHandlerAdapter，覆盖了channelRead()方法，表明它可以处理所有类型的数据。                    

2.在channelRead()方法里，在决定是否把读到的数据传递到后续指令处理器之前，首先会判断是否登录成功。如果未登录，则直接强制关闭连接，否则，就把读到的数据向下传递，传递给后续指令处理器。           

AuthHandler的处理逻辑其实就这么简单。但是，有的读者可能要问，如果客户端已经登录成功，那么在每次处理客户端数据之前，都要经历这么一段逻辑。比如，平均每次用户登录之后发送100次消息，其实剩余的99次身份校验逻辑都是没有必要的，因为只要连接未断开，只要客户端成功登录过，后续就不需要再进行客户端的身份校验。                                          

这里我们为了演示，身份认证逻辑比较简单，在实际生产环境中，身份认证逻辑可能会更复杂。我们需要寻找一种途径来避免资源与性能的浪费，使用ChannelHandler的热插拔机制完全可以做到这一点。                          

## 移除校验逻辑    
对于Netty的设计来说，Handler其实可以看作一段功能相对聚合的逻辑，然后通过Pipeline把一个个小的逻辑聚合起来，串成一个功能完整的逻辑链。既然可以把逻辑串起来，就可以做到动态删除一个或多个逻辑。                

在客户端校验通过之后，我们不再需要AuthHandler这段逻辑，而删除这段逻辑只需要一行代码即可实现。      
**AuthHandler.java**            
```java
public class AuthHandler extends ChannelInboundHandlerAdapter {

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
        if (!LoginUtil.hasLogin(ctx.channel())) {
            ctx.channel().close();
        } else {
            ctx.pipeline().remove(this);
            super.channelRead(ctx, msg);
        }
    }

    @Override
    public void handlerRemoved(ChannelHandlerContext ctx) {
        if (LoginUtil.hasLogin(ctx.channel())) {
            System.out.println("当前连接登录验证完毕，无需再次验证, AuthHandler 被移除");
        } else {
            System.out.println("无登录验证，强制关闭连接!");
        }
    }
}
```

在上面的代码中，判断如果已经经过权限认证，那么就直接调用Pipeline的remove()方法删除自身，这里的this指的其实就是AuthHandler这个对象。删除之后，这条客户端连接的逻辑链中就不再有这段逻辑了。           
另外，我们覆盖了handlerRemoved()方法，主要用于后续演示部分的内容。接下来，我们进行实际演示。                

## 身份校验演示   
在演示之前，对于客户端侧的代码，在客户端向服务端发送消息的逻辑中，我们先把每次都判断是否登录的逻辑去掉，这样就可以在客户端未登录的情况下向服务端发送消息。          
**NettyClient.java**   
```java
private static void startConsoleThread(Channel channel) {
    new Thread(() -> {
        while (!Thread.interrupted()) {
            //if (LoginUtil.hasLogin(channel)) {
                System.out.println("输入消息发送至服务端: ");
                Scanner sc = new Scanner(System.in);
                String line = sc.nextLine();

                channel.writeAndFlush(new MessageRequestPacket(line));

//                    for (int i = 0; i < 1000; i++) {
//                        channel.writeAndFlush(new MessageRequestPacket("你好，欢迎关注我的微信公众号，阿洋聊大数据 !"));
//                    }

            //}
        }
    }).start();
}
```

我们先启动服务端，再启动客户端。在客户端的控制台，我们输入消息发送至服务端，此时客户端与服务端控制台的输出分别如下面所示。      

**客户端：**        
```bash
Wed Jan 01 16:00:21 CST 2025: 连接成功，启动控制台线程……
输入消息发送至服务端: 
Wed Jan 01 16:00:21 CST 2025: 客户端登录成功
你好，yzhou！
输入消息发送至服务端: 
Wed Jan 01 16:00:31 CST 2025: 收到服务端的消息: 服务端回复【你好，yzhou！】
```

**服务端**      
```bash 
Wed Jan 01 16:00:15 CST 2025: 端口[8000]绑定成功!
Wed Jan 01 16:00:21 CST 2025: 收到客户端登录请求……
Wed Jan 01 16:00:21 CST 2025: 登录成功!
登录验证通过，移除 AuthHandler
Wed Jan 01 16:00:31 CST 2025: 收到客户端消息: 你好，yzhou！
```

## 总结
1.如果有很多业务逻辑的Handler都要进行某些相同的操作，则我们完全可以抽取出一个Handler来单独处理。                

2.如果某一个独立的逻辑在执行几次之后（这里是一次）不需要再执行，则可以通过ChannelHandler的热插拔机制来实现动态删除逻辑，使应用程序的性能处理更为高效。          
