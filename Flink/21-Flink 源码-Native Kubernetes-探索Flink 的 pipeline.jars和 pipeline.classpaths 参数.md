# Flink 源码 - Native Kubernetes - 探索 Flink 的 pipeline.jars 和 pipeline.classpaths 参数    

>Flink version: 1.15.4, JDK version: 11, Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8, Seatunnel branch: 2.3.11-release    

## 背景    
博主所在团队的小伙伴们开发数据需求时，经常会使用 `Dolphinscheduler` + (`Datax` 或者 `Flink Job`) 配置成`批任务`解决数据流转问题，团队为了解决 Datax 资源不隔离、Flink Job 开发成本高问题，统一开发基于 Flink 的数据传输工具，目前该工具已投产使用了。 为了可以快速适配更多的 Source，Sink，我们希望再增加 Source 或者 Sink 时，传输工具的 `core` 代码不变动，仅开发某个存储介质的`connector`模块即可。 所以我们决定使用动态加载 + SPI 机制实现 Plugin Connector 和 Third Party Library 加载到 Flink 中。  

在改造过程中博主遇到一些问题，例如 TaskManager 如何处理 Plugin Connector 和 Third Party Library 的动态加载？ JobManager 比较好处理，在基于 Native Kubernetes Application Model 的 JobManager 会调用 user jar 中的 main() 方法来触发动态加载。   

为了解决上述问题，博主调试 Seatunnel Run On Flink 是如何处理动态加载的，想学以致用 !!!    

>所以该篇文章的主题是由 Seatunnel Run On Flink 引入的。     

## Seatunnel Run On Flink MySQL 2 MySQL 示例演示     

`部署结构` 
![debugpipelineparam01](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam01.jpg)  

### 源码编译 
请参考 `https://seatunnel.apache.org/docs/2.3.11/contribution/setup` 文档，将项目克隆下来再导入 IDEA 中，若使用 `mvn clean package -pl seatunnel-dist -am -Dmaven.test.skip=true` 打包源码过程中出现代码格式校验不通过，可先使用 `mvn spotless:apply` 命令对其项目进行格式化。  

打包成功后，使用 `seatunnel-dist/target/apache-seatunnel-2.3.12-SNAPSHOT-bin.tar.gz` 二进制进行部署。   

### 环境部署   
在 VM01 节点使用 `tar -zxf apache-seatunnel-2.3.12-SNAPSHOT-bin.tar.gz` 命令解压安装包，因为我仅测试 MySQL 2 MySQL 示例，所以在 plugins/ 目录创建 `jdbc/lib/` 文件夹，再将 `mysql-connector-j-8.0.33.jar` 放入 jdbc/lib 文件夹下，完整结构如下：  
![debugpipelineparam02](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam02.jpg)     

在 `apache-seatunnel-2.3.12-SNAPSHOT` 同级目录创建 `jobs` 文件夹，在 jobs/ 目录下创建 `mysql2mysql.config`。 内容如下： 
```bash
env {
  parallelism = 1
  job.mode = "BATCH"
}

source {
    jdbc {
        url = "jdbc:mysql://192.168.0.201:3306/yzhou_test"
        driver = "com.mysql.cj.jdbc.Driver"
        user = "root"
        password = "123456"
        database = yzhou_test
        table = yzhou_test02
        table_path = "yzhou_test.yzhou_test02"
    }
}

sink {
  jdbc {
        url = "jdbc:mysql://192.168.0.201:3306/yzhou_test"
        driver = "com.mysql.cj.jdbc.Driver"
        user = "root"
        password = "123456"
        generate_sink_sql = true
        database = yzhou_test
        table = yzhou_test03
    }
}
```

在 VM01 节点下载 Flink 1.15.4 版本的安装包，解压后，修改 conf/flink-conf.yaml 中的 `rest.address` 参数，将其 value 设置为 VM02 节点的IP，示例如下： 
```bash
# The address to which the REST client will connect to
#
rest.address: 192.168.0.202
```

最后通过 `vim /etc/profile` 设置 `FLINK_HOME`参数，例如： export FLINK_HOME=/root/flink/flink1154/flink-1.15.4， 别忘记 source /etc/profile。  

VM02 节点下载 Flink 1.15.4 版本的安装包，解压后,执行 `bin/start-cluster.sh` 启动 Flink Standalone Cluster。 注意：可适当调整 `taskmanager.numberOfTaskSlots` 参数，例如设置为 40。  

此时，seatunnel 的测试环境已搭建好了。    

>注意：我的开发环境是 `JetBrains Gateway SSH` + `RockyLinux 9`。

### Seatunnel 启动 Flink Job  
在 VM01 节点执行以下命令启动 Flink Job， 完整示例如下图：       
```java
/root/seatunnel/apache-seatunnel-2.3.12-SNAPSHOT/bin/start-seatunnel-flink-15-connector-v2.sh --config /root/seatunnel/jobs/mysql2mysql.config
```    

![debugpipelineparam03](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam03.jpg)   

我们可以看到 Seatunnel 启动命令执行会打印 `flink run` 命令。 到这里，我想你应该可以意识到，打印的 Flink Run 命令，是可以单独执行的, 所以单独执行以下命令与执行`start-seatunnel-flink-15-connector-v2.sh` 脚本是等效的。这也就解释了为什么 VM01 节点需要安装 Flink 安装包。      
```bash
${FLINK_HOME}/bin/flink run -c org.apache.seatunnel.core.starter.flink.SeaTunnelFlink /root/seatunnel/apache-seatunnel-2.3.12-SNAPSHOT/starter/seatunnel-flink-15-starter.jar --config /root/seatunnel/jobs/mysql2mysql.config --name SeaTunnel --deploy-mode run
```  

有了 seatunnel 示例的演示，那我们自己编写的 Flink Job 实现 MySQL 2 MySQL，我们会把 mysql-connector-j-8.0.33.jar 放在 pom.xml 添加 mysql-connector-j-8.0.33.jar 或者 flink lib/ 目录下。  

而 seatunnel 并不是。    

### 思考 mysql-connector-j-8.0.33.jar 哪去了？  
VM02 节点启动的 Flink Standalone Cluster lib/下并没有 mysql-connector-j-8.0.33.jar，内部 jar 如下图所示：  
![debugpipelineparam04](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam04.jpg)    

我们通过 Java Decompiler 工具查看 `seatunnel-flink-15-starter.jar` 也没有 MySQL 代码，如下图所示：  
![debugpipelineparam05](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam05.jpg)  

但是我在 seatunnel 的 plugin/jdbc/lib/ 目录下 mysql-connector-j-8.0.33.jar,  那会是这个路径下的 jar么？      

## Seatunnel Run On Flink 查找 JAR   
根据 Seatunnel 脚本打印的 Flink Run 脚本，在 IDEA 中配置 Seatunnel Flink Job 启动类 `SeaTunnelFlink`，如下所示：    
```bash
${FLINK_HOME}/bin/flink run -c org.apache.seatunnel.core.starter.flink.SeaTunnelFlink /root/seatunnel/apache-seatunnel-2.3.12-SNAPSHOT/starter/seatunnel-flink-15-starter.jar --config /root/seatunnel/jobs/mysql2mysql.config --name SeaTunnel --deploy-mode run     
```

![debugpipelineparam06](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam06.jpg)      

>注意：-DSEATUNNEL_HOME 参数配置的是 Seatunnel 二进制包部署的路径。      

>开始动手定位 Code 吧  :)  

![debugpipelineparam07](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam07.jpg)       

Seatunnel 动态加载类后，会在 `AbstractFlinkRuntimeEnvironment#createStreamEnvironment()` 方法中将 jarPaths 当作参数配置给 Flink Configuration `pipeline.jars`,`pipeline.classpaths`， 注意此时的 jarPaths 除了包含 mysql-connector-j-8.0.33.jar以外，还会包含 `connector-jdbc-2.3.12-SNAPSHOT.jar`。       

**AbstractFlinkRuntimeEnvironment#createStreamEnvironment()**
```java
protected void createStreamEnvironment() {
        Configuration configuration = new Configuration();
        EnvironmentUtil.initConfiguration(config, configuration); // 设置 pipeline.jars, pipeline.classpaths 参数
        environment = StreamExecutionEnvironment.getExecutionEnvironment(configuration);
        setTimeCharacteristic();
        setCheckpoint();

        EnvironmentUtil.setRestartStrategy(config, environment.getConfig());

        if (EnvironmentUtil.hasPathAndWaring(config, ConfigKeyName.BUFFER_TIMEOUT_MILLIS)) {
            long timeout = config.getLong(ConfigKeyName.BUFFER_TIMEOUT_MILLIS);
            environment.setBufferTimeout(timeout);
        }

        if (config.hasPath(EnvCommonOptions.PARALLELISM.key())) {
            int parallelism = config.getInt(EnvCommonOptions.PARALLELISM.key());
            environment.setParallelism(parallelism);
        } else if (config.hasPath(ConfigKeyName.PARALLELISM)) {
            log.warn(
                    "the parameter 'execution.parallelism' will be deprecated, please use common parameter 'parallelism' to set it");
            int parallelism = config.getInt(ConfigKeyName.PARALLELISM);
            environment.setParallelism(parallelism);
        }

        if (EnvironmentUtil.hasPathAndWaring(config, ConfigKeyName.MAX_PARALLELISM)) {
            int max = config.getInt(ConfigKeyName.MAX_PARALLELISM);
            environment.setMaxParallelism(max);
        }

        if (this.jobMode.equals(JobMode.BATCH)) {
            environment.setRuntimeMode(RuntimeExecutionMode.BATCH);
        }
    }
```

下面是 Seatunnel 的 Flink Configuration 变量信息： 
![debugpipelineparam17](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam17.jpg) 


到这里，大伙可以大胆猜测是它们可以让Flink 动态加载 mysql-connector-j-8.0.33.jar。上面铺垫这么多，终于引出了 Flink `pipeline.jars`,`pipeline.classpaths` 参数。     

为了验证这个猜想，我们可以自己写个 Flink Job MySQL 2 Console 像 Seatunnel 一样配置 pipeline.jars, pipeline.classpaths。    

### MySQL 2 Console 测试示例   
首先确保项目中不存在 mysql-connector-java 依赖 jar，在 确保 Flink 镜像挂载的 lib pvc 不存在 mysql-connector-java 依赖 jar。    

测试代码 `MySQL2Console.java` 代码如下。  
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

我们可以给添加上 pipeline.jars,pipeline.classpaths 参数，为了不干扰 lib，我提前将 mysql-connector-java jar 放到Flink 镜像挂载 pvc /opt/flink/log 目录下。`这里需要特别注意` 需要将 job jar 和 third party library 都设置到这两个参数去。   

`MySQL2Console02.java`
```java
public class MySQL2Console02 {
  private static final Logger logger = LoggerFactory.getLogger(MySQL2Console02.class);

  public static void main(String[] args) throws Exception {
    Configuration configuration = new Configuration();
    logger.info("yzhou jars,classpaths");
    configuration.setString(PipelineOptions.JARS.key(), "file:/opt/flink/log/mysql-connector-j-8.0.33.jar;file:/opt/flink/log/flink-jar-1-15-4-1.0-SNAPSHOT.jar");
    configuration.setString(PipelineOptions.CLASSPATHS.key(), "file:/opt/flink/log/mysql-connector-j-8.0.33.jar;file:/opt/flink/log/flink-jar-1-15-4-1.0-SNAPSHOT.jar");
    StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment(configuration);

    // 省略部分代码 

```

#### `MySQL2Console`的运行结果
`MySQL2Console`的运行结果是失败的，我们通过 Flink History Server 查看它的异常信息，是符合我们预期的,提示缺少 `Caused by: java.lang.ClassNotFoundException: com.mysql.cj.jdbc.Driver`   
![debugpipelineparam08](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam08.jpg)    

#### `MySQL2Console02`的运行结果 
`MySQL2Console02`的运行结果是成功执行，是符合预期的，如下图所示：   
![debugpipelineparam09](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam09.jpg)  

### 小结 
博主自测的 Flink Job MySQL 2 MySQL 与 Seatunnel Flink Job 测试结果是一样的，所以可以验证 `pipeline.jars`,`pipeline.classpaths` 参数的效果。     

下面开始探索 `pipeline.jars`,`pipeline.classpaths` 的处理细节。   

## `pipeline.jars`,`pipeline.classpaths` 处理逻辑     
pipeline.jars,pipeline.classpaths  这两个参数是通过 Flink Configuration 对象设置的，配置代码如下：  
```java
   Configuration configuration = new Configuration();
    logger.info("yzhou jars,classpaths");
    configuration.setString(PipelineOptions.JARS.key(), "file:/opt/flink/log/mysql-connector-j-8.0.33.jar;file:/opt/flink/log/flink-jar-1-15-4-1.0-SNAPSHOT.jar");
    configuration.setString(PipelineOptions.CLASSPATHS.key(), "file:/opt/flink/log/mysql-connector-j-8.0.33.jar;file:/opt/flink/log/flink-jar-1-15-4-1.0-SNAPSHOT.jar");
    StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment(configuration);
```

JobManager 调用 `PipelineExecutorUtils#getJobGraph()` 将 StreamGraph 转 JobGraph 后，会将 jars 和 classpaths 赋值给 JobGraph 对象上，处理代码如下：  
![debugpipelineparam10](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam10.jpg)  

后续 `EmbeddedExecutor#submitJob()` 会将 JobGraph中的 Jars 通过 BlobClient 发送给 JobManager 中的 BlobServer ，其内部上传的细节逻辑在 `ClientUtils#uploadJobGraphFiles()` 方法，每上传一个 jar，BlobServer 会返回一个 BlobKey，并且会根据 BlobKey 重命名 jar 文件名称，存储到 JobManager `blobStorage` 目录下。 处理代码如下：  

**EmbeddedExecutor#submitJob()**
```java
    private static CompletableFuture<JobID> submitJob(
            final Configuration configuration,
            final DispatcherGateway dispatcherGateway,
            final JobGraph jobGraph,
            final Time rpcTimeout) {
        checkNotNull(jobGraph);

        LOG.info("Submitting Job with JobId={}.", jobGraph.getJobID());

        return dispatcherGateway
                .getBlobServerPort(rpcTimeout)
                .thenApply(
                        blobServerPort ->
                                new InetSocketAddress(
                                        dispatcherGateway.getHostname(), blobServerPort))
                .thenCompose(
                        blobServerAddress -> {
                            try {
                                ClientUtils.extractAndUploadJobGraphFiles(   // 发送 jar 
                                        jobGraph,
                                        () -> new BlobClient(blobServerAddress, configuration));
                            } catch (FlinkException e) {
                                throw new CompletionException(e);
                            }

                            return dispatcherGateway.submitJob(jobGraph, rpcTimeout);
                        })
                .thenApply(ack -> jobGraph.getJobID());
    }
```

**ClientUtils#uploadUserJars()**
```java
private static Collection<PermanentBlobKey> uploadUserJars(
        JobID jobId, Collection<Path> userJars, BlobClient blobClient) throws IOException {
    Collection<PermanentBlobKey> blobKeys = new ArrayList<>(userJars.size());
    for (Path jar : userJars) {
        final PermanentBlobKey blobKey = blobClient.uploadFile(jobId, jar); // 返回 blobKey 
        blobKeys.add(blobKey);
    }
    return blobKeys;
}
```

![debugpipelineparam11](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam11.jpg)  

BlobServer 会根据 `BlobUtils#getStorageLocation()` 方法生成 jar 的存储地址和文件名称。如下图所示：  
![debugpipelineparam12](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam12.jpg)   

等待上传完后，我们看下 JobManager blobStorage 目录下的文件， 我们再对比 /opt/flink/log 目录下的 `mysql-connector-j-8.0.33.jar`,  `flink-jar-1-15-4-1.0-SNAPSHOT.jar`, 光从文件大小来看，是可以匹配上的。   

**JobManager blobStorage 目录** 
```bash
root@flink-application-test-7c4c655f69-zkx85:/tmp/jm_c56d01e31f5efc7f86bfd11365430155/blobStorage/job_5e52459e92f12bf61401f046a2d70ebc# ls -l
total 93508
-rw-r--r-- 1 flink flink 93268062 Jul 12 15:14 blob_p-6fb377e365a962666d15892907ef2b397a2fb02e-c416f049f7c82424dc8e9d98d6333ed6
-rw-r--r-- 1 flink flink  2481560 Jul 12 15:13 blob_p-9e64d997873abc4318620264703d3fdb6b02dd5a-e183a99908817a181d183230137a3395
root@flink-application-test-7c4c655f69-zkx85:/tmp/jm_c56d01e31f5efc7f86bfd11365430155/blobStorage/job_5e52459e92f12bf61401f046a2d70ebc#
```

**JobManager /opt/flink/log  pvc目录** 
```bash
root@flink-application-test-7c4c655f69-zkx85:/opt/flink/log# ls -l |grep jar
-rw-r--r-- 1 root  root  93268062 Jul 12 09:18 flink-jar-1-15-4-1.0-SNAPSHOT.jar
-rw-r--r-- 1 root  root   2481560 Jul 12 02:37 mysql-connector-j-8.0.33.jar
```

## 总结
在 Native Kubernetes Application Model 下，JobManager 的 jars 被上传一遍到 JobManager blobStorage, 所以 TaskManager 是可以通过 BlobServer 拉取到上传的 jar 文件的。   
![debugpipelineparam13](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam13.jpg)  

有了对这两个参数的了解，我们再回到 Seatunnel Run On Flink 的示例，执行 flink run 命令，观察 VM02 节点的 blobStorage 目录下的文件，如下图所示：  
![debugpipelineparam14](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam14.jpg)  

下面是 TaskManager blobStorage 目录文件：  
![debugpipelineparam15](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam15.jpg)   

到这里，pipeline.jars,pipeline.classpaths 的设置，可以让自定义的 jars 通过 BlobClient 传输到 JobManager，TaskManager 中，这也让我们方便实现动态加载 自定义的 jars。    
![debugpipelineparam16](http://img.xinzhuxiansheng.com/blogimgs/flink/debugpipelineparam16.jpg)  

后续，博主也会探索 BlobServer 在 Flink 中的作用。   

refer   
1.https://seatunnel.apache.org/docs/2.3.11/contribution/setup  
