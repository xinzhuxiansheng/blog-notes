**`正文`**

[TOC]

EventLoopGroup bossGroup = new NioEventLoopGroup(CommonConstants.BOSS_GROUP_SIZE, new DefaultThreadFactory("boss", true));
EventLoopGroup workerGroup = new NioEventLoopGroup(CommonConstants.WORKER_GROUP_SIZE, new DefaultThreadFactory("worker", true));
try {
    long start = System.currentTimeMillis();
    ServerBootstrap b = new ServerBootstrap();
    b.option(ChannelOption.SO_BACKLOG, 1024);
    b.group(bossGroup, workerGroup)
        .channel(NioServerSocketChannel.class)
//             .handler(new LoggingHandler(LogLevel.INFO))
        .childHandler(new NettyHttpServerInitializer());

    ChannelFuture future = b.bind(CommonConstants.SERVER_PORT).sync();
    long cost = System.currentTimeMillis()-start;
    LOGGER.info("[NettyHttpServer] Startup at port:{} cost:{}[ms]",CommonConstants.SERVER_PORT,cost);

    // 等待服务端Socket关闭
    future.channel().closeFuture().sync();
} catch (InterruptedException e) {
    LOGGER.error("[NettyHttpServer] InterruptedException:",e);
} finally {
    bossGroup.shutdownGracefully();
    workerGroup.shutdownGracefully();
}


EventLoopGroup



构建一对主从线程组
定义服务器启动类
为服务器设置Channel
设置处理从线程池的助手类初始化器
监听启动和关闭服务器










