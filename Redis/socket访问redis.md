## Socket访问Redis

### 背景
在java程序中使用`jedis`访问redis，在桌面端中使用`RedisInsight`访问redis，在shell终端中使用nc访问redis。  
>那如何设计一个redis console来访问redis呢？

### Docker部署redis
请参考redis安装官网文档 https://redis.io/docs/stack/get-started/install/

```shell
# 1.查看redis镜像仓库
访问 https://hub.docker.com/_/redis?tab=tags

# 2.获取redis镜像
docker pull redis:latest

# 3.查看本地镜像
docker images

# 4.运行容器
docker run -itd --name redis-test -p 6379:6379 redis

# 5.登录redis pod
docker exec -it redis-test /bin/bash
```

### 理解redis网络协议
请参考redis网络协议官网文档https://redis.io/docs/reference/protocol-spec/, 下面梳理下网络协议内容（本章节参考网友翻译blog）。    

**什么是RESP**
Redis是基于客户端-服务端模式工作的，客户端发送命令给服务端，服务端执行命令，然后将命令执行结果返回给客户端。为了满足Redis高性能的要求，Redis特地设计了RESP（全称REdis Serialization Protocol）协议，用来作为Redis客户端与服务端的通讯协议，RESP协议有以下优点。     
* 实现简单
* 解析高效
* 可读性好注意：RESP底层用的连接方式还是TCP，RESP只定义了客户端与服务端的数据交互格式，这也是为什么Redis能用Netcat（nc）等工具进行操作的原因

**初识RESp**
首先，使用redis-cli执行PING命令
```shell
redis-cli -h localhost -p 6379 PING
PONG
``` 
可以看到，redis服务端返回了PONG。接着使用Netcat同样发送PING命令给redis服务端    
```shell
echo "PING" | nc localhost 6379
+PONG
```

>nc即Netcat是一款网络工具，可以方便的从TCP套接字中发送和读取数据

相比于redis-cli,Netcat执行PING命令的返回结果多了个`+`号，这是为什么呢？     
事实上，+PONG才是redis-server返回的真正结果，因为客户端与服务端之间进行数据交互的时候，都要遵循RESP协议，RESP协议规定，在每种数据类型的前面，都要增加一个额外的字节，用于区分不同的数据类型。比如用+表示`Simple String`类型。这也是为什么nc的返回结果为+PONG的原因，+号表示返回结果是一个Simple Strings类型，后面的PONG则是Simple String的内容。    
至于redis-cli返回结果PONG，则是redis-cli解析后的结果。  

**RESP 数据类型**
在RESP中，总共定义了5种数据类型，分别是Simple String、Errors、Intergers、Bulk Strings和Arrays，第一个字节与数据类型的映射关系如下：

* 对于Simple Strings类型，第一个字节为+
* Errors类型，第一个字节为-
* Integers类型，第一个字节为:
* Bulk Strings类型，第一个字节为$
* Arrays类型，第一个字节为*

**RESP 简单字符串—Simple Strings**
Simple Strings的编码方式如下：

* 第一个字符为+
* 紧接着是一个不能包含CR或LF的字符串（不允许换行）
* 以CRLF结束    
>CR即\r->回车，LF即\n->换行

Simple Strings能保证在最小开销的情况下传输非二进制安全的字符串，比如很多Redis命令执行成功都要返回OK字符串，该字符串通过Simple Strings编码为5个字节的数据报如下
```shell
+OK\r\n
```
客户端在收到+OK\r\n的时候，需要将其转换成客户端语言对应的字符串类型返回给调用者。字符串的值为+号至\r\n中间的字符串，也就是OK。  
如果需要返回二进制安全的字符串，则需要使用Bluk Strings，后面会介绍

**RESP 错误响应—Errors**
Errors是RESP特定的数据类型,用于返回异常信息，Errors的编码方式跟Simple Strings的编码方式很像，唯一的区别是Errors的第一个字节为-，完整的的编码方式如下：

* 第一个字节为-
* 紧接着是一个不能包含CR或LF的字符串
* 以CRLF结束只有在发生错误的时候才会返回Errors，比如命令不存在/操作了错误的数据类型，Redis客户端在收到Errors类型的数据时，应该识别为异常消息，比如Redis服务端返回的消息如下
```shell
-ERR unknown command `foo`\r\n
```

**RESP 整型-Integer**
Integer的编码格式如下：

* 第一个字符为:
* 紧接着为不能包含CR或LF的数字
* 以CRLF结尾在Redis中，当需要返回整型的时候，就是用的Integer进行返回，如INCR、LLEN等命令，比如返回1
```shell
:1\r\n
```

**RESP Bluk Strings**
Bluk Strings用于表示二进制安全的字符串，最大为 512M（Bluk有体积大的含义），Bluk Strings的编码格式如下：

* 第一个字节为$
* 紧接着一个数字用于表示字符串的字节数（称为prefixed length，也就是前缀长度），前缀长度以CRLF结尾
* 接着为真实的字符串数据
* 以CRLF结尾比如字符串foobar使用Bluk Strings编码为
```shell
$3\r\nfoobar\r\n
``` 
空字符串（对应java中的""）表示为
```shell
$0\r\n\r\n
```
特别注意的是，RESP使用长度为-1的Bluk Strings表示不存在的值，即NULL
```shell
$-1\r\n
```
Redis客户端在解析的时候，需要转换成对应语言的NULL值，如Ruby中的nil,C中的NULL等。

**RESP 数组 Arrays**
当服务端需要返回多个数据给客户端的时候，就是使用Arrays进行返回的，如LRANGE命令返回的就是一个数组。此外，客户端发送给服务端的命令，也都是使用Arrays进行发送的。Arrays的编码格式如下：

* 第一个字符为*
* 后面接数组的元素个数，以CRLF结尾
* 接着为Arrays中每个元素的数据，元素类型为上面介绍的4中数据类型

比如空数组可以表示为
```shell
*0\r\n
```
包含两个Bluk Strings元素，元素内容分别为foo跟bar的Arrays表示为
```shell
*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
```
Arrays中各个元素的数据类型可以不同，比如包含一个Integer类型跟一个Simple Strings类型的Arrays，表示为
```shell
*2\r\n:1\r\n+hello\r\n
```
客户端发送命令也是使用Arrays进行发送的，下面同样通过nc举例，比如发送SET foo bar命令
```shell
$ echo -e '*3\r\n$3\r\nset\r\n$3\r\nfoo\r\n$3\r\nbar\r\n' | nc localhost 6379
+OK
```

**内联命令**
Redis提供了内联命令，方便我们在没有redis-cli的时候，使用telnet等其他工具与redis进行交互，比如我们可以使用nc与redis交互
```shell
$ nc localhost 6379
PING
+PONG
SET foo bar
+OK
```
上面示例，PING和SET foobar是我们输入的命令，+PONG和OK则是服务端返回的结果


### 利用Socket+Scanner构建redis-cli




### refer
1.《一文搞懂Redis通讯协议RESP》https://juejin.cn/post/6937688561344839711




