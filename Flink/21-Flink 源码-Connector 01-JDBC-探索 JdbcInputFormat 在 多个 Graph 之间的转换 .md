# Flink 源码 - Connector JDBC - 探索 JdbcInputFormat 在 多个 Graph 之间的转换   

>Flink version: 1.15.4，Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8          

我超爱 `Master Worker Architecture`          
![jdbcsourcesplit01](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit01.jpg)          

## 引文   
我们在使用 Flink 分布式计算框架处理数据时，总是涉及到 `Master Worker Architecture`,若 JobManager 配置了 HA，还会涉及到 `Master Slave Architecture`, 大多数时候，我们希望 JobManager 可以起到任务分发的角色，将任务拆分到不同的 TaskManager 上执行，从而达到 `资源隔离场景中的并发`， 并且 TaskManager 很容易实现 `横向扩展`, 如果我说像 `Pekko (Akka)`、`Hazelcast` 这样的框架可以帮助我们快速构建一个 `Cluster` 的应用，那 `Flink` 不仅可以帮助我们快速构建一个 `Cluster` 应用，并且它也简化了我的开发成本，我只需要关注 `Custom Function` 实现自己的业务即可。这样的 `Distributed Architecture` 怎能不爱 !    

下面，我通过 `Flink Connector Jdbc 读取 MySQL 数据示例` 来探索 `Master Worker Architecture`。   

## Flink Connector Jdbc 读取 MySQL 示例    

### 1. MySQL 中存在 `yzhou_test.yzhou_test02` table, 下面是它的建表语句以及它内部的示例数据。    
```sql
create table yzhou_test02
(
    id          int auto_increment
        primary key,
    name        varchar(100) null,
    address     varchar(100) null,
    ext_field01 varchar(100) null
);
```

示例数据：   
![jdbcsourcesplit02](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit02.jpg)   

### 2. 使用 `flink-connector-jdbc` jar, 开发读取 MySQL 数据 Job        
在项目的 `pom.xml` 添加 Flink 相关 jar 以及 `flink-connector-jdbc`，`mysql-connector-j` 依赖。    

我的 `MySQL2Console` 示例中有3个算子，如下所示：    
1.source 算子： 使用 `JdbcInputFormat` 构建。      
2.map 算子：使用 Log 打印数据。  
3.print 算子：使用 STDOUT 打印 数据  

`Job 完整示例代码如下：`       
```java
/**
 * MySQL 2 Console
 */
public class MySQL2Console {
  private static final Logger logger = LoggerFactory.getLogger(MySQL2Console.class);

  public static void main(String[] args) throws Exception {
    Configuration configuration = new Configuration();
    StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment(configuration);
    env.setRuntimeMode(RuntimeExecutionMode.AUTOMATIC);
    env.registerJobListener(new CustomJobListener());
    env.setParallelism(2);

    JdbcNumericBetweenParametersProvider jdbcNumericBetweenParametersProvider = new JdbcNumericBetweenParametersProvider(2, 1, 10);
    DataStreamSource<Row> myDataStream = env.createInput(JdbcInputFormat.buildJdbcInputFormat()
                    .setDrivername("com.mysql.cj.jdbc.Driver")
                    .setDBUrl("jdbc:mysql://192.168.0.201:3306/yzhou_test?serverTimezone=GMT%2B8&useSSL=false")
                    .setUsername("root")
                    .setPassword("123456")
                    .setParametersProvider(jdbcNumericBetweenParametersProvider)
                    .setQuery("select * from yzhou_test02 where id between ? and ?")
                    .setRowTypeInfo(new RowTypeInfo(
                            BasicTypeInfo.INT_TYPE_INFO,
                            BasicTypeInfo.STRING_TYPE_INFO,
                            BasicTypeInfo.STRING_TYPE_INFO,
                            BasicTypeInfo.STRING_TYPE_INFO))
                    .finish())
            .setParallelism(2);
    myDataStream.map(data -> {
              logger.info("yzhou data : " + data.toString());
              return data;
            })
            .print().setParallelism(2);
    System.out.println("yzhou job start ！！！！！" + new DateTime().toString("yyyy-MM-dd HH:mm:ss"));
    JobExecutionResult execute = env.execute("mysql 2 console");
    System.out.println("yzhou job end ！！！！！" + new DateTime().toString("yyyy-MM-dd HH:mm:ss"));
  }
}
```  

>接下来，我们探索 `并发` 读取MySQL的实现。      

## JobManager 设置任务集      
为了可以并发读取 MySQL 数据，我使用 `JdbcNumericBetweenParametersProvider` 类定义好数据的拉取条数 fetchSize 以及 左边界 min id,右边界 max id, 有了这 `3` 个参数，JobManager 调用 `PackagedProgram#callMainMethod()`(也就是示例代码中 `MySQL2Console`的 main() 方法) 会根据每次拉取条数，min，max 计算好 `select * from yzhou_test02 where id between ? and ?` SQL 的参数集合。就如下面2张图所示：            
![jdbcsourcesplit03](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit03.jpg)        

`JdbcNumericBetweenParametersProvider#getParameterValues()`       
![jdbcsourcesplit04](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit04.jpg)      

为了深入理解 `JdbcInputFormat`，我们看下它在`Graph` 之间是如何传递的 JdbcInputFormat？     
![jdbcsourcesplit06](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit06.jpg)    

## JdbcInputFormat 在 Graph 之间的流转      
>注意，这里不会详细介绍 构造 StreamGraph -> JobGraph -> ExectionGraph,大家可访问我的文章：  
1.`Flink 源码 - Standalone - 探索 Flink Stream Job - 构建 StreamGraph 流程` https://mp.weixin.qq.com/s/DCpajYs9Ph9Hon7_P9gEHA   
2.`Flink 源码 - Standalone - 探索 Flink Stream Job - 构建 JobGraph 流程` https://mp.weixin.qq.com/s/Ni2Y5JeAKcbtJlE1urA56A  

### 看 `StreamGraph` 是如何存储 JdbcInputFormat？ 

`JdbcInputFormat` 继承了 `RichInputForm`类，并且也实现了 `ResultTypeQueryable`接口。而 ResultTypeQueryable 接口提供了 JdbcInputFormat(InputFormat) 查询结果类型。  
![jdbcsourcesplit07](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit07.jpg)  

当 `MySQL2Console#main()` 调用 `setParametersProvider()` 方法会计算好 SQL Between 左右边界值。 在执行 `StreamExecutionEnvironment#getStreamGraph()` 构建 StreamGraph 时，JdbcInputFormat 会赋值给 StreamNode的 intputFormat字段中。 如下图所示:   
![jdbcsourcesplit08](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit08.jpg)  

`JdbcInputFormat 字段信息`  
![jdbcsourcesplit09](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit09.jpg)         

`StreamNode inputFormat`   
![jdbcsourcesplit10](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit10.jpg)  

### 看 `JobGraph` 是如何存储 JdbcInputFormat？    

`StreamingJobGraphGenerator#createChain()`会将 StreamGraph 中的 StreamNodes，按照`规则`，合并成一个或者多个 Chain 链，并且会将Chain 中的首节点转换成 `JobVertex`，在这个过程中，会先判断当前节点是否包含 inputFormat 也就是 StreamNode，如果存在 inputFormat，则将其放入 `chainedInputOutputFormats` 容器中。
`StreamingJobGraphGenerator#createChain()` 
```java
if (currentNode.getInputFormat() != null) {
    getOrCreateFormatContainer(startNodeId)
            .addInputFormat(currentOperatorId, currentNode.getInputFormat());
}
```

`StreamingJobGraphGenerator#createJobVertex()`方法会将 `chainedInputOutputFormats` 容器中的 inputFormat 通过 `InstantiationUtil.writeObjectToConfig()` 方法序列化成二进制字节数组后，存放在 jobVertex的 configuration 字段中。     
```java
if (chainedInputOutputFormats.containsKey(streamNodeId)) {
    jobVertex =
            new InputOutputFormatVertex(
                    chainedNames.get(streamNodeId), jobVertexId, operatorIDPairs);

    chainedInputOutputFormats
            .get(streamNodeId)
            .write(new TaskConfig(jobVertex.getConfiguration()));
} else {
    jobVertex = new JobVertex(chainedNames.get(streamNodeId), jobVertexId, operatorIDPairs);
}
```  

通过 `InputOutputFormatContainer#write()` 方法将 formats 以 "udf" 作为key，写入到 configuration。   
```java
public void write(TaskConfig config) {
    config.setStubWrapper(new UserCodeObjectWrapper<>(formats));
    config.setStubParameters(parameters);
}
```  
![jdbcsourcesplit11](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit11.jpg)       

下面是 config 赋值 inputFormat的方法：  
`InputOutputFormatContainer#write()`    
![jdbcsourcesplit12](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit12.jpg)    

此时，我们来 JobVertex 的 configuration 变量值信息，它存储的数据是二进制数据，为了可以看到 udf key 的 value 值，我们需要借助 `InstantiationUtil.readObjectFromConfig()`方法 在 IDEA 中计算它的值。 如下图所示：    
![jdbcsourcesplit13](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit13.jpg) 

`InstantiationUtil.readObjectFromConfig(jobVertex.getConfiguration(),"udf", getClass().classLoader)`    
![jdbcsourcesplit14](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit14.jpg)  

从上述介绍可知，inputFormat 存储在 JobVertex 的configuration 字段中，所以当我们从 jobGraph 对象中也可以看到 inputform 信息。将断点打到 `StreamingJobGraphGenerator#createJobGraph()` 的返回的 jobGraph 对象上, 如下图所示，我们也可以使用 `InstantiationUtil.readObjectFromConfig()` 方法将 inputFormat 信息在 IDEA 中计算出来。    
![jdbcsourcesplit15](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit15.jpg)

`InstantiationUtil.readObjectFromConfig(jobGraph.taskVertices.values().stream().findFirst().get().getConfiguration(),"udf", getClass().classLoader)`    

![jdbcsourcesplit16](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit16.jpg)

`JogGraph inputFormat`   
![jdbcsourcesplit17](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit17.jpg)   
 
### 看 `ExecutionGraph` 是如何存储 JdbcInputFormat？     
>前置知识：JobGraph 在转换成 ExecutionGraph 时，并不会对整个 DAG 结构有所改变，ExecutionGraph 增加了 并行度的概念，所以它的节点上有了"组"的概念。在 JobGraph 的 jobVertex 在 ExecutionGraph 会对应生成 `ExecutionJobVertex` 和 `ExecutionVertex`。   

ExecutionJobVertex 定义数据的处理逻辑，而 ExecutionVertex 是处理逻辑在并行处理时具体实例。每个 ExecutionJobVertex 可以根据其设定的并行度拥有多个ExecutionVertex，每个 ExecutionVertex 都是独立执行的任务单元。  

下图可查看两者的关系：  
`DefaultExecutionGraphFactory#createAndRestoreExecutionGraph()`
![jdbcsourcesplit18](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit18.jpg) 

通过上面的 newExecutionGraph 变量信息可知，它的 `ExecutionJobVertex` 包含 JobVertex 信息。  
![jdbcsourcesplit19](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit19.jpg)  

`ExecutionGraph JdbcInputFormat`  
![jdbcsourcesplit20](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit20.jpg)  

该篇文章就暂时介绍到这里，了解 JdbcInputFormat 在各个 Graph 之间的流程逻辑，后面再来探索 TaskManager 是如何获取 JobManager 的任务集的？       
![jdbcsourcesplit21](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit21.jpg)    

