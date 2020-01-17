
kafka的消息传输保证机制非常直观。当producer向broker发送消息时，一旦这条消息被commit
，由于副本机制(replication)的存在，它就不会丢失。但是如果producer发送数据给broker后，遇到的网络问题而造成通信中断，那producer就无法判断该消息是否已经提交(commit)。虽然kafka无法确定网络故障期间发生了什么，但是producer可以retry多次，确保消息已经正确传输到broker中，所以目前kafka实现的是at least once。

## 幂等性
`场景`
所谓幂等性，就是对接口的多次调用所产生的结果和调用一次是一致的。生产者在进行重试的时候有可能会重复写入消息，二使用kafka的幂等性功能就可以避免这种情况。
幂等性是有条件的:
* 只能保证Producer在单个会话内不丢不重，如果Producer出现意外挂掉再重启是无法保证的(幂等性情况下，是无法获取之前的状态信息，因此是无法做到跨会话级别的不丢不重)；
* 幂等性不能跨多个Topic-Partition，只能保证单个partition内的幂等性，当涉及多个Topic-Partition时，这中间的状态并没有同步。

开启幂等性功能的方式很简单，只需要显示地将生产者客户端参数`enable.idempotence`设置为true即可(这个参数的默认值为false),参考如下：
```java
properties.put(ProducerConfig.ENABLE_IDEMPOTENCE_CONFIG,true);
或者
properties.put("enable.idempotence",true);
```
  不过如果要确保幂等性功能正常，还需要确保生产者客户端的 `retries` ,`acks` ,`max.in.flight.requests.per.connection`这个几个参数不被配置错。实际上在使用幂等性功能的时候，用户完全可以不用配置(也不建议配置)这几个参数。

**retries**
  如果用户显示地指定了retries参数，那么这个参数值必须要大于0，否则会报出`ConfigException`：
  ```java
  org.apache.kafka.common.config.ConfigException: Must set retries to non-zero when using the idempotent producer.
  ```
  如果用户没有显示地指定retries参数，那么KafkaProducer会将它置为`Integer.MAX_VALUE`。
  
**max.in.flight.requests.per.connection**
  同时还需要保证`max.in.flight.requests.per.connection`参数的值不能大于5(这个参数的值默认为5)，否则也会报出`ConfigException`：
  ```java
  org.apache.kafka.common.config.ConfigException:Must set max.in.flight.requests.per.connection to at most 5 to use the idempotent producer.
  ```
**acks**
如果用户显示地指定了acks参数，那么还需要保证这个参数的值为-1(all)，如果不为-1(这个参数的值默认为1)，那么也会报出`ConfigException`：
```java
 org.apache.kafka.common.config.ConfigException:Must set acks to all in order to at most 5 to use the idempotent producer.
```
如果用户没有显式地指定这个参数，那么KafkaProducer会将它置为-1。开启幂等性功能之后，生产者就可以如同未开启幂等时一样发送消息了。

为了实现生产者的幂等性，kafka为此引入了 producer id(以下简称PID)和序列化(sequence number)这两个概念。
每个新的生产者实例在初始化的时候都会被分配一个PID，这个PID对用户而言是完全透明的。对于每个PID，消息发送到的每个分区都有对应的序列号，这些序列号从0开始单调递增。生产者每发送一次消息就会将<PID,分区>对应的序列号的值加1.
  broker端会在内存中每一对<PID，分区>维护一个序列号。对于收到的每一条消息，只有当它的序列号的值(SN_new)比broker端中维护的对应的序列号的值(SN_old)大1(即SN_new=SN_old+1)时，broker才会接受它。如果SN_new<SN_old+1，那么说明消息被重复写入，broker可以直接将其丢弃。如果SN_new>SN_old+1，那么说明中间有数据尚未写入，出现了乱序，暗示可能有消息丢失，对应的生产者会抛出`OutOfOrderSequenceException`，这个异常是一个严重的异常，后续的诸如send(),beginTransaction(),commitTransaction()等方法的调用都会抛出IllegalStateException异常。
  引入序列号来实现幂等也只是针对每一个<PID,分区>而言的，也就是说，kafka的幂等只能保证单个生产者会话(session)中单分区的幂等。
  ```java
ProducerRecord<String,String> record = new ProducerRecord<>(topic,"key","msg");
producer.send(record);
producer.send(record);
  ```
  注意，上面示例中发送了两条相同的消息，不过这仅仅是指消息内容相同，但对kafka而言是两条不同的消息，因为会为这两条消息分配不同的序列号。kafka并不会保证消息内容的幂等。