## Flink Checkpoint 使用    

### 本地开发使用 Checkpoint 

```java

String checkpointPath = "file:///Users/a/TMP/flink_checkpoint"; 

 // 0. 创建流式执行环境
StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
env.setParallelism(parallelism);
env.setRuntimeMode(RuntimeExecutionMode.AUTOMATIC);

// 1. 配置checkpoint
env.enableCheckpointing(interval);
CheckpointConfig checkpointConfig = env.getCheckpointConfig();
checkpointConfig.setCheckpointingMode(CheckpointingMode.EXACTLY_ONCE);
checkpointConfig.setMaxConcurrentCheckpoints(1);
checkpointConfig.enableExternalizedCheckpoints(CheckpointConfig.ExternalizedCheckpointCleanup.RETAIN_ON_CANCELLATION);
checkpointConfig.enableUnalignedCheckpoints();
checkpointConfig.setCheckpointStorage(checkpointPath);

// 2. 配置Kafka
Properties properties = new Properties();
properties.setProperty("bootstrap.servers", kafkaServer);
properties.setProperty("group.id", "wc-consumer-group");
properties.setProperty("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
properties.setProperty("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
properties.setProperty("auto.offset.reset", "latest");
```

**Checkpoint 目录** 
```shell    
➜  flink_checkpoint ls
b8bcf8e1977347b72a2e20ab1bcc172b cf7f437dd47741c6fd217ef1f42b698e

➜  cf7f437dd47741c6fd217ef1f42b698e ls
chk-56    shared    taskowned
```
