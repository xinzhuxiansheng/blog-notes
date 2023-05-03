## Spring Boot服务实现优雅停机

>官方文档: https://docs.spring.io/spring-boot/docs/2.7.11/reference/htmlsingle/#web.graceful-shutdown

### 引言
在Server提供服务过程中，若直接停掉服务（ps：类似于 kill -9），这会导致正在访问的请求出现中断，无法正常返回数据。即使在Nginx或者Apisix等开源做反向代理时，经常会遇到Server bug修复、升级等需要重新发版的操作。故保证用户请求完整性这点至关重要。 所以在`SpringBoot 从2.3.0.RELEASE 开始支持 web 服务器的优雅停机`。

### 介绍
所有四种嵌入式 Web 服务器（Jetty、Reactor Netty、Tomcat 和 Undertow）以及反应式和基于servlet的Web应用程序都支持正常关闭。它作为关闭应用程序上下文的一部分出现，并在停止 SmartLifecycle bean 的最早阶段执行。此停止处理使用超时提供宽限期，在此期间允许完成现有请求但不允许新请求。不允许新请求的确切方式因所使用的 Web 服务器而异。 Jetty、Reactor Netty 和 Tomcat 将停止在网络层接受请求。 Undertow 将接受请求，但会立即响应服务不可用 (503) 响应。

>注意：优雅停机需要Tomcat版本在9.0.33及以上版本（Graceful shutdown with Tomcat requires Tomcat 9.0.33 or later. ）

### 操作
**1.**要启用优雅关机，请配置server.shutdown属性，如以下示例所示： 
```
server.shutdown=graceful
```
而在spring boot server.shutdown配置项默认的参数值是`immediate`

**2.**要配置超时时间，请配置 spring.lifecycle.timeout-per-shutdown-phase属性，如以下示例所示： 
```
spring.lifecycle.timeout-per-shutdown-phase=20s
```

