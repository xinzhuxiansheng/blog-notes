## Openjob Worker Akka 启动过程 

### OpenjobWorker#doInitialize()  
在 `openjob-samples` https://github.com/open-job/openjob-samples 项目中，可以很清楚看到Openjob Worker的接入入口。 这里先简单描述下 Openjob的worker组成部分：  
`Worker`的构造      
![workerstart01](images/workerstart01.png)      

```java
public static void main(String[] args) {
    try {
        OpenjobWorker openjobWorker = new OpenjobWorker();
        openjobWorker.init();

        Thread.currentThread().join();
    } catch (Exception e) {
        e.printStackTrace();
    }
}

public synchronized Boolean doInitialize() {
    try {
        this.workerChecker.init();
        this.workerConfig.init();
        this.workerActorSystem.init();
        this.workerRegister.register();
        this.workerHeartbeat.init();
        this.workerInitializer.init();
        this.workerShutdown.init();
        return true;
    } catch (Throwable var2) {
        log.error("Openjob worker initialize failed!", var2);
        return false;
    }
}
```

### OpenjobWorker 实现 InitializingBean 简化Spring 接入
在Spring框架中，`InitializingBean`接口定义了一个方法：`afterPropertiesSet()`。当一个bean实现了这个接口，并且所有bean属性都已经被设置之后，这个方法将被Spring容器自动调用。  
实现`InitializingBean`接口允许bean在Spring容器设置完所有属性后执行某些初始化逻辑。这是一个在bean完全初始化之后，执行初始化任务（如资源分配、自定义初始化逻辑或依赖校验等）的标准Spring回调方法。    
这里是`InitializingBean`接口的示例：    

```java
public class ExampleBean implements InitializingBean {
    // bean properties and setters
    @Override
    public void afterPropertiesSet() throws Exception {
        // custom initialization logic here
    }
}
```
当Spring容器实例化这个`ExampleBean`类的一个对象，并设置完所有属性后，`afterPropertiesSet()`方法将被自动调用，你可以在这个方法里加入你需要的初始化逻辑。  

>回到 OpenjobWorker#doInitialize()方法来。 下面通过 `this.workerRegister.register()` 讲解 Worker是如何与Master发起请求的.   

### 案例一 this.workerRegister.register() 通信处理逻辑  
```java
public void register() {
    String serverAddress = WorkerConfig.getServerHost();

    WorkerStartRequest startReq = new WorkerStartRequest();
    startReq.setAddress(WorkerConfig.getWorkerAddress());
    startReq.setAppName(WorkerConfig.getAppName());
    startReq.setProtocolType(ProtocolTypeEnum.AKKA.getType());

    try {
        ServerWorkerStartResponse response = FutureUtil.mustAsk(WorkerUtil.getServerWorkerActor(), startReq, ServerWorkerStartResponse.class, 15000L);
        log.info("Register worker success. serverAddress={} workerAddress={}", serverAddress, WorkerConfig.getWorkerAddress());

        // Do register.
        this.doRegister(response);
    } catch (Throwable e) {
        log.error("Register worker fail. serverAddress={} workerAddress={}", serverAddress, WorkerConfig.getWorkerAddress());
        throw e;
    }
}
```
上面是`register()`方法，在代码中不难看出`ServerWorkerStartResponse response = FutureUtil.mustAsk(WorkerUtil.getServerWorkerActor(), startReq, ServerWorkerStartResponse.class, 15000L);` 是处理请求逻辑的核心方法。 FutureUtil.mustAsk() 特别像HttpUtil工具类一样，在Java HttpClient中经常定义GET或者POST来请求，而在Akka中通过Ask()发起网络请求。      

我们继续用HttpClient理解，要定义通用的GET/POST方法，我们需要知道`请求地址`，`请求参数`，`定义返回值（通常返回String，而外部方法体中处理String转换实体逻辑）`，`超时时间`。 **若读者这里还没有了解过，那可自己封装一个HttpUtil**







refer   
1.https://openjob.io/zh-Hans/docs/intro             
