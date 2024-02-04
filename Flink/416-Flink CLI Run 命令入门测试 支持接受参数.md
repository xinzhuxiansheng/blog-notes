## Flink Run 入门测试 支持接受参数       

>Flink version: 1.15.4      

### 如何给 main() 接受参数  
使用 Flink 提供的 ParameterTool 参数工具类，接受参数， 例如，我们要传 `参数名为 arg01`，在 flink run 命令中添加 `--arg01` 即可。 
```java
final ParameterTool params = ParameterTool.fromArgs(args);   
```

>注意：请注意查看 Blog 中 `log 查看`  

### 测试代码    

```java
/**
 * 入门案例二，使用 ParameterTool 工具类 接受 main() 参数
 */
public class GettingStartedCase02 {
    private static final Logger logger = LoggerFactory.getLogger(GettingStartedCase01.class);

    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        env.setParallelism(1);
        env.setRuntimeMode(RuntimeExecutionMode.AUTOMATIC);

        logger.info("receive main() params： ");
        // 解析命令行参数
        final ParameterTool params = ParameterTool.fromArgs(args);
        String arg01 = params.get("arg01", "defaultArg01");
        logger.info("GettingStartedCase02 print params: arg01 {}",arg01);

        // 配置checkpoint
        env.enableCheckpointing(60000);
        CheckpointConfig checkpointConfig = env.getCheckpointConfig();
        checkpointConfig.setCheckpointingMode(CheckpointingMode.EXACTLY_ONCE);
        checkpointConfig.setMaxConcurrentCheckpoints(1);
        checkpointConfig.enableExternalizedCheckpoints(CheckpointConfig.ExternalizedCheckpointCleanup.RETAIN_ON_CANCELLATION);
        checkpointConfig.enableUnalignedCheckpoints();
        // checkpointConfig.setCheckpointStorage();

        // 创建 Kafka
        KafkaSource<String> kafkaSource = KafkaSource.<String>builder()
                .setBootstrapServers("dn-kafka3:9092")
                .setTopics("yzhoujsontp01")
                .setGroupId("ygid02021")
                .setStartingOffsets(OffsetsInitializer.committedOffsets(OffsetResetStrategy.LATEST))
                .setValueOnlyDeserializer(new SimpleStringSchema()).build();

        DataStreamSource<String> kfkStream = env.fromSource(kafkaSource, WatermarkStrategy.noWatermarks(), "kfk-source");

        kfkStream.print();
        env.execute("flink jar01");
    }
}
```

### 提交 Jar  

* -m,--jobmanager <arg>             Address of the JobManager to which to
                                     connect. Use this flag to connect to a
                                     different JobManager than the one specified
                                     in the configuration. Attention: This
                                     option is respected only if the
                                     high-availability configuration is NONE.       

* -d,--detached                     If present, runs the job in
                                     detached mode
表示 “分离模式”，当 Job 提交完后，立即退出客户端。 

* -c,--class <classname>                Class with the program entry
                                        point ("main()" method). Only
                                        needed if the JAR file does not
                                        specify the class in its
                                        manifest.       

表示 main()的启动类     

```shell
./flink run \
-m localhost:8081 \
-d \
-c com.yzhou.flink.example.GettingStartedCase02 \
job/flink-jar-1.0-SNAPSHOT.jar \
--arg01 yzhouputarg01  
```

### log 查看          
在 Flink Standalone模式下，Flink run 提交作业后，main()方法中打印的log 日志，会`flink-root-client-xxx.log` 中，log 打印如下：       

```
2024-02-04 11:41:34,154 INFO  com.yzhou.flink.example.GettingStartedCase02                 [] - receive main() params： 
2024-02-04 11:41:34,156 INFO  com.yzhou.flink.example.GettingStartedCase02                 [] - GettingStartedCase02 print params: arg01 yzhouputarg01
```  

