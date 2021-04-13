# flink.partition-discovery.interval-millis参数在Flink的工作原理

* 

```java
private FlinkKafkaConsumer(
    List<String> topics,
    Pattern subscriptionPattern,
    KafkaDeserializationSchema<T> deserializer,
    Properties props) {
    
    //这里是KEY_PARTITION_DISCOVERY_INTERVAL_MILLIS参数
    super(
        topics,
        subscriptionPattern,
        deserializer,
        getLong(
            checkNotNull(props, "props"),
            KEY_PARTITION_DISCOVERY_INTERVAL_MILLIS, PARTITION_DISCOVERY_DISABLED),
        !getBoolean(props, KEY_DISABLE_METRICS, false));

    props = KafkaSdkSupport.addKafkaSdkSpeicalizedProperty(props);

    this.properties = props;
    setDeserializer(this.properties);

    // configure the polling timeout
    try {
        if (properties.containsKey(KEY_POLL_TIMEOUT)) {
            this.pollTimeout = Long.parseLong(properties.getProperty(KEY_POLL_TIMEOUT));
        } else {
            this.pollTimeout = DEFAULT_POLL_TIMEOUT;
        }
    }
    catch (Exception e) {
        throw new IllegalArgumentException("Cannot parse poll timeout for '" + KEY_POLL_TIMEOUT + '\'', e);
    }
}
```

## 类之间的关系

* public class FlinkKafkaConsumer<T> extends FlinkKafkaConsumerBase<T>
* public abstract class FlinkKafkaConsumerBase<T> extends RichParallelSourceFunction<T> implements
   CheckpointListener,
   ResultTypeQueryable<T>,
   CheckpointedFunction
* 至于 FlinkKafkaConsumerBase从哪里被执行 run(), 这个目前还没了解这块（？？？）

**以下方法执行过程，已本人debug为主**
`FlinkKafkaConsumerBase.java  run()方法为入口`

```java
//因为createFetcher是抽象方法，所以调用的是FlinkKafkaConsumer.java的 createFetcher()方法
this.kafkaFetcher = createFetcher(
    sourceContext,
    subscribedPartitionsToStartOffsets,
    periodicWatermarkAssigner,
    punctuatedWatermarkAssigner,
    (StreamingRuntimeContext) getRuntimeContext(),
    offsetCommitMode,
    getRuntimeContext().getMetricGroup().addGroup(KAFKA_CONSUMER_METRICS_GROUP),
    useMetrics);
```

FlinkKafkaConsumer.java createFetcher() 会去创建 new KafkaFetcher<>(...);

```java
@Override
protected AbstractFetcher<T, ?> createFetcher(
    SourceContext<T> sourceContext,
    Map<KafkaTopicPartition, Long> assignedPartitionsWithInitialOffsets,
    SerializedValue<AssignerWithPeriodicWatermarks<T>> watermarksPeriodic,
    SerializedValue<AssignerWithPunctuatedWatermarks<T>> watermarksPunctuated,
    StreamingRuntimeContext runtimeContext,
    OffsetCommitMode offsetCommitMode,
    MetricGroup consumerMetricGroup,
    boolean useMetrics) throws Exception {

    //... 删除部分代码

    return new KafkaFetcher<>(
        sourceContext,
        assignedPartitionsWithInitialOffsets,
        watermarksPeriodic,
        watermarksPunctuated,
        runtimeContext.getProcessingTimeService(),
        runtimeContext.getExecutionConfig().getAutoWatermarkInterval(),
        runtimeContext.getUserCodeClassLoader(),
        runtimeContext.getTaskNameWithSubtasks(),
        deserializer,
        properties,
        pollTimeout,
        runtimeContext.getMetricGroup(),
        consumerMetricGroup,
        useMetrics);
}
```

在new KafkaFetcher()的构造方法中会 new KafkaConsumerThread()

### 重点详细说明 KafkaConsumerThread.java

增加分区后，debug的断点会进入 run()方法

```java
try {
    if (hasAssignedPartitions) {
        newPartitions = unassignedPartitionsQueue.pollBatch();
    }
    else {
        // if no assigned partitions block until we get at least one
        // instead of hot spinning this loop. We rely on a fact that
        // unassignedPartitionsQueue will be closed on a shutdown, so
        // we don't block indefinitely
        newPartitions = unassignedPartitionsQueue.getBatchBlocking();
    }
    //newPartitions有值，接下来会这行这里 ？？
    //这里会存在疑问？ 增加分区后，unassignedPartitionsQueue队列是如何add新分区的？？？
    if (newPartitions != null) {
        reassignPartitions(newPartitions);
    }
} catch (AbortedReassignmentException e) {
    continue;
}
```

**增加分区后，unassignedPartitionsQueue队列是如何add新分区的？？？**
在KafkaConsumerThread.java 唯一有引用add的地方，断点一直都没进入

```java
while(var7.hasNext()) {
    KafkaTopicPartitionState<TopicPartition> newPartition = (KafkaTopicPartitionState)var7.next();
    this.unassignedPartitionsQueue.add(newPartition);
}
```

不断debug，才关注到： unassignedPartitionsQueue对象没有在KafkaConsumerThread.java里面new ，而是直接通过构造方法传参过来，所以...

```java
this.unassignedPartitionsQueue = (ClosableBlockingQueue)Preconditions.checkNotNull(unassignedPartitionsQueue);
```

所以跟踪形参`unassignedPartitionsQueue` 在哪里赋值的 ......
并且还告诉了新分区从哪里开始消费
**AbstractFetcher.java**

```java
public void addDiscoveredPartitions(List<KafkaTopicPartition> newPartitions) throws IOException, ClassNotFoundException {
    List<KafkaTopicPartitionState<KPH>> newPartitionStates = createPartitionStateHolders(
            newPartitions,
            //从最早点开始消费
            KafkaTopicPartitionStateSentinel.EARLIEST_OFFSET,
            timestampWatermarkMode,
            watermarksPeriodic,
            watermarksPunctuated,
            userCodeClassLoader);

    if (useMetrics) {
        registerOffsetMetrics(consumerMetricGroup, newPartitionStates);
    }

    for (KafkaTopicPartitionState<KPH> newPartitionState : newPartitionStates) {
        // the ordering is crucial here; first register the state holder, then
        // push it to the partitions queue to be read
        subscribedPartitionStates.add(newPartitionState);
        unassignedPartitionsQueue.add(newPartitionState);
    }
}
```

现在我们大概知道，当扩分区时，由外部方法增加 新分区，并且从最早点开始消费

