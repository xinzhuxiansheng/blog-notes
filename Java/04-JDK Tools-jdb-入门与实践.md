# JDK Tools - jdb - 入门与实践    

>Java version: 11

## 背景  
"假设" 你身处一个`无可视化`环境中，如何对 java 程序进行排错呢？（ps：在无特别说明的情况下，默认是 Linux Server环境，不包含桌面）。    
![javajdb01](images/javajdb01.png)       

通过上面描述的背景，来引出 jdb。     

## jdb 介绍     
大家可访问`https://docs.oracle.com/en/java/javase/11/tools/jdb.html`，了解 jdb 细节，下面引用官网描述进行开场：  

>#### jdb
>You use the jdb command and its options to find and fix bugs in Java platform programs.   

>#### Description
>The Java Debugger (JDB) is a simple command-line debugger for Java classes. The jdb command and its options call the JDB. The jdb command demonstrates the Java Platform Debugger Architecture and provides inspection and debugging of a local or remote JVM.

**1.** 通过官网 或者 其他 Blog介绍可知道，`jdb` 是 JDK自带的一个工具，类似于 `jstack`，它存在 `jdk-11.0.19/bin` 目录下。    

**2.** Synopsis 如下：  
```bash
jdb [options] [classname] [arguments]   
```

`options`           
This represents the jdb command-line options. See Options for the jdb command.          

`classname`             
This represents the name of the main class to debug.            

`arguments`             
This represents the arguments that are passed to the main() method of the class.





下面我们通过 `jdb --help`了解它包含的参数 

```bash
[root@vm03 bin]# jdb --help
Usage: jdb <options> <class> <arguments>

where options include:
    -? -h --help -help print this help message and exit
    -sourcepath <directories separated by ":">
                      directories in which to look for source files
    -attach <address>
                      attach to a running VM at the specified address using standard connector
    -listen <address>
                      wait for a running VM to connect at the specified address using standard connector
    -listenany
                      wait for a running VM to connect at any available address using standard connector
    -launch
                      launch VM immediately instead of waiting for 'run' command
    -listconnectors   list the connectors available in this VM
    -connect <connector-name>:<name1>=<value1>,...
                      connect to target VM using named connector with listed argument values
    -dbgtrace [flags] print info for debugging jdb
    -tclient          run the application in the HotSpot(TM) Client Compiler
    -tserver          run the application in the HotSpot(TM) Server Compiler

options forwarded to debuggee process:
    -v -verbose[:class|gc|jni]
                      turn on verbose mode
    -D<name>=<value>  set a system property
    -classpath <directories separated by ":">
                      list directories in which to look for classes
    -X<option>        non-standard target VM option

<class> is the name of the class to begin debugging
<arguments> are the arguments passed to the main() method of <class>

For command help type 'help' at jdb prompt
```

**翻译**            
>以下是 `jdb` 命令中各个选项的含义：

1. **`-?`、`-h`、`--help`、`-help`**:           
   - 打印帮助信息并退出。这些选项都是等价的，用于显示如何使用 `jdb` 以及可用的选项。            

2. **`-sourcepath <directories>`**:             
   - 指定查找源文件的目录。目录之间用冒号（`:`）分隔。这对于调试时需要查看源代码的情况很有用。              

3. **`-attach <address>`**:         
   - 连接到正在运行的虚拟机（VM），使用标准连接器，并在指定的地址进行连接。这在调试已经运行的程序时非常有用。           

4. **`-listen <address>`**:
   - 等待运行中的虚拟机在指定地址连接，使用标准连接器。这是为被调试的程序设置一个监听地址。         

5. **`-listenany`**:            
   - 等待运行中的虚拟机在任何可用地址连接，使用标准连接器。这与 `-listen` 类似，但不需要指定具体地址。              

6. **`-launch`**:       
   - 立即启动虚拟机，而不是等待 `run` 命令。这意味着调试会从虚拟机启动时就开始。            

7. **`-listconnectors`**:           
   - 列出当前虚拟机中可用的连接器。连接器用于设置 `jdb` 如何连接到被调试的虚拟机。

8. **`-connect <connector-name>:<name1>=<value1>,...`**:            
   - 使用指定的连接器并带有列出的参数值连接到目标虚拟机。这个选项允许通过指定连接器及其参数来精细控制连接方式。         

9. **`-dbgtrace [flags]`**:         
   - 打印 `jdb` 的调试信息，用于 `jdb` 本身的调试。`flags` 可以用来进一步指定需要调试的信息类型。           

10. **`-tclient`**:             
    - 在 HotSpot™ 客户端编译器模式下运行应用程序。客户端编译器通常用于桌面应用或需要快速启动时间的场景。            

11. **`-tserver`**:             
    - 在 HotSpot™ 服务器编译器模式下运行应用程序。服务器编译器适用于长期运行的服务器端应用程序，优化程度更高。          

>### 转发到被调试进程的选项：            
12. **`-v`、`-verbose[:class|gc|jni]`**:
    - 启用详细模式。可以通过可选参数 `class`、`gc` 或 `jni` 来指定要详细输出的内容，例如类加载、垃圾回收或 JNI 调用的详细信息。

13. **`-D<name>=<value>`**:
    - 设置系统属性。这与 Java 命令行选项 `-D` 类似，允许为被调试的应用程序设置系统属性。

14. **`-classpath <directories>`**:
    - 指定查找类文件的目录。目录之间用冒号（`:`）分隔。

15. **`-X<option>`**:
    - 设置非标准的目标虚拟机选项。`-X` 选项通常用于配置虚拟机的特定行为，这些选项是 JVM 专有的。

>### 其他参数：
16. **`<class>`**:
    - 要开始调试的类的名称。`jdb` 将从该类的 `main()` 方法开始执行和调试。

17. **`<arguments>`**:
    - 传递给 `<class>` 的 `main()` 方法的参数。这些参数与正常运行 Java 程序时传递的命令行参数相同。


## 构建 java 示例  
该示例使用Netty框架开发一个 HTTP 服务，当可通过 `curl 模拟 POST`请求 `http://localhost:8080/netty/api/getUserInfo` 获取用户信息, 下面介绍示例的项目构造：       

**pom.xml**     
```bash
<dependency>
    <groupId>io.netty</groupId>
    <artifactId>netty-all</artifactId>
    <version>4.1.35.Final</version>
</dependency>
<dependency>
    <groupId>com.fasterxml.jackson.core</groupId>
    <artifactId>jackson-databind</artifactId>
    <version>2.17.0</version>
    <scope>compile</scope>
</dependency>


<build>
    <plugins>
        <plugin>
            <artifactId>maven-compiler-plugin</artifactId>
            <configuration>
                <source>${java.version}</source>
                <target>${java.version}</target>
                <encoding>UTF-8</encoding>
            </configuration>
        </plugin>

        <plugin>
            <groupId>org.apache.maven.plugins</groupId>
            <artifactId>maven-shade-plugin</artifactId>
            <version>3.2.4</version>
            <executions>
                <execution>
                    <phase>package</phase>
                    <goals>
                        <goal>shade</goal>
                    </goals>
                    <configuration>
                        <transformers>
                            <transformer implementation="org.apache.maven.plugins.shade.resource.ManifestResourceTransformer">
                                <mainClass>com.example.MainClass</mainClass> <!-- 入口点类 -->
                            </transformer>
                        </transformers>
                    </configuration>
                </execution>
            </executions>
        </plugin>
    </plugins>
</build>
```

**NettyHttpServer**                   
```java
package com.javamain.netty.example02;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.buffer.Unpooled;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.http.*;
import io.netty.handler.codec.json.JsonObjectDecoder;
import io.netty.handler.codec.string.StringEncoder;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import io.netty.util.CharsetUtil;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class NettyHttpServer {

    private static final int PORT = 8080;

    public static void main(String[] args) throws Exception {
        EventLoopGroup bossGroup = new NioEventLoopGroup(1);
        EventLoopGroup workerGroup = new NioEventLoopGroup();
        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .handler(new LoggingHandler(LogLevel.INFO))
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel ch) throws Exception {
                            ChannelPipeline p = ch.pipeline();
                            p.addLast(new HttpServerCodec());
                            p.addLast(new HttpObjectAggregator(65536));
                            p.addLast(new JsonObjectDecoder());
                            p.addLast(new StringEncoder(CharsetUtil.UTF_8));
                            p.addLast(new HttpServerHandler());
                        }
                    });

            ChannelFuture f = b.bind(PORT).sync();
            System.out.println("Server started, listening on " + PORT);
            f.channel().closeFuture().sync();
        } finally {
            bossGroup.shutdownGracefully();
            workerGroup.shutdownGracefully();
        }
    }

    static class HttpServerHandler extends SimpleChannelInboundHandler<FullHttpRequest> {

        private static final ObjectMapper objectMapper = new ObjectMapper();

        @Override
        protected void channelRead0(ChannelHandlerContext ctx, FullHttpRequest request) throws Exception {
            if (request.uri().equals("/netty/api/getUserInfo") && request.method().name().equals("POST")) {
                String json = request.content().toString(StandardCharsets.UTF_8);
                Map<String, Object> data = objectMapper.readValue(json, HashMap.class);

                int userId = (int) data.get("id");
                Map<String, Object> response = getUserInfo(userId);

                FullHttpResponse httpResponse = createHttpResponse(objectMapper.writeValueAsString(response));
                ctx.writeAndFlush(httpResponse);
            } else {
                ctx.writeAndFlush(createHttpResponse("404 Not Found", HttpResponseStatus.NOT_FOUND));
            }
        }

        private Map<String, Object> getUserInfo(int userId) {
            // 模拟从数据库或其他服务获取用户信息
            Map<String, Object> userInfo = new HashMap<>();
            userInfo.put("id", userId);
            userInfo.put("name", "User" + userId);
            userInfo.put("email", "user" + userId + "@example.com");
            return userInfo;
        }

        private FullHttpResponse createHttpResponse(String content) {
            return createHttpResponse(content, HttpResponseStatus.OK);
        }

        private FullHttpResponse createHttpResponse(String content, HttpResponseStatus status) {
            FullHttpResponse response = new DefaultFullHttpResponse(
                    HttpVersion.HTTP_1_1, status,
                    Unpooled.copiedBuffer(content, CharsetUtil.UTF_8)
            );
            response.headers().set(HttpHeaderNames.CONTENT_TYPE, "application/json; charset=UTF-8");
            response.headers().setInt(HttpHeaderNames.CONTENT_LENGTH, response.content().readableBytes());
            return response;
        }

        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
            cause.printStackTrace();
            ctx.close();
        }
    }
}
```  

下面是 curl 示例：     
```
curl -X POST http://localhost:8080/netty/api/getUserInfo -H "Content-Type: application/json" -d '{"id":1}'
```

有了上面示例，我们将其打包好后部署在 RockyLinux94。  

## 测试 java 示例   
>官网doc 的一句描述： `Do this by substituting the jdb command for the java command in the command line.`（通过在命令行中用 jdb 命令替换 java 命令来完成此操作）        

回顾一下使用 java 命令：        
```shell
# -jar 参数 
java -jar xxxxx.jar   

# -cp 参数 
java -cp  xxxxx.jar MainClass    
```

那么使用 jdb，是如何使用的呢：  
```shell 
# -classpath 指定 jar  
jdb -classpath javamain-netty-1.0-SNAPSHOT.jar com.javamain.netty.example02.NettyHttpServer  
```  

## Start a JDB Session（开启 jdb）     
创建 JDB Session 有两种方式：         
**1.** 使用 JDB 替代 JAVA 启动程序     
**2.** 使用 JDB Remote JVM Debug               

![javajdb04](images/javajdb04.png)               

下面来分别介绍它们。    

### 使用 jdb 替代 java 启动程序    
执行 `jdb -classpath javamain-netty-1.0-SNAPSHOT.jar com.javamain.netty.example02.NettyHttpServer` 命令后，终端会进入一个交互输入的界面： 
![](images/javajdb02.png)     

```bash
[root@vm03 service]# jdb -classpath javamain-netty-1.0-SNAPSHOT.jar com.javamain.netty.example02.NettyHttpServer
Initializing jdb ...
>
```

此时输入 `run`   

![javajdb03](images/javajdb03.png)   

回车后，会发现终端会打印 Netty Http Server启动 log：            
```bash
024-09-21 20:26:06  [ PooledByteBufAllocator.java:159 ] - [ DEBUG ]  -Dio.netty.allocator.cacheTrimInterval: 8192
2024-09-21 20:26:06  [ PooledByteBufAllocator.java:160 ] - [ DEBUG ]  -Dio.netty.allocator.cacheTrimIntervalMillis: 0
2024-09-21 20:26:06  [ PooledByteBufAllocator.java:161 ] - [ DEBUG ]  -Dio.netty.allocator.useCacheForAllThreads: true
2024-09-21 20:26:06  [ PooledByteBufAllocator.java:162 ] - [ DEBUG ]  -Dio.netty.allocator.maxCachedByteBuffersPerChunk: 1023
2024-09-21 20:26:06  [ ByteBufUtil.java:84 ] - [ DEBUG ]  -Dio.netty.allocator.type: pooled
2024-09-21 20:26:06  [ ByteBufUtil.java:93 ] - [ DEBUG ]  -Dio.netty.threadLocalDirectBufferSize: 0
2024-09-21 20:26:06  [ ByteBufUtil.java:96 ] - [ DEBUG ]  -Dio.netty.maxThreadLocalCharBufferSize: 16384
2024-09-21 20:26:06  [ AbstractInternalLogger.java:150 ] - [ INFO ]  [id: 0x5569cc3e] REGISTERED
2024-09-21 20:26:06  [ AbstractInternalLogger.java:150 ] - [ INFO ]  [id: 0x5569cc3e] BIND: 0.0.0.0/0.0.0.0:8080
Server started, listening on 8080
2024-09-21 20:26:06  [ AbstractInternalLogger.java:150 ] - [ INFO ]  [id: 0x5569cc3e, L:/0:0:0:0:0:0:0:0:8080] ACTIVE
```

此时进行 curl 命令请求接口，查看服务是否启动正常。       
```bash         
[root@vm03 ~]# curl -X POST http://localhost:8080/netty/api/getUserInfo -H "Content-Type: application/json" -d '{"id":1}'
{"name":"User1","id":1,"email":"user1@example.com"}
```

### 使用 JDB Remote JVM Debug      
**1.** 编写 `start.sh`，内容如下：         
```shell
#!/bin/bash

nohup java -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=*:5005 -cp javamain-netty-1.0-SNAPSHOT.jar com.javamain.netty.example02.NettyHttpServer > /dev/null 2>&1 &
```  

**2.** 执行`sh start.sh` 命令，启动 Netty Http Server。再调用 curl 请求接口，判断服务是否正常启动。     

**3.** 执行 `jdb -attach 5005` 访问远程调试端口     
![javajdb05](images/javajdb05.png)

因为远程调试时可通过`suspend=y/n` 配置控制服务可调试的启动阶段不同，所以远程调试与 使用 jdb 替代 java 启动程序在`代码调试范围没有什么差异`。   

## Breakpoints（断点）   
>Breakpoints can be set in the JDB at line numbers or at the first instruction of a method, for example:    
	•	The command stop at MyClass:22 sets a breakpoint at the first instruction for line 22 of the source file containing MyClass.     
	•	The command stop in java.lang.String.length sets a breakpoint at the beginning of the method java.lang.String.length.      
	•	The command stop in MyClass.<clinit> uses <clinit> to identify the static initialization code for MyClass.  

>When a method is overloaded, you must also specify its argument types so that the proper method can be selected for a breakpoint. For example, MyClass.myMethod(int,java.lang.String) or MyClass.myMethod().   

>The clear command removes breakpoints using the following syntax: clear MyClass:45. Using the clear or stop command with no argument displays a list of all breakpoints currently set. The cont command continues execution.      

### stop at (eg: MyClass:22)          
我们将断点打到 `com.javamain.netty.example02.NettyHttpServer:76`      
```java
stop at com.javamain.netty.example02.NettyHttpServer:76     
```


### stop in (eg: java.lang.String.length)

```bash
[root@vm03 service]# jdb -attach 5005
Set uncaught java.lang.Throwable
Set deferred uncaught java.lang.Throwable
Initializing jdb ...
> stop
No breakpoints set.
> stop in java.lang.String.length
Set breakpoint java.lang.String.length
> stop
Breakpoints set:
        breakpoint java.lang.String.length

```

### stop in (eg: MyClass.<clinit>)





 

refer       
1.https://docs.oracle.com/en/java/javase/11/tools/jdb.html    
