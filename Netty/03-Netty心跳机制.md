## Netty心跳机制

心跳机制的工作原理是：在服务器和客户端之间一定时间内没有数据交互时，即处于idle状态时，客户端或服务器会发送一个特殊的数据包给对方，当接收方收到这个数据报文后，也立即发送一个特殊的数据报文，回应发送方，此即一个PING-PONG交互，自然的，当某一端收到心跳消息后，就知道了对方仍然在线，这就确保TCP连接的有效性。

>如何实现心跳？  
两种方式实现心跳机制：
* 使用TCP协议层面的keepalive机制
TCP keepalive机制依赖操作系统的实现，它默认是关闭的，但一般应用都会修改设置来开启，这种需要调整系统配置，故灵活性不够。
>refer: 《linux下tcp keepalive相关参数调整测试》https://imliuda.com/post/727

* 在应用层上实现自定义的心跳机制
HTTP的KeepAlive在HTTP 1.0版本默认是关闭的，但在HTTP1.1是默认开启的。    



refer: 《TCP和HTTP中的KeepAlive机制总结》https://cloud.tencent.com/developer/news/696654