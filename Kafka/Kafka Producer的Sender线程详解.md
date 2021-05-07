--In Blog
--Tags: Kafka

# Kafka Producer的Sender线程详解

>涉及Kafka是2.2.1版本

## 1. Sender线程的定义
在KafkaProducer的构造方法中会启动一个守护线程`KafkaThread` 称为Sender线程，线程名称为"kafka-producer-network-thread|"+clientId，通过创建Sender对象来处理该线程的run()方法。 它负责从RecordAccumulator中获取消息并将其发送到Kafka Broker中。
```java
this.sender = newSender(logContext, kafkaClient, this.metadata);
String ioThreadName = NETWORK_THREAD_PREFIX + " | " + clientId;
this.ioThread = new KafkaThread(ioThreadName, this.sender, true);
this.ioThread.start();
```

## 2. KafkaThread与Sender关系
**2.1.** KafkaThread 继承了Thread，并且在自身的构造方法中，调用了父类的构造方法
```java
public KafkaThread(final String name, Runnable runnable, boolean daemon) {
    super(runnable, name);
    configureThread(name, daemon);
}
```     
**2.2** Sender 实现了 Runnable的run()。创建KafkaThread对象将Sender对象赋值给父类Thread的`target`字段
```java
 private Runnable target;
```

## 3. Sender的run()
当KafkaThread的守护线程启动后，会执行Sender的run()方法，进入while(running)循环。当running=true时，会循环执行runOnce()。接下来我们来了解runOnce()方法。          
**runOnce()**
在runOnce()方法大篇幅的涉及到事务(Transaction)的逻辑处理,后续会推送Kafka的Producer事务的讲解，这里就不过多阐述。 除事务相关，只剩下3行代码。
`接下来，重点分析这3行代码`     
```java
void runOnce() {
    if (transactionManager != null) {
        //...省略事务处理逻辑
    }

    long currentTimeMs = time.milliseconds();
    long pollTimeout = sendProducerData(currentTimeMs);
    client.poll(pollTimeout, currentTimeMs);
}
```

### 4. long currentTimeMs = time.milliseconds()
time是接口，而它的派生类是 SystemTime，所以在Kafka的源码中涉及time.milliseconds(),只是为了获取当前时间戳(long)
```java
@Override
public long milliseconds() {
    return System.currentTimeMillis();
}
```

`由于代码篇幅过长，之前的推文粘贴大部分代码，并且在代码中增加标记点, 紧接着下面会通过标记点来讲解每段代码的逻辑， 博主发现，代码片断过多整合在一起，代码与讲解文字离的太远，无法让读者有很好的阅读体验。所以这里调整为 代码拆成小片断，每个小片段统一加上片断所属的方法名,并且将每个小片断再加上 步骤编号`      

>面向对象编程、查看方法注释     

### 4.1 long pollTimeout = sendProducerData(currentTimeMs)
之前我在讲解MemoryRecordsBuilder的推文中涉及到 sendProducerData()方法，在这里再详细的补充这里面的处理逻辑。         

*Sender.sendProducerData() : step01* 
```java
Cluster cluster = metadata.fetch();
// get the list of partitions with data ready to send
RecordAccumulator.ReadyCheckResult result = this.accumulator.ready(cluster, now);
```

**Step01**: 获取存在要发送的Topic的分区副本Leader为unknown的集合，获取RecordAccumulator中batches对象中每个TopicPartition是否存在可以准备发送的ProducerBatch， 这里还涉及到 "linger.ms"与是否重试及重试间隔时间的判断。所以适当增加linger.ms，可以提高Producer发送性能。当然也必须承受 延迟发送所带来的风险。    
```java
boolean backingOff = batch.attempts() > 0 && waitedTimeMs < retryBackoffMs;
long timeToWaitMs = backingOff ? retryBackoffMs : lingerMs;
```

*Sender.sendProducerData() : step02* 
```java
// if there are any partitions whose leaders are not known yet, force metadata update
if (!result.unknownLeaderTopics.isEmpty()) {
    // The set of topics with unknown leader contains topics with leader election pending as well as
    // topics which may have expired. Add the topic again to metadata to ensure it is included
    // and request metadata update, since there are messages to send to the topic.
    for (String topic : result.unknownLeaderTopics)
        this.metadata.add(topic);

    log.debug("Requesting metadata update due to unknown leader topics from the batched records: {}",
        result.unknownLeaderTopics);
    this.metadata.requestUpdate();
}
```

**Step02**: 获取存在要发送的Topic的分区副本Leader为unknown的集合，获取RecordAccumulator中batches对象中每个TopicPartition是否存在可以准备发送的ProducerBatch， 这里还涉及到 "linger.ms"与是否重试及重试间隔时间的判断。所以适当增加linger.ms，可以提高Producer发送性能。当然也必须承受 延迟发送所带来的风险。    
```java
boolean backingOff = batch.attempts() > 0 && waitedTimeMs < retryBackoffMs;
long timeToWaitMs = backingOff ? retryBackoffMs : lingerMs;
```