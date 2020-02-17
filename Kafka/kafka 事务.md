

#### 事务
幂等性并不能跨多个分区运作，而`事务`可以弥补这个缺陷。事务可以保证对多个分区写入操作的原子性。操作的原子性是指多个操作要么全部成功，要么全部失败，不存在部分成功、部分失败的可能。

对流失应用(Stream Processing Applications)而言，一个典型的应用模式为"consume-transform-produce"。在这种模式下消费和生产并存：应用程序从某个主题中消费消息，然后经过一系列转换后写入另一个注入，消费者可能在提交消费位移的过程中出现问题而导致重复消费，也可能生产者重复生产消息。kafka中的事务可以使应用程序将消费消息、生产消息、提交消息位移当作原子操作来处理，同时成功或失败，即使该生产或消费会跨多个分区。

为了实现事务，应用程序必须提供唯一的transactionalId，这个transactionalId通过客户端参数`transactional.id`来显示设置，参数如下：
```java
properties.put(ProducerConfig.TRANSACTIONAL_ID_CONFIG,"transactionId");
或者
properties.put("transactional.id","transactionId");
```
事务要求生产者开启幂等特性，因此通过将transactional.id参数设置为非空从而开启事务特性的同时需要将 `enable.idempotence`设置为true(如果未显示设置，则KafkaProducer默认会将它的值设置为true)，如果用户显示地将`enable.idempotence`设置为flase，则会报出`ConfigException`:
```java
 org.apache.kafka.common.config.ConfigException:Must set a transactional.id without also enabling idempotent.
```
transactionlId与PID一一对应，两者之间所不同的是transactionalId由用户显示设置，而PID是有kafka内部分配的，另外，为了保证新的生产者启动后具有相同transactionlId的旧生产者能够立即失效，每个生产者通过transactionalId获取PID的同时，还会获取一个单调递增的producer epoch(对应下面要讲述的KafkaProducer.initRransactions()方法)。如果使用同一个transactionlId开启两个生产者，那么前一个开启的生产者会报出如下的错误：
```java
org.apache.kafka.common.errors.ProducerFencedException:Producer attempted an operation with an old epoch. Either there is a newer producer with the same transactionalId, or the producer's transaction has been expired by the broker.
```

从生产者的角度分析，通过事务，kafka可以保证跨生产者会话的消息幂等发送，以及跨生产者会话的事务恢复。前者表示具有相同transactionalId的新生产者实例被创建且工作的时候，旧的且拥有transactionalId的生产者实例将不再工作。后者指当某个生产者实例宕机后，新的生产者实例可以保证任何未完成的旧事务要么被提交，要么被中止，如此可以使新的生产者实例从一个正常的状态开始工作。

而从消费者的角度分析，事务能保证的语义相对偏弱。出于以下原因，kafka并不能保证已提交的事务中的所有消息都能够被消费：
* 对采用日志压缩策略的主题而言，事务中的某些消息有可能被清理(相同key的消息，后写入的消息会覆盖前面写入的消息)
* 事务中消息可能分布在同一个分区的多个日志分段(LogSegment)中，当老的日志分段删除时，对应的消息可能会丢失
* 消费者可以通过seek()方法访问任意offset的消息，从而可能遗漏事务中的部分消息
* 消费者在消费时可能没有分配到事务内的所有分区，如此它也就不能读取事务中的所有消息

```java
void initRransactions();
void beginTransaction() throws ProducerFencedExcetion;
void sendOffsetsToTransaction(Map<TopicPartition,OffsetAndMetadata> offsets,String consumerGroupId) throws ProducerFencedException;
void commitTransaction() throws ProducerFencedException;
void abortTransaction() throws ProducerFencedException;
```

initRransactions()方法用来初始化事务，这个方法能够执行的前提是配置了transactionalId，如果没有则会报出`IllegalStateException`：
```java
java.lang.IllegalStateException:Cannot use transactional methods without enabling transactions by setting the transactional.id configuration property
```
beginTransaction()方法用来开启事务：sendOffsetsToTransaction()方法为消费者提供在事务内的位移提交的操作；commitTransaction()方法用来提交事务；abortTransaction()方法用来中止事务，类似于事务回滚。

eg：
```java
//1.构建producer 注意参数
//...
//2 初始化事务
producer.initTransactions();

//3 开启事务
producer.beginTranscation();

try{
    //4 producer 发送消息
    producer.sender();
}catch(Exception e){
    //6  别忘记在 catch(Exception e) 中回滚
    producer.abortTransaction();
} 
```



### 事务隔离级别
在消费端有一个参数`isolation.level`，与事务有着莫大的关联，这个参数的默认值为“ read uncommitted ”， 意思是说消费端应用可以看到（消费到）未提交的事务，当然对于己提交的事务也是可见的。这个参数还可以设置为“ read committed ”，表示消费端应用不可以看到尚未提交的事务内的消息。举个例子，如果生产者开启事务并向某个分区值发送 3 条消息 msgl 、msg2 和 msg3 ，在 执行 commitTransaction （） 或 abortTransaction （）方法前，设置为“ read_committed” 的消费端应用是消费不到这些消息的，不过在 Kafka Consumer 内部会缓存这些消息，直到生产者执行 commitTransaction （）方法之后它才能将这些消息推送给消费端应用。反之，如果生产者执行了 abortTransaction （）方法，那么 Kafka Consumer 会将这些缓存的消息丢弃而不推送给消费
端应用。
日志文件中除了普通的消息，还有一种消息专门用来标志一个事务的结束，它就是控制消息(ControlBatch)。控制消息一共有两种类型：COMMIT和ABORT，分别用来表征事务已经成功提交或已经被成功中止。KafkaConsumer可以通过这个控制消息来判断对应的事务是被提交了还是被中止了，然后结合参数isolation.level配置的隔离级别来决定是否将相应的消息返回给消费端应用.