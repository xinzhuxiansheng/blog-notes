Producer同步发送消息是需要经过RecordAccumulator(消息累加器的)。
**下面提供Producer同步发送消息的demo：**
```java
public static void main(String[] args) throws InterruptedException, ExecutionException {

    Properties properties = new Properties();
    properties.put("bootstrap.servers", "xxxxxxxxxxxxxxxx");
    properties.put("batch.size", "1048576");
    properties.put("compression.type", "snappy");
    properties.put("linger.ms", "100"); 
    properties.put("buffer.memory", "67108864");
    properties.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
    properties.put("value.serializer", "org.apache.kafka.common.serialization.StringSerializer");

    Producer<String, String> producer = new KafkaProducer<String, String>(properties);

    Long i = 0L;
    while (true) {
        String data = i + "xxxxxxxxxtestdata";
        Long startTime = System.currentTimeMillis();
        RecordMetadata result =  producer.send(new ProducerRecord<String, String>("test01", data)).get();
        Long endTime = System.currentTimeMillis();
        System.out.println("time: "+ (endTime - startTime));
        System.out.println(i);
        Thread.currentThread().sleep(1000L);
        i++;
    }
}
```

1. producer的send()方法返回值`Future<RecordMetadata>`，只需要调用Future的get()方法阻塞Producer线程继续执行， 所以 同步发送就是等待send()方法返回的结果值 RecordMetaData。
```java
RecordMetadata result =  producer.send(new ProducerRecord<String, String>("test01", data)).get();
```

2. 查看Producer的send()方法调用逻辑，下面代码给出的是KafkaProducer.java的 doSend()方法。这里涉及到2个class，一个是RecordAppendResult,一个是FutureRecordMetadata。
```java
//KafkaProducer.java doSend()方法 代码片断
RecordAccumulator.RecordAppendResult result = accumulator.append(tp, timestamp, serializedKey,
                    serializedValue, headers, interceptCallback, remainingWaitMs);
if (result.batchIsFull || result.newBatchCreated) {
    log.trace("Waking up the sender since topic {} partition {} is either full or getting a new batch", record.topic(), partition);
    this.sender.wakeup();
}
return result.future;
```

结合下面类图，和 序号1 方法调用`producer.send(new ProducerRecord<String, String>("test01", data)).get()` 。 所以这里需要看的是FutureRecordMetadata实现Future<RecordMetadata>的get()方法， get()方法又调用了ProduceRequestResult的await(),它利用CountDownLatch实现线程等待机制。
下面给出get()方法代码：
```java
//FutureRecordMetadata.java get()
@Override
public RecordMetadata get() throws InterruptedException, ExecutionException {
    this.result.await();
    if (nextRecordMetadata != null)
        return nextRecordMetadata.get();
    return valueOrError();
}
```

> 以上梳理了 梳理几个相关类的 关于get()方法的调用。

**接下来，问题是 ProduceRequestResult的await() 什么时候结束？**

3. ProduceRequestResult的done()方法，它标记已完成，并解除阻塞等待线程的状态
这里不过多说明 sender线程是如何读取ProduceBatch的数据和sender线程的NIO数据发送， 主要是看 NIO数据发送回调处理函数。  看ProduceBatch.java completeFutureAndFireCallbacks()方法，它会执行produceFuture.done();

```java
private void completeFutureAndFireCallbacks(long baseOffset, long logAppendTime, RuntimeException exception) {
    // Set the future before invoking the callbacks as we rely on its state for the `onCompletion` call
    produceFuture.set(baseOffset, logAppendTime, exception);

    // execute callbacks
    for (Thunk thunk : thunks) {
        try {
            if (exception == null) {
                RecordMetadata metadata = thunk.future.value();
                if (thunk.callback != null)
                    thunk.callback.onCompletion(metadata, null);
            } else {
                if (thunk.callback != null)
                    thunk.callback.onCompletion(null, exception);
            }
        } catch (Exception e) {
            log.error("Error executing user-provided callback on message for topic-partition '{}'", topicPartition, e);
        }
    }

    produceFuture.done();
}
```

我想这里你差不多可以明白， future的get()是如何与sender线程的关系了。

4. RecordAccumulator作用消息累加，以至于达到批量。 Sender线程才是负责发送， `this.accumulator.ready` RecordAccumulator的ready()方法会判断 消息集中哪些消息可以发送。
`下面给出Sender线程的一部分处理逻辑：`
```java
 private long sendProducerData(long now) {
        Cluster cluster = metadata.fetch();
        // get the list of partitions with data ready to send
        RecordAccumulator.ReadyCheckResult result = this.accumulator.ready(cluster, now);
        //... 省略部分代码
```

RecordAccumulator的ready()方法 它会判断重试间隔，linger.ms是否达到，batch.size是否达到，消息是否过期等等。 它仍然要经历这些判断，当然这里最容易达到的条件是 linger.ms。
```java
boolean backingOff = batch.attempts() > 0 && waitedTimeMs < retryBackoffMs;
long timeToWaitMs = backingOff ? retryBackoffMs : lingerMs;
boolean full = deque.size() > 1 || batch.isFull();
boolean expired = waitedTimeMs >= timeToWaitMs;
boolean sendable = full || expired || exhausted || closed || flushInProgress();
```

