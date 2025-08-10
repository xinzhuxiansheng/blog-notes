# Flink 源码 - Connector JDBC - 了解 JdbcInputFormat 到 自定义实现 InputFormat  

>Flink version: 1.15.4，Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8         

## 引文  
由于距离上一篇发布的 `探索 JdbcInputFormat 在 多个 Graph 之间的转换` 公众号文章时间较长，所以需要一些冗余的文字来说明探索的方向。 我是想以 JdbcInputFormat 为示例探索 Connector 实现机制。           

下面是 MySQL 2 Console 示例代码以及 Shell 提交 Job 命令内容。          

### MySQL2Console 代码示例    
Flink Job 示例代码使用 JdbcInputFormat 读取 MySQL 数据打印到控制台。 它并行度设置为 2，numberOfTaskSlots 设置为 1，其目的是模拟在多个 TaskManager 场景。 同时也包括并行读取的场景。`JdbcNumericBetweenParametersProvider` 负责帮助我们完成 Source Ids集合的拆分。             

![jdbcsourcesplit22](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit22.jpg)    

`MySQL2Console.java 示例代码`   
```java
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

### 提交 Flink Job 脚本     
```bash
[root@master01 flink15]# cat submit-application-job-mysql.sh
/root/yzhou/flink/flink-1.15.4/bin/flink  run-application \
    --target kubernetes-application \
    -Dkubernetes.namespace=flink \
    -Dkubernetes.service-account=flink-service-account \
    -Dkubernetes.cluster-id=flink-application-test \
    -Dkubernetes.pod-template-file.jobmanager=./jobmanager-pod-template.yaml \
    -Dkubernetes.pod-template-file.taskmanager=./taskmanager-pod-template.yaml \
    -Dclassloader.resolve-order=parent-first \
    -Dkubernetes.rest-service.exposed.type=NodePort \
    -Dtaskmanager.numberOfTaskSlots=1 \
    -c com.yzhou.flink.example.MySQL2Console \
    local:///opt/flink/artifacts/app.jar   
``` 

### Flink Job 运行时的状态记录   
`Kubernetes Pod 状态`  
![jdbcsourcesplit23](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit23.jpg)   

`Flink Job DAG 图`   
![jdbcsourcesplit24](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit24.jpg)     

在后面探索的手段是 remote debug 和 print log。 若这部分还不熟悉的话可参考 `https://mp.weixin.qq.com/s/G6J_W1Yuetg-rGEYa5f7ZQ`,`我不知道大家是否跟我有同样的疑问`：某些类或者某些方法不太清楚是在 JobManager 还是在 TaskManager 运行 ? 所以 remote debug 可以让我很方便定位具体位置，这里大家可以思考一个问题 `JdbcInputFormat` 支持并行读取，id 为拆分成多个区间段，那 TaskManager 是如何被分配到具体哪个区间呢？ 是由 JobManager 下发给 TaskManager 还是 TaskManager 主动去 JobManager 申请的呢？     
![jdbcsourcesplit26](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit26.jpg)       

若一些逻辑之间需要判断执行先后，或者部分逻辑需要长期跟进的话，可添加 log 打印再重新打包，最后通过 pvc 方式替换 Flink image 中的 lib目录。 你也可以使用 Loki + Promtail + Grafana 搭建 Kubernetes Pod STDOUT Log 收集。               

## 思考一 
在 MySQL2Console 代码示例中，我们可知晓 `JdbcInputFormat.buildJdbcInputFormat()` 是调用 `flink-connector-jdbc.jar`类库构造的。  `env.createInput()` 会使用 JdbcInputFormat 对象构造出一个通用模板类型 `InputFormatSourceFunction`，两者是关联关系。       

![jdbcsourcesplit25](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit25.jpg)   

>为什么称呼它为通过模板类型是因为 `InputFormatSourceFunction` 类是 `flink-streaming-java` 模块的类（公共模块）。   

`InputFormatSourceFunction#open()` 方法执行了`非常重要`的逻辑，首先获取负责 input split 的 RPC调用类 `RpcInputSplitProvider`，`getInputSplits()` 创建了一个请求 JobManager 的迭代器，该迭代器两个非常重要的方法，hasNext(), next() ,判断是否有值，获取下一个返回值都依托于 `InputSplitProvider` RPC 调用 getNextInputSplit() 请求 JobManager。     
 
**InputFormatSourceFunction#open()**      
```java
public void open(Configuration parameters) throws Exception {
        StreamingRuntimeContext context = (StreamingRuntimeContext) getRuntimeContext();

        if (format instanceof RichInputFormat) {
                ((RichInputFormat) format).setRuntimeContext(context);
        }
        format.configure(parameters);

        provider = context.getInputSplitProvider();
        serializer = typeInfo.createSerializer(getRuntimeContext().getExecutionConfig());
        splitIterator = getInputSplits();
        isRunning = splitIterator.hasNext();
}
```     

`InputFormatSourceFunction` 是 `JdbcInputFormat` 包装类，并且也是 Source 的入口类，当 TaskManager 启动后，它的 `InputFormatSourceFunction#run()` 方法管控着 inputformat 的核心方法，例如 `splitIterator.next()` 方法获取任务分片，`format.open()` 方法根据获取到的任务分片执行具体逻辑，`format.nextRecord()` 方法获取数据，`format.close()` 方法处理需要关闭的逻辑。     

当任务分片已被认领完后，则 while(isRunning) 循环。  
**InputFormatSourceFunction#run()**    
```java
@Override
public void run(SourceContext<OUT> ctx) throws Exception {
    try {

        Counter completedSplitsCounter =
                getRuntimeContext().getMetricGroup().counter("numSplitsProcessed");
        if (isRunning && format instanceof RichInputFormat) {
            ((RichInputFormat) format).openInputFormat();
        }

        OUT nextElement = serializer.createInstance();
        while (isRunning) {
            format.open(splitIterator.next());

            // for each element we also check if cancel
            // was called by checking the isRunning flag

            while (isRunning && !format.reachedEnd()) {
                nextElement = format.nextRecord(nextElement);
                if (nextElement != null) {
                    ctx.collect(nextElement);
                } else {
                    break;
                }
            }
            format.close();
            completedSplitsCounter.inc();

            if (isRunning) {
                isRunning = splitIterator.hasNext();
            }
        }
    } finally {
        format.close();
        if (format instanceof RichInputFormat) {
            ((RichInputFormat) format).closeInputFormat();
        }
        isRunning = false;
    }
}
```

![jdbcsourcesplit27](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit27.jpg) 

`InputFormatSourceFunction#run()` 方法根据 `Iterator<InputSplit> splitIterator` 迭代器获取到 `InputSplit` 任务下标号来调用 `format.open(splitIterator.next())` 构造查询 SQL 参数，`while (isRunning && !format.reachedEnd())` 循环会将查询 MySQL 数据集 ResultSet 遍历后调用 ctx.collect() 发送出去 。  

**InputFormatSourceFunction#run()**   
```java
@Override
    public void run(SourceContext<OUT> ctx) throws Exception {
        try {

            Counter completedSplitsCounter =
                    getRuntimeContext().getMetricGroup().counter("numSplitsProcessed");
            if (isRunning && format instanceof RichInputFormat) {
                ((RichInputFormat) format).openInputFormat();
            }

            OUT nextElement = serializer.createInstance();
            while (isRunning) {
                format.open(splitIterator.next());

                // for each element we also check if cancel
                // was called by checking the isRunning flag

                while (isRunning && !format.reachedEnd()) {
                    nextElement = format.nextRecord(nextElement);
                    if (nextElement != null) {
                        ctx.collect(nextElement);
                    } else {
                        break;
                    }
                }
                format.close();
                completedSplitsCounter.inc();

                if (isRunning) {
                    isRunning = splitIterator.hasNext();
                }
            }
        } finally {
            format.close();
            if (format instanceof RichInputFormat) {
                ((RichInputFormat) format).closeInputFormat();
            }
            isRunning = false;
        }
    }
```

**JdbcInputFormat#open()**
```java
@Override
public void open(InputSplit inputSplit) throws IOException {
try {
        if (inputSplit != null && parameterValues != null) {
        for (int i = 0; i < parameterValues[inputSplit.getSplitNumber()].length; i++) {
                Object param = parameterValues[inputSplit.getSplitNumber()][i];
                if (param instanceof String) {
                statement.setString(i + 1, (String) param);
                } else if (param instanceof Long) {
                statement.setLong(i + 1, (Long) param);
                } else if (param instanceof Integer) {
                statement.setInt(i + 1, (Integer) param);
                } else if (param instanceof Double) {
                statement.setDouble(i + 1, (Double) param);
                } else if (param instanceof Boolean) {
                statement.setBoolean(i + 1, (Boolean) param);
                } else if (param instanceof Float) {
                statement.setFloat(i + 1, (Float) param);
                } else if (param instanceof BigDecimal) {
                statement.setBigDecimal(i + 1, (BigDecimal) param);
                } else if (param instanceof Byte) {
                statement.setByte(i + 1, (Byte) param);
                } else if (param instanceof Short) {
                statement.setShort(i + 1, (Short) param);
                } else if (param instanceof Date) {
                statement.setDate(i + 1, (Date) param);
                } else if (param instanceof Time) {
                statement.setTime(i + 1, (Time) param);
                } else if (param instanceof Timestamp) {
                statement.setTimestamp(i + 1, (Timestamp) param);
                } else if (param instanceof Array) {
                statement.setArray(i + 1, (Array) param);
                } else {
                // extends with other types if needed
                throw new IllegalArgumentException(
                        "open() failed. Parameter "
                                + i
                                + " of type "
                                + param.getClass()
                                + " is not handled (yet).");
                }
        }
        if (LOG.isDebugEnabled()) {
                LOG.debug(
                        String.format(
                                "Executing '%s' with parameters %s",
                                queryTemplate,
                                Arrays.deepToString(
                                        parameterValues[inputSplit.getSplitNumber()])));
        }
        }
        LOG.info("yzhou statement.executeQuery");
        resultSet = statement.executeQuery();
        hasNext = resultSet.next();
} catch (SQLException se) {
        throw new IllegalArgumentException("open() failed." + se.getMessage(), se);
}
}
```

我们需要`小结`一下：     
1.InputFormatSourceFunction 是 TaskManager 读取 Source 的模板类。       

2.InputFormatSourceFunction 会调用 InputFormat 以下方法：  
![jdbcsourcesplit29](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit29.jpg)      

如果我们要实现自己的 InputFormat，就需要实现以上方法。   

3.InputFormatSourceFunction 中的 迭代器对象 `Iterator<InputSplit> splitIterator` ，它返回的是拆分后 Source 集合ids 下标号，TaskManager 是主动向 JobManager 申请任务，而不是 JobManager 主动下发的。 每个 JdbcInputFormat 都保存了一份完整的 拆分 Ids 集合。          
![jdbcsourcesplit28](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit28.jpg)      

为了深入了解内部机制，接下来，我们干一件 从0到1的事。像 JdbcInputFormat 一样，开发一个分布式爬虫。    

## 从0到1 开发分布式爬虫   
我们先通过 `http://192.168.0.2:8082/get/ids` 接口获取需要抓取的 id集合，当 TaskManager 启动后，就像 JdbcInputFormat 一样请求 JobManager 中的 JobMaster 认领 id，有了详情页的 Id，根据 `http://192.168.0.2:8082/get/id/${id}` 拼接详情页的 URL，当一个 id 抓取完后，再次请求 JobMaster 是否还有剩余的 id。以此类推，直到 id集合被认领完成。   
![jdbcsourcesplit30](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit30.jpg)       

### 创建项目 
项目是基于 Flink 1.15.4 版本源码，`git clone -b release-1.15.4 git@github.com:apache/flink.git`, 在 `flink-connectors` 模块下，复制 `flink-connector-jdbc` 子模块并且修改名为 `flink-connector-crawler` 子模块。  如下图所示：  
![jdbcsourcesplit31](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit31.jpg)   

在 `pom.xml` 文件中添加依赖和打包插件，添加 maven-assembly-plugin 插件目的是为了打 fat jar 这样可以省去手动找包的过程，如下：  
```xml
<dependency>
    <groupId>com.fasterxml.jackson.core</groupId>
    <artifactId>jackson-databind</artifactId>
    <version>2.13.0</version> <!-- 用于 JSON 序列化 -->
</dependency>
<dependency>
    <groupId>com.squareup.okhttp3</groupId>
    <artifactId>okhttp</artifactId>
    <version>4.9.3</version>
</dependency>

<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-assembly-plugin</artifactId>
    <version>3.3.0</version>
    <executions>
        <execution>
            <id>package-with-dependencies</id>
            <phase>package</phase>
            <goals>
                <goal>single</goal>
            </goals>
            <configuration>
                <descriptorRefs>
                    <descriptorRef>jar-with-dependencies</descriptorRef>
                </descriptorRefs>
            </configuration>
        </execution>
    </executions>
</plugin>
```

`flink-connector-crawler` 目录结构如下：  
![jdbcsourcesplit32](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit32.jpg)    

### `flink-connector-crawler` 实现 
split 包下的 `CrawlerParameterValuesProvider` & `CrawlerIdsParametersProvider` 是为了获取 ids集合。 这里需要特别注意 `Serializable[][] parameters` 二维数组值得存储方式，纵向代表的是ids 长度，横向代表的是 id参数具体的值。若参数是多个时，只需横向插入值即可。     
```java
@Experimental
public interface CrawlerParameterValuesProvider {

    /** Returns the necessary parameters array to use for query in parallel a table. */
    Serializable[][] getParameterValues();
}
``` 

```java
@Experimental
public class CrawlerIdsParametersProvider implements CrawlerParameterValuesProvider {
    private static final Logger logger = LoggerFactory.getLogger(CrawlerIdsParametersProvider.class);
    private static final ObjectMapper objectMapper = new ObjectMapper();
    private static final OkHttpClient httpClient = new OkHttpClient();
    String idsURL = "http://192.168.0.2:8082/get/ids";

    @Override
    public Serializable[][] getParameterValues() {
        try {
            Request request = new Request.Builder()
                    .url(idsURL)
                    .build();

            // 2. 发送请求并处理响应
            try (Response response = httpClient.newCall(request).execute()) {
                if (!response.isSuccessful()) {
                    throw new RuntimeException("请求失败: " + response.code());
                }
                // 3. 解析 JSON 并转换为二维数组
                Integer[] ids = objectMapper.readValue(response.body().string(), Integer[].class);
                Serializable[][] parameters = new Serializable[ids.length][1];
                for (int i = 0; i < ids.length; i++) {
                    parameters[i] = new Integer[]{ids[i]};
                }
                return parameters;
            }
        } catch (IOException e) {
            throw new RuntimeException("HTTP请求异常", e);
        }
    }
}
```   

**CrawlerInputFormat** 是核心类，createInputSplits() 方法负责将二维数组转换成 `GenericInputSplit` 类型，返回 InputSplit[]。  CrawlerInputFormat#open() 根据认领到的 InputSplit， 获取 id，请求页面接口。      
```java
public class CrawlerInputFormat extends RichInputFormat<String, InputSplit> {
    private static final Logger logger = LoggerFactory.getLogger(CrawlerInputFormat.class);

    protected Object[][] parameterValues;
    protected boolean hasNext;

    //     Document doc = null;
    String resp = null;

    @Override
    public void configure(Configuration parameters) {

    }

    public static CrawlerInputFormatBuilder buildJdbcInputFormat() {
        return new CrawlerInputFormatBuilder();
    }

    @Override
    public BaseStatistics getStatistics(BaseStatistics cachedStatistics) throws IOException {
        return cachedStatistics;
    }

    @Override
    public InputSplit[] createInputSplits(int minNumSplits) throws IOException {
        logger.info("CrawlerInputFormat createInputSplits: {}", minNumSplits);
        if (parameterValues == null) {
            return new GenericInputSplit[]{new GenericInputSplit(0, 1)};
        }
        GenericInputSplit[] ret = new GenericInputSplit[parameterValues.length];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = new GenericInputSplit(i, ret.length);
        }
        return ret;
    }

    @Override
    public InputSplitAssigner getInputSplitAssigner(InputSplit[] inputSplits) {
        return new DefaultInputSplitAssigner(inputSplits);
    }

    @Override
    public void open(InputSplit split) throws IOException {
        /*
          在 open() 方法执行爬虫业务
         */
        Object param = parameterValues[split.getSplitNumber()][0];
        String url = String.format(
                "http://192.168.0.2:8082/get/id/%s",
                (Integer) param);
        logger.info("CrawlerInputFormat open() url: {}", url);
        OkHttpClient httpClient = new OkHttpClient();
        Request request = new Request.Builder()
                .url(url)
                .build();
        try (Response response = httpClient.newCall(request).execute()) {
            if (!response.isSuccessful()) {
                throw new RuntimeException("请求失败: " + response.code());
            }
            resp = response.body().string();
            logger.info("resp info: {}", resp);
        }
        hasNext = true;
    }

    @Override
    public boolean reachedEnd() throws IOException {
        return !hasNext;
    }

    @Override
    public String nextRecord(String reuse) throws IOException {
        if (!hasNext) {
            return null;
        }

        logger.info("CrawlerInputFormat nextRecord: {}", resp);
        hasNext = false;
        return resp;
    }

    @Override
    public void close() throws IOException {

    }

    public static class CrawlerInputFormatBuilder {
        private final CrawlerInputFormat format;

        public CrawlerInputFormatBuilder() {
            this.format = new CrawlerInputFormat();
        }


        public CrawlerInputFormatBuilder setParametersProvider(
                CrawlerParameterValuesProvider parameterValuesProvider) {
            format.parameterValues = parameterValuesProvider.getParameterValues();
            return this;
        }


        public CrawlerInputFormat finish() {
            if (format.parameterValues == null) {
                logger.debug("No input splitting configured (data will be read with parallelism 1).");
            }
            return format;
        }
    }
}
```   

到这里，我的 `flink-connector-crawler` 支持 split 功能就已经开发完了， 现在我需要将它接入到 Job 中。   

### 创建 Flink Job 项目  
仍然还是在 Flink 1.15.4 的源码项目中，在 `flink-examples` 模块中创建 `flink-blog` 子模块。    
![jdbcsourcesplit33](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit33.jpg)  

在 pom.xml 添加依赖和打包插件,我们需要将 `flink-connector-crawler` 添加到 pom 中，内容如下：  
```xml
<dependencies>
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
    <dependency>
        <groupId>org.apache.flink</groupId>
        <artifactId>flink-state-processor-api</artifactId>
        <version>${flink.version}</version>
        <scope>provided</scope>
    </dependency>
    <dependency>
        <groupId>org.apache.flink</groupId>
        <artifactId>flink-connector-crawler</artifactId>
        <version>${flink.version}</version>
    </dependency>
    <dependency>
        <groupId>org.apache.flink</groupId>
        <artifactId>flink-core</artifactId>
        <version>${flink.version}</version>
        <scope>provided</scope>
    </dependency>

    <!-- 引入日志管理相关依赖-->
    <dependency>
        <groupId>org.slf4j</groupId>
        <artifactId>slf4j-api</artifactId>
        <version>${slf4j.version}</version>
    </dependency>
    <dependency>
        <groupId>joda-time</groupId>
        <artifactId>joda-time</artifactId>
        <version>2.14.0</version>
    </dependency>
</dependencies>

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
    </plugins>
</build>
```

**Crawler2Console**  
```java
public class Crawler2Console {
    private static final Logger logger = LoggerFactory.getLogger(Crawler2Console.class);

    public static void main(String[] args) throws Exception {
        Configuration configuration = new Configuration();
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment(configuration);
        env.setRuntimeMode(RuntimeExecutionMode.AUTOMATIC);
        env.setParallelism(2);

        CrawlerIdsParametersProvider crawlerIdsParametersProvider = new CrawlerIdsParametersProvider();
        DataStreamSource<String> myDataStream = env.createInput(CrawlerInputFormat.buildJdbcInputFormat()
                        .setParametersProvider(crawlerIdsParametersProvider)
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

### 打包 
执行 `mvn clean install -DskipTests -Dfast -Dscala-2.12 -Dcheckstyle.skip=true -Dspotless.check.skip=true -Drat.skip=true` 命令打包 Flink 源码。  

### 运行 & 测试  
将 `flink-connector-crawler` jar 放入 flink lib pvc 中， 使用 flink run 命令部署 Application Model Job。   

查看 log，可以观察到 2个 TaskManager 分别承担部分任务分片。    

注意：Flink on K8s 启动 Job 是比较耗时的，为了演示 2个 TaskManager 都可以分配到任务分片，我在 `CrawlerInputFormat#open()` 方法添加 ` Thread.sleep(3000);` , 避免出现一个TaskManager正在启动时，另一个就已经处理完数据了。 这样你会在 log 中看到 任务都被分配到 1个 TaskManager了。这都是因为处理速度太快，测试数据少造成的。  
```java
try {
    Thread.sleep(3000);
} catch (Exception e) {
    e.printStackTrace();
}
```

![jdbcsourcesplit34](http://img.xinzhuxiansheng.com/blogimgs/flink/jdbcsourcesplit34.jpg)  

到这里，从0到1 开发分布式爬虫示例已经完成。这样让我加深了对 Flink InputFormat 的了解，也可以让我快速开发一个 master-worker 架构的程序。   