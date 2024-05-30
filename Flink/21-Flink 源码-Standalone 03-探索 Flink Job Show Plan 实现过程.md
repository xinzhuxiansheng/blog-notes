# Flink 源码 - Standalone - 探索 Flink Job Show Plan 实现过程             

>Flink version: 1.17.2          

>注意，该篇 Blog 涉及到的“点”较多,阅读源码过程要实践“第一性原理”,似乎可行性不高，学习的过程像是剥洋葱一样，从外到内，也无法拿着`Flink confluence FLIP`(https://cwiki.apache.org/confluence/display/FLINK/Flink+Improvement+Proposals)对着它的 `Motivation`直接理解，若做到 do-find-why 似乎可行性较高。      

>如果我有表述的不清楚，还麻烦大家给我留言,帮忙修订它。       

## 引言     
在之前的 Blog 内容中，部署了一些 Flink Job，对下面的图(`红色框标记的`)肯定有所了解：         
![showplan13](images/showplan13.png)        

它是 Flink Job 的 `Job Graph 拓扑图`，它展示了 Job 的执行计划，拓扑图显示了数据流通过各个算子（operators）的路径，以及每个算子的并行度（Parallelism）。 `It's very important`，它可以帮助我们理解 Job的结构和处理过程，它显示的并行度，可帮助我们优化资源使用和提高处理效率，在后面的 Blog 中，我们会很长时间围绕这 `JobGraph`。         

>在之前的 Blog "Flink 源码 - Standalone - Idea 启动 Standalone 集群 (Session Model)" 内容中提到 `Flink Architecture 的拼图游戏`，那 ”Show Plan“ 涉及到哪些角色呢？  

接下来，我们通过 Job 示例，探索 Show Plan 实现过程，并最终画到架构图上：                
![showplan14](images/showplan14.png)             

## 开发 Stream WordCount 作业        
关于从零开始搭建 Flink Job 开发项目，可参考Flink 官网给出的模板示例： https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/configuration/maven/，以下是我的 Flink Job 开发项目的构造过程：         

### 添加 Flink 相关依赖         
```xml
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-streaming-java</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-runtime-web</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>

<build>
    <plugins>
        <plugin>
            <groupId>org.apache.maven.plugins</groupId>
            <artifactId>maven-assembly-plugin</artifactId>
            <version>3.0.0</version>
            <configuration>
                <descriptorRefs>
                    <descriptorRef>jar-with-dependencies</descriptorRef>
                </descriptorRefs>
            </configuration>
            <executions>
                <execution>
                    <id>make-assembly</id>
                    <phase>package</phase>
                    <goals>
                        <goal>single</goal>
                    </goals>
                </execution>
            </executions>
        </plugin>

        <plugin>
            <groupId>org.apache.maven.plugins</groupId>
            <artifactId>maven-compiler-plugin</artifactId>
            <version>3.8.0</version>
            <configuration>
                <source>${target.java.version}</source>
                <target>${target.java.version}</target>
                <encoding>UTF-8</encoding>
            </configuration>
        </plugin>
    </plugins>
</build>
```     

### StreamWordCount 示例      
实现的逻辑是 读取 Socket 文本数据，根据数据的“空格” 进行拆分，然后对 word 进行分组求和。            

```java
public class StreamWordCount {
    private static Logger logger = Logger.getLogger(StreamWordCount.class);

    public static void main(String[] args) throws Exception {
        // 1. 创建流式执行环境
//        StreamExecutionEnvironment env = StreamExecutionEnvironment
//                .createLocalEnvironmentWithWebUI(new Configuration());
        StreamExecutionEnvironment env = StreamExecutionEnvironment   // 非本地运行，使用 getExecutionEnvironment()   
                .getExecutionEnvironment(new Configuration());
        env.setRestartStrategy(RestartStrategies
                .fixedDelayRestart(3, Time.of(10, TimeUnit.SECONDS)));
        // 2. Socket 读取  nc -lk 7777
        DataStreamSource<String> lineDSS = env
                .socketTextStream("localhost", 7777);

        // 3. 转换数据格式
        SingleOutputStreamOperator<Tuple2<String, Long>> wordAndOne = lineDSS
                .flatMap((String line, Collector<String> words) -> {
                    Arrays.stream(line.split(" ")).forEach(words::collect);
                })
                .returns(Types.STRING)
                .map(word -> Tuple2.of(word, 1L))
                .returns(Types.TUPLE(Types.STRING, Types.LONG)).setParallelism(2);

        // 4. 分组
        KeyedStream<Tuple2<String, Long>, String> wordAndOneKS = wordAndOne
                .keyBy(t -> t.f0);
        // 5. 求和
        SingleOutputStreamOperator<Tuple2<String, Long>> result = wordAndOneKS
                .sum(1).setParallelism(1);

        // 6. 打印
        result.print();
        logger.info(result.toString());
        // 7. 执行
        env.execute();
    }
}
```

### 打包        
打包后，产出 `flink-blog-1.0-SNAPSHOT-jar-with-dependencies.jar`, 完整项目示例可访问 https://github.com/xinzhuxiansheng/flink-tutorial/tree/main/flink-blog。    
```shell        
mvn clean package       
```                 

## How to view job plan    
查看 Job Plan 有两种方式，一个是使用 CLI 命令行，另一种是浏览 Flink WEB UI 页面。下面，介绍两种查看方式。               

### CLI 查看 Job Plan     
使用 `./flink info` 查看 Job Graph，可访问官网 `https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/deployment/cli/` 查看 ./flink info 更多介绍。               

```shell
[root@vm01 bin]# ./flink info -h   
Action "info" shows the optimized execution plan of the program (JSON).   
  Syntax: info [OPTIONS] <jar-file> <arguments>  
  "info" action options:
     -c,--class <classname>           Class with the program entry point
                                      ("main()" method). Only needed if the JAR
                                      file does not specify the class in its
                                      manifest.   
     -p,--parallelism <parallelism>   The parallelism with which to run the
                                      program. Optional flag to override the
                                      default value specified in the
                                      configuration.   
```

**执行命令**        
```shell
./flink info -c com.yzhou.blog.wordcount.StreamWordCount TMP/flink-blog-1.0-SNAPSHOT-jar-with-dependencies.jar   
```

Output log:             
```shell
[root@vm01 bin]# ./flink info -c com.yzhou.blog.wordcount.StreamWordCount TMP/flink-blog-1.0-SNAPSHOT-jar-with-dependencies.jar 
WARNING: An illegal reflective access operation has occurred
WARNING: Illegal reflective access by org.apache.flink.api.java.ClosureCleaner (file:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-dist-1.17.2.jar) to field java.lang.String.value
WARNING: Please consider reporting this to the maintainers of org.apache.flink.api.java.ClosureCleaner
WARNING: Use --illegal-access=warn to enable warnings of further illegal reflective access operations
WARNING: All illegal access operations will be denied in a future release
----------------------- Execution Plan -----------------------
{
  "nodes" : [ {
    "id" : 1,
    "type" : "Source: Socket Stream",
    "pact" : "Data Source",
    "contents" : "Source: Socket Stream",
    "parallelism" : 1
  }, {
    "id" : 2,
    "type" : "Flat Map",
    "pact" : "Operator",
    "contents" : "Flat Map",
    "parallelism" : 1,
    "predecessors" : [ {
      "id" : 1,
      "ship_strategy" : "FORWARD",
      "side" : "second"
    } ]
  }, {
    "id" : 3,
    "type" : "Map",
    "pact" : "Operator",
    "contents" : "Map",
    "parallelism" : 2,
    "predecessors" : [ {
      "id" : 2,
      "ship_strategy" : "REBALANCE",
      "side" : "second"
    } ]
  }, {
    "id" : 5,
    "type" : "Keyed Aggregation",
    "pact" : "Operator",
    "contents" : "Keyed Aggregation",
    "parallelism" : 1,
    "predecessors" : [ {
      "id" : 3,
      "ship_strategy" : "HASH",
      "side" : "second"
    } ]
  }, {
    "id" : 6,
    "type" : "Sink: Print to Std. Out",
    "pact" : "Data Sink",
    "contents" : "Sink: Print to Std. Out",
    "parallelism" : 1,
    "predecessors" : [ {
      "id" : 5,
      "ship_strategy" : "FORWARD",
      "side" : "second"
    } ]
  } ]
}
--------------------------------------------------------------

No description provided.
```     

### Flink WEB UI 查看 Job Plan      

**Job 未提交前，查看 Job Plan**         

![showplan15](images/showplan15.png)    

**Job 提交后，查看 Job Plan**   

![showplan16](images/showplan16.png)        


>注意，若你实践过后，一定要得到一个结论：CLI 返回的 Plan JSON信息 并不与 Standalone 集群通信，所以它独立在 Flink Client 完成的。 这点无法在 Flink WEB UI 验证，因为 Flink WEB UI 是由 JobManager 的 Netty Server 提供的。         

![showplan17](images/showplan17.png)        

其实，这个结论在 官网也有提到以下内容 :  (https://nightlies.apache.org/flink/flink-docs-master/docs/internals/job_scheduling/)      

```bash
The JobManager receives the JobGraph , which is a representation of the data flow consisting of operators ( JobVertex ) and intermediate results ( IntermediateDataSet ). Each operator has properties, like the parallelism and the code that it executes. In addition, the JobGraph has a set of attached libraries, that are necessary to execute the code of the operators.
```  
 
JobManager 接受到 JobGraph ..., 那说明 Flink Client 在提交 Job 的时候，会带有 JobGraph 参数给 JobManager。      
![showplan19](images/showplan19.png)        


## 配置源码调试     
`在后面的内容学习过程中，当我一筹莫展的时候，唯有 Debug 解惑`。It's very important.        

### CLI 调试    
该步骤可参考之前 Blog “Flink 源码 - Standalone - 通过 CliFrontend 提交 Job 到 Standalone 集群” 的配置项，但需修改 “Program arguments” 参数项为：`info
-c
com.yzhou.blog.wordcount.StreamWordCount
D:\Code\Java\flink-tutorial\flink-blog\target\flink-blog-1.0-SNAPSHOT-jar-with-dependencies.jar`        

### Flink WEB UI 远程调试 
该步骤可参考之前 Blog “Flink 源码 - Standalone - 通过 CliFrontend 提交 Job 到 Standalone 集群” 对 conf/flink-conf.yaml的 `env.java.opts.jobmanager` 参数配置。          

>注意，关于 Flink WEB UI的 API 服务是由 JobManager 的Netty Sever 提供的，在调试`Show Plan` 功能，我们还需找到它对应的 Handler。 接下来，我简单介绍下，也可减少大家定位代码的时间成本。下面是 Netty Server的结构图：     

![showplan12](images/showplan12.png)        

可阅读 `RestServerEndpoint#start()` 了解 Netty Server的启动过程，那么 Show Plan 对应的 Handler 是 `JarPlanHandler`, 处理逻辑在 `handleRequest()`; 下面是 JarPlanHandler#handleRequest()具体代码：       
```java
@Override
protected CompletableFuture<JobPlanInfo> handleRequest(
        @Nonnull final HandlerRequest<JarPlanRequestBody> request,
        @Nonnull final RestfulGateway gateway)
        throws RestHandlerException {
    final JarHandlerContext context = JarHandlerContext.fromRequest(request, jarDir, log);
    final Configuration effectiveConfiguration = new Configuration(this.configuration);
    context.applyToConfiguration(effectiveConfiguration, request);

    return CompletableFuture.supplyAsync(
            () -> {
                try (PackagedProgram packagedProgram =
                                context.toPackagedProgram(effectiveConfiguration)) {
                    final JobGraph jobGraph =
                            context.toJobGraph(packagedProgram, effectiveConfiguration, true);
                    return planGenerator.apply(jobGraph);
                }
            },
            executor);
}
```

到此，我就不花篇幅说明 Netty Server是如何构造的，我们还是要回到主线`Show Plan`。                 

## Debug Show Plan Code      
>我采用的是 Flink WEB UI 远程调试, 如下图所示：         
![showplan15](images/showplan15.png) 

### PackagedProgram                  
**JarPlanHandler#handleRequest()**      
```java     
@Override
protected CompletableFuture<JobPlanInfo> handleRequest(
        @Nonnull final HandlerRequest<JarPlanRequestBody> request,
        @Nonnull final RestfulGateway gateway)
        throws RestHandlerException {

    ...... 省略部分代码    

    return CompletableFuture.supplyAsync(
            () -> {
                try (PackagedProgram packagedProgram =
                        context.toPackagedProgram(effectiveConfiguration)) {  
                    
                    ...... 省略部分代码   

                }
            },
            executor);
}     
```

### 创建 PackagedProgram userCodeClassLoader    
PackagedProgram 在它的构造方法中，创建了一个自定义类加载器 `FlinkUserCodeClassLoader userCodeClassLoader`。 `FlinkUserCodeClassLoaders#create()`方法会根据 `classloader.resolve-order` 配置项来决定创建的 ChildFirstClassLoader 还是 ParentFirstClassLoader, 这样是为了定义从用户代码加载类时的类解析策略，即是先检查用户代码 jar（“child-first”）还是应用程序类路径（“parent-first”）。默认设置是先从用户代码 jar 加载类，这意味着用户代码 jar 可以包含和加载与 Flink 使用的不同的依赖项。关于这部分的参数可访问 `https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/deployment/config/#classloader-resolve-order`。      

![showplan03](images/showplan03.png)  

ChildFirstClassLoader 、 ParentFirstClassLoader 是 FlinkUserCodeClassLoader的派生类，且父类 MutableURLClassLoader 继承了 URLClassLoader，也重写了 addURL() 方法，这样就可以动态新的 URL，再使用 loadCLass()方法加载类，从而达到动态 class的效果。          
**FlinkUserCodeClassLoader 类图：**           
![showplan02](images/showplan02.png)

![showplan01](images/showplan01.png)           

显然，`child-first` ChildFirstClassLoader 打破了双亲机制，在 `ChildFirstClassLoader#loadClassWithoutExceptionHandling()`方法，首先判断当前class是否已加载，若没有，再去判断当前的 class 是否符合`classloader.parent-first-patterns.default`参数的 package path，
如果是 true，则会走父类加载。          
```java
@Override
protected Class<?> loadClassWithoutExceptionHandling(String name, boolean resolve)
        throws ClassNotFoundException {

    ...... 省略部分代码 
        // check whether the class should go parent-first
        for (String alwaysParentFirstPattern : alwaysParentFirstPatterns) {
            if (name.startsWith(alwaysParentFirstPattern)) {
                return super.loadClassWithoutExceptionHandling(name, resolve);
            }
        }
    ...... 省略部分代码 
}
```     

在上面代码中的 alwaysParentFirstPatterns 集合变量，可通过参数配置，参考 Flink 官网 `Configuration`文档，可访问 `https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/deployment/config/#classloader-parent-first-patterns-default`    

![showplan20](images/showplan20.png)    


>双亲委派（Parent Delegation）是 Java 类加载机制中的一种重要的原则，用于保证类的唯一性和安全性。该机制要求类加载器在加载类时首先委派给父类加载器，只有在父类的加载器无法加载该类时，才由子类加载器尝试加载。    
![showplan05](images/showplan05.png)      

对于其他类，先查找 Class，这部分对应的是 ChildFirstClassLoader的构造 方法，传入了 Flink Job的 Jar, 所以 findClass(name)的结果，是可以找到 `com.yzhou.blog.wordcount.StreamWordCount.class`。             
![showplan06](images/showplan06.png)        

![showplan07](images/showplan07.png)        

若没有找到，则委托父类加载`c = super.loadClassWithoutExceptionHandling(name, resolve);`         
ParentFirstClassLoader 类就不需要过多介绍，它遵循的是双亲委派，下面列出
`ChildFirstClassLoader#loadClassWithoutExceptionHandling()` Code：          

**ChildFirstClassLoader#loadClassWithoutExceptionHandling()**           
```java
@Override
protected Class<?> loadClassWithoutExceptionHandling(String name, boolean resolve)
        throws ClassNotFoundException {

    // First, check if the class has already been loaded
    Class<?> c = findLoadedClass(name);

    if (c == null) {
        // check whether the class should go parent-first
        for (String alwaysParentFirstPattern : alwaysParentFirstPatterns) {
            if (name.startsWith(alwaysParentFirstPattern)) {
                return super.loadClassWithoutExceptionHandling(name, resolve);
            }
        }

        try {
            // check the URLs
            c = findClass(name);
        } catch (ClassNotFoundException e) {
            // let URLClassLoader do it, which will eventually call the parent
            c = super.loadClassWithoutExceptionHandling(name, resolve);
        }
    } else if (resolve) {
        resolveClass(c);
    }

    return c;
}
```

>关于 Class Loading 的配置部分，Flink 官网 doc 也单独列出来，可访问 `https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/deployment/config/#class-loading` 了解更多。                                        

### 创建 PackagedProgram mainClass            
该章节内容的介绍 与 上一章节 "创建 PackagedProgram userCodeClassLoader"的内容是联动的， 正如它的实现一样，是使用 userCodeClassLoader 来加载 mainClass。 
```java
this.mainClass =
    loadMainClass(
            // if no entryPointClassName name was given, we try and look one up through
            // the manifest
            entryPointClassName != null
                    ? entryPointClassName
                    : getEntryPointClassNameFromJar(this.jarFile),
            userCodeClassLoader);       
```

`使用自定义的ClassLoader（并打破双亲委托机制），其目的是为了保障 class 的唯一性`，避免因为 lib version 不一致，出现类冲突，例如 ClassNotFoundException、NoSuchMethodError等 异常出现。 

下面补充一个概念：                      
>类的唯一性是如何确定？       
Java 类的唯一性是通过类加载器和类的全限定名来确定的。   
1. **类加载器**：每个类加载器在 JVM 中都是唯一的，不同的类加载器加载同一个类会产生不同的类对象。因此，类的唯一性与加载它的类加载器密切相关。  
2. **类的全限定名**：类的全限定名包括包名和类名，形如 `com.example.MyClass`。在同一个类加载器中，类的全限定名必须是唯一的。如果两个类具有相同的全限定名，它们会被视为同一个类。  
综合来说，Java 类的唯一性由类加载器和类的全限定名共同确定。在同一个类加载器中，类的全限定名必须是唯一的，否则会导致类的冲突。    

`这里特别说明一点`，为了不破坏原有线程的 ClassLoader，当加载 MainClass 时设置线程的 classloader 为 FlinkUserCodeClassLoader, 处理完后再将原来的 ClassLoader 设置回去。`这点特别重要，这么做的好处是，既保证了 “当前线程” 需要 “加载的类” 的唯一性，又不破坏 JVM 原有的 ClassLoader实现。` 下面列出 ClassLoader 切换的 Code：                        
**PackagedProgram#loadMainClass()**      
```java
private static Class<?> loadMainClass(String className, ClassLoader cl)
        throws ProgramInvocationException {
    ClassLoader contextCl = null;
    try {
        contextCl = Thread.currentThread().getContextClassLoader();
        Thread.currentThread().setContextClassLoader(cl);
        return Class.forName(className, false, cl);
    
    ...... 省略部分代码 

    } finally {
        // 在 finnally 部分，在赋值为原来的 ClassLoader
        if (contextCl != null) {
            Thread.currentThread().setContextClassLoader(contextCl);
        }
    }
}
```

大家可阅读 `DataX 项目 CLassLoaderSwapper.java`的实现。 代码示例如下，它使用了同样的 classloader 切换逻辑， 这对于一个动态任务架构的服务来说，特别经典的处理方式。      
**CLassLoaderSwapper#setCurrentThreadClassLoader()**        
```java
/**
 * 保存当前classLoader，并将当前线程的classLoader设置为所给classLoader
 *
 * @param
 * @return
 */
public ClassLoader setCurrentThreadClassLoader(ClassLoader classLoader) {
    this.storeClassLoader = Thread.currentThread().getContextClassLoader();
    Thread.currentThread().setContextClassLoader(classLoader);
    return this.storeClassLoader;
}
```

到此，PackagedProgram.newBuilder() 构建的重要部分已介绍的差不多了，接下来，探讨 JobGraph 的构造过程。                  

## JobGraph     

**JarPlanHandler#handleRequest()**   
```java
@Override
protected CompletableFuture<JobPlanInfo> handleRequest(
        @Nonnull final HandlerRequest<JarPlanRequestBody> request,
        @Nonnull final RestfulGateway gateway)
        throws RestHandlerException {
    
    ...... 省略部分代码 

    return CompletableFuture.supplyAsync(
            () -> {
                try (...... 省略部分代码 ) {
                    final JobGraph jobGraph =
                            context.toJobGraph(packagedProgram, effectiveConfiguration, true);
                    return planGenerator.apply(jobGraph);
                }
            },
            executor);
}  
```         

下面给出 JobGraph 构造时序图                
![showplan08](images/showplan08.png)             

JobGraph的构造主要部分在`PackagedProgramUtils#createJobGraph(...)`方法的内部，接下来，我们来重点讲解这部分的逻辑。             

### 创建 Pipeline      
![showplan09](images/showplan09.png)    

**PackagedProgramUtils#createJobGraph(......)**       
```java
final Pipeline pipeline =
        getPipelineFromProgram(
                packagedProgram, configuration, defaultParallelism, suppressOutput);
``` 

`PackagedProgramUtils#getPipelineFromProgram()`方法实现逻辑让我花了些时间去思考。       
**PackagedProgramUtils#getPipelineFromProgram() 时序图**
![showplan10](images/showplan10.png)   

从 benv 、senv 对象的创建，到 `program#invokeInteractiveModeForExecution()` 内部调用 `PackagedProgram#callMainMethod()` 执行 Flink Job 的 main() 方法。 但在调试过程中发现，callMainMethod() 总是会抛出 `org.apache.flink.client.program.ProgramAbortException`。如下图所示：  
![showplan21](images/showplan21.png)                 

通过 Idea 查看 `ProgramAbortException`的 usages，如下图所示：        
![showplan22](images/showplan22.png)        

OptimizerPlanEnvironment、StreamPlanEnvironment 正好对应的 benv、senv。分别在它们的 executeAsync() 方法打了断点，可知 
callMainMethod() 异常是由 `StreamPlanEnvironment#executeAsync()` 抛出。**那是不是意味着 Flink Job StreamWordCount的 main() 在执行的过程中，会调用`StreamPlanEnvironment#executeAsync()`?**。

是的，Debug 过程中，断点进入`StreamPlanEnvironment#executeAsync()`后，通过 Idea 查看 JVM虚拟机栈，可以看到 main()的栈帧，这足够证明 StreamPlanEnvironment 与 Flink Job 的 `env.execute()`的关联性，如下图所示：     
![showplan23](images/showplan23.png)    

接下来，了解 `StreamPlanEnvironment 与 Flink Job 的 env.execute()`是如何关联的。         

#### 1）Flink 源码绑定 StreamWordCount example 项目     
StreamWordCount.java 是篇章节 “开发 Stream WordCount 作业” 的示例代码，`PackagedProgram#callMainMethod()`通过发射调用`StreamWordCount#main()`方法，而 Flink 源码中是不存在`StreamWordCount.java`, 所以为了调试 StreamWordCount#main(), 我将 示例项目 copy 一份到 Flink的 `flink-examples` 模块下，结构如下图：     

![showplan24](images/showplan24.png)   

此时，在 StreamWordCount#main() 打上断点，重新在Flink Web UI 中点击 “Show Plan”，就可以调试了。如下图所示： 

![showplan25](images/showplan25.png)    

#### 2）StreamWordCount DataStream API 链路 与 Transformation 关系       
通过 DataStream<T> 类图可了解到，每个 DataStream<T>内部 都包含一个 Transformation 对象，在后续的代码中可以证实，`DataStream<T> 是 Flink User API 衔接转换的 Object，但其本质是构建出其内部的 Transformation 对象`如下图所示：       

![showplan31](images/showplan31.png)    

#### 3）DataStream 派生类中的 Transformation 属性会存放在 StreamExecutionEnvironment.transformations 集合中     

>transformations的类型是 List<Transformation<?>>  

如下图所示, 我将 StreamWordCount 分成两个模块。         
![showplan27](images/showplan27.png) 

在上图中`模块一`代码中，调用 .socketTextStream()、.flatMap()、.map()、.keyBy()、.sum()、.print() 方法, 它会创建不同类型的 DataStream<T> 派生类以及 它内部的 `Transformation<T> transformation` 属性。 StreamWordCount的 链路如下图所示：         

![showplan30](images/showplan30.png)   
![showplan29](images/showplan29.png)                    

Flink 官网文档的code 示例也将中间算子 Transformation 标记出来，可访问`https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/learn-flink/overview/#stream-processing`,如下图所示：   
![showplan32](images/showplan32.png)        

Flink 首先会将该链路算子转成 `Transformation`对象，存储在 `StreamExecutionEnvironment.transformations`集合中，如下图所示(标记出红框)：   
![showplan26](images/showplan26.png)                   

跟着上面的思路，那 flatMap、map、keyBy、sum 会创建其内部 Transformation 对象，那都会存在 `StreamExecutionEnvironment.transformations`集合中么 ？。                 

`但结果并不这样`，首先根据 Debug 结果可知，`StreamExecutionEnvironment.transformations` 集合长度为4，子项为：  
* OneInputTransformation flatMap   
* OneInputTransformation Map  
* ReduceTransformation Keyed Aggregation  
* LegacySinkTransformation Prnt to Std.Out      

并不包含 keyBy() 的 PartitionTransformation，下面是 keyBy 的 KeyedStream#KeyedStream()代码，显然它没有调用`StreamExecutionEnvironment#addOperator()`方法。         
```java
this(
    dataStream,
    new PartitionTransformation<>(
            dataStream.getTransformation(),
            new KeyGroupStreamPartitioner<>(
                    keySelector,
                    StreamGraphGenerator.DEFAULT_LOWER_BOUND_MAX_PARALLELISM)),
    keySelector,
    keyType);  
```

那这里我会有 2个疑惑？      
* 1.transformations集合 没有 keyBy()的 PartitionTransformation 如何保证 StreamWordCount DataStram API 链路的完整性 ？       
* 2.哪些 Transformation 该加入 `StreamExecutionEnvironment.transformations`集合中。         

**针对第一个疑惑**：Transformation<T> 抽象类中，它内部包含有一个 `public abstract List<Transformation<?>> getInputs()` 抽象方法，它的派生类内有一个 `Transformation<IN> input` 属性，再重写 getInputs(),返回 `Collections.singletonList(input)`。  

Transformation<IN> input 存放的是它上一个 DataStream<T> 的 input。 我们可以通过 flatMap 对应的 OneInputTransformation 对象来做演示，下面是创建 OneInputTransformation 对象的代码：  
```java
OneInputTransformation<T, R> resultTransform =
    new OneInputTransformation<>(
            this.transformation,    // 特别注意该形参值 
            operatorName,
            operatorFactory,
            outTypeInfo,
            environment.getParallelism(),
            false);
```

而 this.transformation 是 Source DataStream对象内部的 Transformation。  根据`引用传递`,那 `StreamExecutionEnvironment.transformations`集合 如下图所示：     
![showplan33](images/showplan33.png)  

所以 父节点 inputs 存放了 `PartitionTransformation （Partition）`。并且 每个 transformation 都保存了它 前面所有的 transformations 引用链路。        

**针对第二个疑惑**：我们知晓调用`StreamExecutionEnvironment#addOperator()`方法，会将 Transformation 对象添加到 `StreamExecutionEnvironment.transformations`集合中。         

通过 Idea 查看 addOperator() 方法引用，这太粗放了，至于添不添加，后面还是 case by case 再分析。          
![showplan34](images/showplan34.png)        

#### 3）List<Transformation<?>> transformations 构造 StreamGraph 
在`模块二`中，当执行 `env.execute();`, 是使用 `StreamExecutionEnvironment.transformations`集合来构造 StreamGraph的。 如下图所示：       

![showplan28](images/showplan28.png)      

`StreamGraphGenerator#generate()`   

>StreamGraphGenerator#translate() 方法中的形参 `translator`是什么？  
```java
static {
    @SuppressWarnings("rawtypes")
    Map<Class<? extends Transformation>, TransformationTranslator<?, ? extends Transformation>>
            tmp = new HashMap<>();
    tmp.put(OneInputTransformation.class, new OneInputTransformationTranslator<>());
    tmp.put(TwoInputTransformation.class, new TwoInputTransformationTranslator<>());
    tmp.put(MultipleInputTransformation.class, new MultiInputTransformationTranslator<>());
    tmp.put(KeyedMultipleInputTransformation.class, new MultiInputTransformationTranslator<>());
    tmp.put(SourceTransformation.class, new SourceTransformationTranslator<>());
    tmp.put(SinkTransformation.class, new SinkTransformationTranslator<>());
    tmp.put(LegacySinkTransformation.class, new LegacySinkTransformationTranslator<>());
    tmp.put(LegacySourceTransformation.class, new LegacySourceTransformationTranslator<>());
    tmp.put(UnionTransformation.class, new UnionTransformationTranslator<>());
    tmp.put(PartitionTransformation.class, new PartitionTransformationTranslator<>());
    tmp.put(SideOutputTransformation.class, new SideOutputTransformationTranslator<>());
    tmp.put(ReduceTransformation.class, new ReduceTransformationTranslator<>());
    tmp.put(
            TimestampsAndWatermarksTransformation.class,
            new TimestampsAndWatermarksTransformationTranslator<>());
    tmp.put(BroadcastStateTransformation.class, new BroadcastStateTransformationTranslator<>());
    tmp.put(
            KeyedBroadcastStateTransformation.class,
            new KeyedBroadcastStateTransformationTranslator<>());
    tmp.put(CacheTransformation.class, new CacheTransformationTranslator<>());
    translatorMap = Collections.unmodifiableMap(tmp);
}
```





















>PackagedProgram#callMainMethod() 执行的是 StreamWordCount的 main()方法，而方法是无返回值的，那 callMainMethod()又是如何与 Pipeline 是如何关联的呢？ 还有 main()方法执行逻辑似乎并不像普通的java程序一样，会启动独立的 Java Server程序。 跟固化的思维逻辑相比，此时的 main() 执行还是在 Flink Client，还并未到 Flink Job 执行物理线程那一步。      

![showplan11](images/showplan11.png)    
        

**PackagedProgramUtils#getPipelineFromProgram()**               
```java
public static Pipeline getPipelineFromProgram(
        PackagedProgram program,
        Configuration configuration,
        int parallelism,
        boolean suppressOutput)
        throws CompilerException, ProgramInvocationException {
    
    ...... 省略部分代码 

    // temporary hack to support the optimizer plan preview
    OptimizerPlanEnvironment benv =
            new OptimizerPlanEnvironment(
                    configuration, program.getUserCodeClassLoader(), parallelism);
    benv.setAsContext();
    StreamPlanEnvironment senv =
            new StreamPlanEnvironment(
                    configuration, program.getUserCodeClassLoader(), parallelism);
    senv.setAsContext();

    try {
        program.invokeInteractiveModeForExecution(); // 执行 main()
    } catch (Throwable t) {
        if (benv.getPipeline() != null) {
            return benv.getPipeline();
        }

        if (senv.getPipeline() != null) {
            return senv.getPipeline();
        }

        if (t instanceof ProgramInvocationException) {
            throw t;
        }

        throw generateException(
                program, "The program caused an error: ", t, stdOutBuffer, stdErrBuffer);
    } finally {
        benv.unsetAsContext();
        senv.unsetAsContext();
        if (suppressOutput) {
            System.setOut(originalOut);
            System.setErr(originalErr);
        }
        Thread.currentThread().setContextClassLoader(contextClassLoader);
    }

    throw generateException(
            program,
            "The program plan could not be fetched - the program aborted pre-maturely.",
            null,
            stdOutBuffer,
            stdErrBuffer);
}
```   

### PackagedProgram#callMainMethod





## Rest Router   




refer   
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/configuration/maven/          
2.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/configuration/overview/        
3.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/deployment/cli/    
4.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/datastream/operators/overview/        


