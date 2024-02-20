## Flink 二开 增加 REST API 接口    

>Flink version: 1.15.4   

### 引言    
Flink 提供大量的 REST API 工具请求帮助我们完成对 Flink Job的管理。JobManager 中的 `webserver`模块 提供 REST API。默认端口是8081，关于 REST API 配置可通过 `conf/flink-conf.yaml` 进行配置。   

在有多个JobManager的情况下(HA场景下)，`每个JobManager都运行自己的 REST API实例`，而由被选为leader的JobManager实例提供有关已完成和正在运行的作业的信息。 

### 开发指南    
REST API 位于 `flink-runtime` 项目下，核心实现org.apache.flink.runtime.webmonitor.WebMonitorEndpoint (因为Flink早期REST API都是用于监控，所以命名是WebMonitorEndpoint。现在其工作职能还包含一些任务启停等非监控场景)，其主要是负责server实现和请求路由。  


### 开发自己的 REST API 

#### 设计与规划 
##### 1.需求
向 http 链接 http://${jobmaster-host}:8081/pierre/foo 发起 GET 请求，返回一个 json 字符串
```json
{"response":"bar"}
```  

##### 2.实现规划    
当我们要新增加一个 REST API 的时候，我们至少需要：  
* 在 `org.apache.flink.runtime.rest.messages`包下，实现一个 `MessageHeaders`,作为新请求的接口,      
* 实现一个`ResponseBody`,作为返回结果的 Body    
* 实现一个`AbstractRestHandler`,根据添加的`MessageHeaders类`处理请求 
* 将 handler 注册到 `org.apache.flink.runtime.webmonitor.WebMonitorEndpoint#initializeHandlers()`   

>以上操作，除了 handler 注册，其他都在 flink-runtime 模块在的 `org.apache.flink.runtime.rest.messages.pierre`（记得创建 pierre 包命） 包在实现

#### 实现细节 

##### 1.实现 MessageHeaders 
```java

``` 
>注意: 必须是单实例模式; HttpResponseStatus、getResponseClass等均不能return null，否则会有`NullPointerException`   

##### 2.实现 ResponseBody  
```java

``` 
这里使用到了 jackson注解，需要import FLINK shaded的版本，避免冲突       

##### 3.实现 AbstractRestHandler    
```java

```

##### 4.注册handler 
```java

```

##### 5.编译打包 


refer   
1.https://cloud.tencent.com/developer/article/1833460       

