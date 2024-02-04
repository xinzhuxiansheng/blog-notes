## Flink Run 入门测试 支持 Checkpoint & Savepoint        

>Flink version: 1.15.4      

### 支持 Checkpoint & Savepoint  

从 Checkpoint 、 Savepoint 都添加 `-s` 参数, 需注意 两者目录的区别  

>若需保留 Checkpoint 时，在 Jar 作业代码中，请务必配置 `execution.checkpointing.externalized-checkpoint-retention` 为 RETAIN_ON_CANECLLATION ，否则的话，当你 Job 取消后，checkpoint 数据会丢死，只有设置了 RETAIN_ON_CANCELLATION 才会保留 Checkpoint 数据。   

#### 若是 Checkpoint 
```shell
./flink run \
-m localhost:8081 \
-s file:///root/flink/checkpoint/11303b0f9334bc42656e86adc46c91d3/chk-20 \
-d \
-c com.yzhou.flink.example.GettingStartedCase02 \
job/flink-jar-1.0-SNAPSHOT.jar \
--arg01 yzhouputarg01  
```

#### 若是 Savepoint     

```shell
./flink run \
-m localhost:8081 \
-s file:///root/flink/savepoint/savepoint-c7354f-bb04f0042cd9 \
-d \
-c com.yzhou.flink.example.GettingStartedCase02 \
job/flink-jar-1.0-SNAPSHOT.jar \
--arg01 yzhouputarg01  
```
