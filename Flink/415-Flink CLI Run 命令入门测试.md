## Flink Run 入门测试       

>Flink version: 1.15.4      

### 测试代码    

```java
/**
 * 入门案例一，读取 Kafka 再打印
 */
public class GettingStartedCase01 {
    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        env.setParallelism(1);
        env.setRuntimeMode(RuntimeExecutionMode.AUTOMATIC);

        // 配置checkpoint
        env.enableCheckpointing(10000);
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
-c com.yzhou.flink.example.GettingStartedCase01 \
job/flink-jar-1.0-SNAPSHOT.jar     
```



