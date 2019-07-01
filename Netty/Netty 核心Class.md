**正文**

[TOC]

## NioEventLoop Class
Netty的NioEventLoop并不是一个纯碎的I/O的线程，它除了负责I/O的读写之外，还兼顾处理一下两类任务。
* 系统Task：通过调用NioEventLoop的execute(Runnable task)方法实现，Netty有很多系统Task，创建它们的主要原因是：当I/O线程和用户线程同事操作网络资源时，为了防止并发操作导致的锁竞争，将用户线程的操作封装程Task放入消息队列中，由I/O线程负责执行，这样就实现了局部无锁化。
* 定时任务：通过调用NioEventLoop的schedule(Runnable command,long delay,TimeUnit unit)方法实现。