### Netty简易聊天室实践

一个简单的聊天室具备哪些功能

* 创建Server（具备端口检测）
* 创建Client（具备启动重连）


### 设置编解码
`encoder`,`decoder`设置Netty的编解码处理类

```java
ServerBootstrap b = new ServerBootstrap();
b.group(bossGroup, workerGroup)
        .channel(NioServerSocketChannel.class) 
        .childHandler(new ChannelInitializer<SocketChannel>() { 
            @Override
            public void initChannel(SocketChannel ch) throws Exception {
                Charset gbk = Charset.forName("utf-8");
                ch.pipeline().addLast(new DelimiterBasedFrameDecoder(1024, Unpooled.copiedBuffer("_"
                        .getBytes())));
                ch.pipeline().addLast("encoder", new StringEncoder(gbk));
                ch.pipeline().addLast("decoder", new StringDecoder(gbk));
                ch.pipeline().addLast(new DiscardServerHandler());
            }
        })
        .option(ChannelOption.SO_BACKLOG, 128) 
        .childOption(ChannelOption.SO_KEEPALIVE, true); 
```
