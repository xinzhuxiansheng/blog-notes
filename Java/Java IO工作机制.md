
# BIO

## 1. BIO工作机制

案例：
* 使用BIO模型编写一个服务器端，监听6666端口，当有客户端连接时，就启动一个线程与之通讯。
* 要求使用线程池机制改善，可以连接多个客户端
* 服务端可以接受客户端发送的数据(telnet 方式即可)



`telnet操作`    
windows telnet 模拟， 并且执行"CTRL + ]"
```shell
# 1. 执行 telnet
telnet 127.0.0.1 6666
# 2. 执行 CTRL+]
CTRL+]
```
`telnet 参数`   
```java
Microsoft Telnet> help
命令可能是缩写。支持的命令为:
c    - close                    关闭当前连接
d    - display                  显示操作参数
o    - open hostname [port]     连接到主机(默认端口 23)。
q    - quit                     退出 telnet
set  - set                      设置选项(键入 'set ?' 获得列表)
sen  - send                     将字符串发送到服务器
st   - status                   打印状态信息
u    - unset                    解除设置选项(键入 'set ?' 获得列表)
?/h  - help                     打印帮助信息   
```

![telnet](images/io01.png)


## 2. BIO问题分析
* 每个请求都需要创建独立的线程，与对应的客户端进行数据Read，业务处理，数据Write。
* 当并发数较大时，需要创建大量线程来处理连接，系统资源占用较大。
* 连接建立后，如果当前线程暂时没有数据可读，则线程就阻塞在Read操作上，造成线程资源浪费。



# NIO

* NIO相关类都放在java.nio包及子包下，并且对原java.io包中的很多类进行改写。
* NIO有三大核心部分：Channel(通道), Buffer(缓冲区), Selector(选择器)。
* NIO是面向缓冲区或者面向块编程的，数据读取到一个它稍后处理的缓冲区，需要时可在缓冲区中前后移动，这就增加了处理过程中的灵活性，使用它可以提供非阻塞式的高伸缩性网络。


## NIO和BIO的比较
* BIO以流的方式处理数据，而NIO以块的方式处理数据，块I/O的效率比流I/O高很多
* BIO是阻塞的，NIO是非阻塞的
* BIO基于字节流和字符流进行操作，而NIO基于Channel(通道)和Buffer(缓冲区)进行操作，数据总是从通道读取到缓冲区中，或者从缓冲区写入到通道中。Selector(选择器)用于监听多个通道的事件（比如:连接请求，数据到达等），因此使用单个线程就可以监听多个客户端通道


## Selector、Channel和Buffer的关系图
* 每个Channel都会对应一个Buffer
* Selector对应一个线程，一个线程对应多个Channel(连接)
* 程序切换到哪个Channel是由事件决定，Event就是一个重要的概念
* Selector会根据不同的事件，在各个通道上切换
* Buffer就是一个内存块
* 数据的读取写入是通过Buffer
* Channel是双向的，可以返回底层操作系统的情况，比如Linux，底层的操作系统通道就是双向的



Selector 参考 https://www.cnblogs.com/snailclimb/p/9086334.html



## select()方法介绍

在刚初始化的Selector对象中，这三个集合都是空的。 通过Selector的select（）方法可以选择已经准备就绪的通道 （这些通道包含你感兴趣的的事件）。比如你对读就绪的通道感兴趣，那么select（）方法就会返回读事件已经就绪的那些通道。下面是Selector几个重载的select()方法：

int select()：阻塞到至少有一个通道在你注册的事件上就绪了。
int select(long timeout)：和select()一样，但最长阻塞时间为timeout毫秒。
int selectNow()：非阻塞，只要有通道就绪就立刻返回。

select()方法返回的int值表示有多少通道已经就绪,是自上次调用select()方法后有多少通道变成就绪状态。之前在select（）调用时进入就绪的通道不会在本次调用中被记入，而在前一次select（）调用进入就绪但现在已经不在处于就绪的通道也不会被记入。例如：首次调用select()方法，如果有一个通道变成就绪状态，返回了1，若再次调用select()方法，如果另一个通道就绪了，它会再次返回1。如果对第一个就绪的channel没有做任何操作，现在就有两个就绪的通道，但在每次select()方法调用之间，只有一个通道就绪了。

一旦调用select()方法，并且返回值不为0时，则 可以通过调用Selector的selectedKeys()方法来访问已选择键集合 。如下：
Set selectedKeys=selector.selectedKeys();
进而可以放到和某SelectionKey关联的Selector和Channel。如下所示：
```java
Set selectedKeys = selector.selectedKeys();
Iterator keyIterator = selectedKeys.iterator();
while(keyIterator.hasNext()) {
    SelectionKey key = keyIterator.next();
    if(key.isAcceptable()) {
        // a connection was accepted by a ServerSocketChannel.
    } else if (key.isConnectable()) {
        // a connection was established with a remote server.
    } else if (key.isReadable()) {
        // a channel is ready for reading
    } else if (key.isWritable()) {
        // a channel is ready for writing
    }
    keyIterator.remove();
}
```


## NIO与零拷贝
* 零拷贝是网络编程的关键，很多性能优化都离不开  
* 在Java程序中，常用的零拷贝有mmap(内存映射)和sendFile，那么他们在OS里，到底是怎么样的一个的设计？ 我们分析mmap和sendFile这两个零拷贝   
* NIO中如何使用零拷贝   