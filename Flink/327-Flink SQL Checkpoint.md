## Flink SQL Checkpoint 

>针对 Flink SQL 中的状态是如何保障容错性？      

### Checkpoint 使用 
只需要开启 Checkpoint 即可，下面结合案例演示一下：          

通过 Flink SQL 实现一个类似于单词计数这种聚合需求的案例, 这种案例里面 Flink SQL 会通过状态维护数据的中间结果，也便于测试。(若使用 Flink SQL 双流 JOIN 去验证也是可以的)     

```java
val env = StreamExecutionEnvironment.getExecutionEnvironment
env.setRuntimeMode(RuntimeExecutionMode.STREAMING)

// 开启Checkpoint
env.enableCheckpointing(1000*10) // 为了便于观察方便，设置为10秒一次
// 获取Checkpoint的配置对
val cpConfig = env.getCheckpointConfig
// 在任务故障和手工停止任务时 都会保留之前生成的Checkpoint数据
cpConfig.setExternalizedCheckpointCleanup(ExternalizedCheckpointCleanup.RETAIN_ON_CANCELLATION);
// 设置Checkpoint后的状态数据的存储位置
cpConfig.setCheckpointStorage("file:///Users/a/TMP/flink_checkpoint");
val tEnv = StreamTableEnvironment.create(env)
``` 










