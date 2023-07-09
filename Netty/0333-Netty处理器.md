
## Netty处理器


### 介绍handler处理器

Netty发送和接受数据handler处理器主要是继承`SimpleChannelInboundHandler`和`ChannelInboundHandlerAdapter`。 那么这两个到底有什么区别呢？ 

其实用这两个抽象类是有讲究的，在客户端的业务Handler继承的是SimpleChannelInboundHandler, 而在服务端继承的是ChannelInboundHandlerAdapter。    

最主要的区别是SimpleChannelInboundHandler在接受数据后会自动release掉数据占用Bytebuffer资源（自动调用Bytebuffer.release())。而为何服务端不能用呢，而是将消息传递给下一个ChannelHandler处理。

**SimpleChannelInboundHandler**
```java

public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    boolean release = true;

    try {
        if (this.acceptInboundMessage(msg)) {
            this.channelRead0(ctx, msg);
        } else {
            release = false;
            ctx.fireChannelRead(msg);
        }
    } finally {
        if (this.autoRelease && release) {
            ReferenceCountUtil.release(msg);
        }

    }

}

```

**ChannelInboundHandlerAdapter**
```java
@Skip
public void channelRead(ChannelHandlerContext ctx, Object msg) throws Exception {
    ctx.fireChannelRead(msg);
}
```

从类定义看，SimpleChannelInboundHandler<T>是抽象类且支持泛型消息处理，而ChannelInboundHandlerAdapter是普通类，不支持泛型
**SimpleChannelInboundHandler**
```java
public abstract class SimpleChannelInboundHandler<I> extends ChannelInboundHandlerAdapter
```

```java
public class ChannelInboundHandlerAdapter extends ChannelHandlerAdapter implements ChannelInboundHandler
```

### ChannelInboundHandlerAdapter的方法
请参考 https://netty.io/4.0/api/io/netty/channel/ChannelInboundHandlerAdapter.html
