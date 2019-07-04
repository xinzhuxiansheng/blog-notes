**正文**

[TOC]

## NioEventLoop Class
Netty的NioEventLoop并不是一个纯碎的I/O的线程，它除了负责I/O的读写之外，还兼顾处理一下两类任务。
* 系统Task：通过调用NioEventLoop的execute(Runnable task)方法实现，Netty有很多系统Task，创建它们的主要原因是：当I/O线程和用户线程同事操作网络资源时，为了防止并发操作导致的锁竞争，将用户线程的操作封装程Task放入消息队列中，由I/O线程负责执行，这样就实现了局部无锁化。
* 定时任务：通过调用NioEventLoop的schedule(Runnable command,long delay,TimeUnit unit)方法实现。

**NioEventLoopGroup的构造函数**

| 构造方法      |  含义   |
| :-------- | --------:|
| public NioEventLoopGroup()  | 使用默认线程数，默认线程工厂和selectorProvider(由selectorProvider.provider()返回的)  |
| public NioEventLoopGroup(int nThreads)  | 使用指定数量的线程数，默认线程工厂和selectorProvider(由selectorProvider.provider()返回的)  |
| public NioEventLoopGroup(int nThreads,java.util.concurrent.ThreadFactory threadFactory)      |  使用指定数量的线程数，给定的threadFactory和selectorProvider(由selectorProvider.provider()返回的)   |
| public NioEventLoopGroup(int nThreads,java.util.concurrent.Executor executor)  |     |
| public NioEventLoopGroup(int nThreads,java.util.concurrent.ThreadFactory threadFactory， 
java.nio.channels.spi.SelectorProvider selectorProvider) |  使用指定数量的线程，给定的ThreadFactory和给定的SelectorProvider  |
|  public NioEventLoopGroup(int nThreads,java.util.concurrent.ThreadFactory threadFactory,java.nio.channels.spi.SelectorProvider selectorProvider,SelectStrategyFactory selectStrategyFactory)   |     |
|  public NioEventLoopGroup(int nThreads,java.util.concurrent.ThreadFactory threadFactory,java.nio.channels.spi.SelectorProvider selectorProvider,SelectStrategyFactory selectStrategyFactory)   |     |
|  public NioEventLoopGroup(int nThreads,java.util.concurrent.Executor executor,
java.nio.channels.spi.SelectorProvider selectorProvider)  |  |
|  public NioEventLoopGroup(int nThreads,java.util.concurrent.Executor executor,java.nio.channels.spi.SelectorProvider selectorProvider,SelectStrategyFactory selectStrategyFactory)  |    |
|  public NioEventLoopGroup(int nThreads,java.util.concurrent.Executor executor,
EventExecutorChooserFactory chooserFactory,java.nio.channels.spi.SelectorProvider selectorProvider,SelectStrategyFactory selectStrategyFactory)  |   |
|  public NioEventLoopGroup(int nThreads,java.util.concurrent.Executor executor,EventExecutorChooserFactory chooserFactory,java.nio.channels.spi.SelectorProvider selectorProvider,SelectStrategyFactory selectStrategyFactory,RejectedExecutionHandler rejectedExecutionHandler)  |   |
| public NioEventLoopGroup(int nThreads,java.util.concurrent.Executor executor,
EventExecutorChooserFactory chooserFactory,java.nio.channels.spi.SelectorProvider selectorProvider,SelectStrategyFactory selectStrategyFactory, RejectedExecutionHandler rejectedExecutionHandler,EventLoopTaskQueueFactory taskQueueFactory)  |   |
