## Apache Pulsar Consumer实战

>Pulsar version: 2.11  

### 引言    
在上一篇《Apache Pulsar Producer实战》中介绍了Client的构建以及相关参数。本篇主要讲解Consumer如何构建、相关参数以及如何使用Consumer来消费数据。  


### 构建Consumer    
在构建出 PulsarClient对象后。我们可以通过 newConsumer()构建消费者。newConsumer() 会创建`ConsumerBuilder`构造器，其中涉及的主题名 (topic)属性和订阅名(subscriptiorName)性为必填项，在完善了构造器参数之后，通过subscribe()可以创建消费(Consumer)。 对应的订阅方法会自动对主题发起订阅 。     
`示例`  
```java
public static void main(String[] args) throws PulsarClientException {
    PulsarClient client = PulsarClient.builder()
            .serviceUrl("pulsar://192.168.175.129:6650")
            .build();

    Consumer<String> consumer = client.newConsumer(Schema.STRING)
            .topic("my-topic")
            .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
            .subscriptionName("subscription_test")
            .subscribe();
    
}
``` 
Consumer创建完成后就可以使用receive()来接收消息了。这种接收方法是同步的，未收到消息时对应的线程是阻寨的。还可以使用带有超时时间的receive(timeout,TimeUnit)来接收消息。  对消息进行业务处理后，需要使用`acknowledge()`来确认该消息是否已经被接收，服务端虽然会收到该确认请求，但不会再发送该条消息并标记该消息已处理。与之相对的是，如果在接收到消息之后，对该消息的处理出现问题，可以使用`negativeAcknowledge()` 来标记这条消息没有被成功确认。`这时服务端会根据配置决定是否重发该消息`。   

>注意，这点与Kafka是非常不同的，当ack未被成功确认，对于kafka来说是会再次重发该消息的。  

`Consumer ack示例`  
```java
Message<String> msg = consumer.receive();
try {
    System.out.println("Message received: " + new String(msg.getData()));
    // 确认消息已收到
    consumer.acknowledge(msg);
} catch (Exception e) {
    consumer.negativeAcknowledge(msg);
}
``` 

#### 异步接受与分区接受 
在上面内容中使用了同步方式接受消息。 Pulsar Consumer也可支持异步方式接受消息，receiveAsync被调用后会立即返回一个Completable-Future对象，你可以按照异步编程的方式处理对应的业务逻辑。    
`示例`  
```java
CompletableFuture<Message<String>> messageCompletableFuture = consumer.receiveAsync();
messageCompletableFuture.thenAccept((Message<String> msgAsync) -> {
    System.out.println("Async message received: " + msgAsync.getValue());
    try {
        consumer.acknowledge(msgAsync);
    } catch (PulsarClientException e) {
        consumer.negativeAcknowledge(msgAsync);
    }
});
```

Consumer还提供了批量接收数的方法`batchReceive`，通过batchReceive可以一次接收一批消息。`可以在确认消息时选择逐条确认或者整批确认`。注意，这里的`batchReceive`和生产者中的分批发送并无直接关系，生产者中的分批发送是将多条清息打包为一条消息发送到服务端，Consumer在消费消息时并无感知。而这里提到的批量接收消息是指消息到达Consumer之后的消息接收方式，可以理解为消费者对单条接收的封装。之前是从内存队列中取出一条消息，现在是在内存队列中取出多条消息。

>目前调整该参数不会显著加快服务端到Consumer的传输速度。     

目前社区已经提到一些优化的建议。当前的 Pusar客户端将批消息拆分为单条消息并将多条消息收集到一条消息中。在理想情况下，Pulsar客户端应该实现一个批消息(每个批消息是由一条消息或多条消息组成的)的队列。在收到单条消息时，会从批消息队列中轮询一条消息，并从批消息中再取出一条消息。      
`从消费者中批量获取消息的方式如下`
```java
Messages<String> msgs = consumer.batchReceive();
for (Message<String> msg : msgs) {
    System.out.println("Message received: " + new String(msg.getData()));
    // 可以选择确认单条消息
    // consumer.acknowledge(msg);
}
// 或者确认整批消息
consumer.acknowledge(msgs);
``` 
批量接收消息会受到`BatchReceivePolicy`参数的控制。`BatchReceivePolicy`有3个属性一最大消息数量(`maxNumMessages`)、最大字节数(`maxNumBytes`)、最大超时时间(`timeout`)。当批消息满足3 个条件中任意一个时，都会把当前所有的消息打包为一批消息并返回给用户。     

#### Consumer参数介绍   
* **consumerName**: 消费者的名字，是消费者的唯一身份凭证，需要保证全局唯一。若不指定，则系统会自动生成全局唯一的消费者名称。    
* **topicNames**: topicName 的集合，表示该消费者要消费的一组主题。  
* **topicsPattern**: 主题模式，可以按照正则表达式的规则匹配一组主题。   
* **patternAutoDiscoveryPeriod**: 和 topicsPattern 一起使用，表示每隔多长时间重新按照模式匹配主题。 
* **regexSubscriptionMode**: 正则订阅模式的类型。使用正则表达式订阅主题时，你可以选择订阅哪种类型的主题。PersistentOnly 表示只订阅持久性主题；NonPersistentOnly 表示仅订阅非持久性主题;AlTopics 表示订阅持久性和非持久两种主题。        
* **subscriptionType**: 定义订阅模式，它分为独占，故障转移，共享、键共享    
* **subscriptioninitialPositlon**: 提交订阅请求，但是在当前时刻订阅未被创建，服多端会创建该订阅，该参数用于设定新创建的订阅中消费位置的初始值       
* **priorityLevel**: 订阅优先级。在共享订阅模式下分发消息时，服务端会优先给高优为级的消费者发送消息。在拥有最高优先级的消费者可以接收消息的情况下，所有的消息都会被发送到该消费者。 当拥有最高优先级的消费者不能接收消息时，服务端才会考虑下一个优先级消费者。      
* **receiverQueueSize**: 用于设置消费者接收队列的大小，在应用程序调用`Receive`方法之前，消费者会在内存中缓存部分消息。`该参数用于控制队列中最多缓存的消息数。配置高于默认值的值虽然会提高使用者的吞吐量，但会占用更多的内存。`          
* **maxTotalRecelverQueueSizeAcrossPartitlons**: 用于设置多个分区内最大内存队列的长度，与`receiverQueueSize`一同生效。当达到任意一个队列长度限制时，所有接受队列都不能再继续接收数据了  

> 下面是与消息确认相关的参数    

* **acknowledgementsGroupTimeMicros**: 用于设置消费者分批确认的最大允许时间默认情况下，消费者每100 毫秒就会向服务端发送确认请求。将该时间设置为0会
立即发送确认请求。  
* **ackTimeoutMillis**: 未确认消息的超时时间    
* **negativeAckRedeliveryDelayMicros**: 用于设置重新传递消息的延迟时间。客户端在请求重新发送未能处理的消息时，不会立刻发送，而是会有一段时间的延迟。当应用程序使用 negativeAcknowledge方法时，失败的消息会在该时间后重新发送口       
* **tickDurationMillis**: ack-timeout 重新发送请求的时间粒度其他高级特性的配置与使用。  
* **readCompacted**: 在支持压缩的主题中，如果启用`readCompacted`，消费者会从压缩的主题中读取消息，而不是读取主题的完整消息积压。消费者只能看到压缩主题中每个键的最新值。    
* **DeadLetterPolicy**: 用于启动消费者的死信主题。默认情况下，某些消息可能会多次重新发送，甚至可能永远都在重试中。通过使用死信机制，消息具有最大重新发送计数。当超过最大重新发送次数时，消息被发送到死信主题并自动确认。    
* **replicateSubscriptionState**: 如果启用了该参数，则订阅状态将异地复制到集群。    

#### 数据确认   
在Pulsar中，消息一旦被生产者成功写人服务端，该消息会被永久存储，只有在所有订阅都确认后该消息才会被允许删除。因此，当消费者成功消费一条消息时，消费者需要向服务端发送确认请求告知服务端该消息已经被成功消费。（yzhou 这句话博主也没理解，感觉。。。。）    

**独立确认和累计确认**  
在未开启生产者的批发送功能时，每条消息都会被独立发送到服务端，然后又会被独立发送到消费端，接下来我们将讨论这种单条消息的确认方式        

清息可以被一条一条地独立确认，也可以累积后被确认。`采用独立确认时`，消费者需要一认每条消息并向服务端发送确认请求。`采用累积确认时`，消费者只需要确认它收到的最后一条消息就可确认该条消息及之前的消息。累计确认可以应用在非共享订阅模式中，包括独占模式与灾备模式。因为共享模式中涉及多个消费者访问同一订阅，确认一条消息并不能代表该条消息之前的消息已经被成功消费了。在共享模式中，消息都采用独立确认模式。    
`累计确认的使用方式如下`    
```java
Consumer<String> consumer = client.newConsumer(Schema.STRING)
        .topic("my-topic")
        .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
        .subscriptionName("subscription_test")
        .acknowledgmentGroupTime(100, TimeUnit.MICROSECONDS) // 累计确认
        .subscribe();
```
累计确认受到`acknowledgmentGroupTime`参数的控制，该参数作用于累计确认的累计窗口。在每个累计窗口结束后，消费者都会将目前累计的最后一个确认位置发给服务端。  若将acknowledgmentGroupTime 参数的值设为0后，则在任何订阅模式下都会采用独立确认的模式。  

在客户端实现中，消费者通过`acknowledgmentsGroupingTracker`对象对累计确认进行希踪。在独立确认模式下会在方法调用后立即发起服务端请求，而在累积确认模式下将更新量后-次确认的消息ID的值，这样在周期性发送任务时才会真正触发一次确认请求。   

**消息否认确认**    
若消费者已经成功消费一条消息，但在处理该消息时出现了问题，例如下游服务不可用，想要再次消费该消息，此时消费者可以向服务端发送否认确认(`negative
ucknowledgement`)，这时服务端会重新发送消息。         

`消息的否认确认模式也分为逐条否认确认和累积否认确认`，具体采用哪种模式取决于消费订阅模式。在独占和故障转移订阅模式下，消费者只否认收到的最后一条消息。在共享和键共享订阅模式下，用户可以单独否认确认消息。      

>注意，对有序订阅类型(例如独占、故障转移和键共享)的否定确认，可能会导致失败的消息脱离原始顺序到达消费者，从而破坏原有消息的有序性。

在用户否认确认一条消息后，下一步就是如何来重新发送此消息了。    

**确认超时与重试**  
Pulsar对于未能成功消费的消息提供了重试机制。    

首先针对所有成功被客户端消费的消息，在客户端可以配置一个超时参数`ackTimeomt`,在该参数内没被确认的消息会被重新发送。`当该参数为0时，将不会对确认超时的消息进行重新发送`。    

如下代码演示了消息重试方法，其中配置了ackTimeout 参数，超时时间为秒。我们为其中的主题写人一条消息，运行下面的程序后，会发现每5秒都将重新消费一遍生产者写人的消息。      
```java
Consumer<String> consumer = client.newConsumer(Schema.STRING)
        .topic("my-topic")
        .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
        .subscriptionName("subscription_test")
        .ackTimeout(5, TimeUnit.SECONDS)
        .enableRetry(true)
        .subscribe();

for (int i = 0; i < 100; i++) {
    System.out.println("Message received: " + consumer.receive().getValue());
    // 取消消息确认逻辑
    // consumer.acknowledge(msg);
}
``` 

`使用否认确认可以达到消息重试的效果，比起确认状态超时机制，否认确认可以更精确地控制单个消息的重新发送`。使用否认确认可以避免在超时机制中，因数据处理较慢超过越时间值而引起重新发送无效的情况。  
被否认确认的消息会在固定超时时间后重新发送重新发送的周期由`negativeAckRedeliveryDelay`参数控制，默认为1min。    

在对一条消息进行否认确认后，在延迟周期内，会对该消息进行一次确认操作，此时服务端不会再重新发送该消息。与之类似，对该消息进行过一次确认操作后，再进行一次否认确认也不会重新发送该消息。在这种场景下还有另一种解决方式可以重新发送该消息就是利用 Pulsar的死信机制中的重试主题功能。   

对于很多在线业务系统来说，若是在业务逻辑处理中出现异常，并且需要一个精准的延迟发送时间，那么消息需要被重新消费。这时可以使用消费者的自动重试机制(github,com/apache/pulsar/pull/6449)°。当消费者开启自动重试时，如果调用`reconsumeLater`方法请求重新消费该消息，则会在重试主题中存储一条消息，因此消费者会在指定的延迟时间后自动从重试主题中消费该条消息。默认情况下，自动重试处于禁用状态，可以将`enableRetry`参数设置为 true 以启用自动重试。  

下列代码演示了自动重试机制。在配置消费者时需要开启`enableRetry`功能，在未指定死信配置(DeadLetterPolicy)时，客户端会自动以当前的主题名和订阅名生成默认重试主题，规则为`${topic_name-S{subscription name}-RETRY`。在调用`reconsumeLater`方法请求重新写人一条消息时，系统会自动发送确认请求到服务端，然后在配置的重试主题下重新写人一条消息。在超过指定延迟时间后，客户端会重新消费该消息。消息的延迟接收依赖于Pulsar消息延迟传递机制。    
`示例`  
```java
Consumer<String> consumer = client.newConsumer(Schema.STRING)
        .topic("my-topic")
        .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
        .subscriptionName("subscription_test")
        .ackTimeout(5, TimeUnit.SECONDS)
        .enableRetry(true)
        .subscribe();

for (int i = 0; i < 100; i++) {
    Message<String> msg = consumer.receive();
    System.out.println("Message received: " + msg.getValue());
    consumer.reconsumeLater(msg, 10, TimeUnit.SECONDS);
    System.out.println("send retry " + msg.getValue());
}
```

**分批消息确认**    
在开启生产者的批发送后，多条消息会被打包为一条批消息，该消息会作为多条消息的集合被独立存储，在被消费时又作为一个整体被发送到消费端。这时的消息确认会比非分批情况下复杂得多。在 Pulsar 2.6.0之前的版本中对批消息中的任意一条消息进行否认确认时，都会导致该批次的所有消息被重新发送(https://github.corn/apachc/pulsar/issues/5969)。因此，为了避免将确认的消息批量重新发送给消费者，Pulsar从 2.6.0 版本`开始引人了批量索引确认机制`(htps:/github.com/apache/pulsar/wiki/PIP-54:-Support-acknowledgment-at-batch-index-level)。        

对于批消息，如果启用了批量索引确认机制，则服务端会维护批量索引确认状态并跟踪每个批量索引的确认状态，以避免将已确认的消息分发给消费者。当批消息的所有索引都得到确认时。批消息将被删除。默认情况下，服务端和客户端都默认禁用了批量索引确认机制(截至2.10.0版本)。若想开启该功能，需要在代理端设置相关参数，具体见如下代码，然后重启Broker服务。注意，要在服务端使用批量索引功能也要开启该机制，但因为要维护更多的批量索引信息，所以启用批量索引确认后会导致更多的内存开销。    
```
# 修改服务端配置 conf/broker.conf
# 是否开启批消息的确认
acknowledgmentAtBatchIndexLevelEnabled=false
```
    
```java
Consumer<String> consumer = client.newConsumer(Schema.STRING)
        .topic("my-topic")
        .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
        .subscriptionName("subscription_test")
        .enableBatchIndexAcknowledgment(true)
        .subscribe();
```

**Consumer拦截器**  
在Consumer中提供了拦截器功能，该功能可以让用户在消费者中对消息生命周期中的各个节点进行功能增强。例如，对接收后的消息进行转化过滤处理，在消息被确认时调用业务逻辑，在消息确认超时时进行信息统计要想实现消费者拦截器，需要先实现 Consumerlnterceptor 接口。该接口中有以下几方法 ：    

* **beforeConsume**: 在消息到达 receive方法前进行拦截   
* **onAcknowledge**: 在消费者向服务端发送调用请求前被调用   
* **onAcknowledgeCumulative**: 在消费者向服务端发送累计确认请求前被调用     
* **onAckTimeoutSend**: 当消息确认超时后，向服务端发送“重新发送请求”前被调用    
* **onNegativeAcksSend**: 在消费者向服务端周期性发送否认确认请求前被调用    

`示例`  
```java
public class MessageLengthInterceptor implements ConsumerInterceptor {
    private AtomicInteger tooLongMsgCount = new AtomicInteger(0);

    @Override
    public void close() {

    }

    @Override
    public Message beforeConsume(Consumer consumer, Message message) {
        MessageImpl<String> messageImpl = ((MessageImpl<String>) message);
        if (messageImpl.getValue().length() > 20) {
            try {
                consumer.acknowledge(message);
                tooLongMsgCount.incrementAndGet();
            } catch (PulsarClientException e) {
                System.out.println("beforeConsume ack failed");
            }
            return null;
        }
        return message;
    }

    @Override
    public void onAcknowledge(Consumer consumer, MessageId messageId, Throwable exception) {

    }

    @Override
    public void onAcknowledgeCumulative(Consumer consumer, MessageId messageId, Throwable exception) {

    }

    @Override
    public void onAckTimeoutSend(Consumer consumer, Set set) {

    }

    @Override
    public void onNegativeAcksSend(Consumer consumer, Set set) {

    }
}

Consumer<String> consumer = client.newConsumer(Schema.STRING)
        .topic("my-topic")
        .subscriptionInitialPosition(SubscriptionInitialPosition.Earliest)
        .subscriptionName("subscription_test")
        .intercept(new MessageLengthInterceptor())
        .subscribe();
```


**消费监听器**  
Pulsar的Consumer提供了两种类型的监听器 ———— `ConsumerEventListener`和`MessageListener`。    
ConsumerEventListener是用来监听消费者状态变动的监听器，可在故障转移模式下发当分区分配策略变化时监所状态的变动。 `becameActive`在前消费者获取到一个分区的消费权利时被调用。 `becamelnactive`在当前消费者没有分区消费权利时被调用。下面的代码酒示了如何使用消费者事件监听器。   
```java
public class StatusConsumerEventListener implements ConsumerEventListener {
    @Override
    public void becameActive(Consumer<?> consumer, int partitionId) {
        System.out.println(String.format("consumer name=%s,subscription=%s,topic=%s, partitionId=%s active",
                consumer.getConsumerName(),
                consumer.getSubscription(), consumer.getTopic(), partitionId));
    }

    @Override
    public void becameInactive(Consumer<?> consumer, int partitionId) {
        System.out.println(String.format("consumer name=%s,subscription=%s,topic=%s, partitionId=%s inactive",
                consumer.getConsumerName(),
                consumer.getSubscription(), consumer.getTopic(), partitionId));
    }
}
``` 

MessageListener 是一种区别于receive方法的监听器。 

>在使用消息监听器时，receive方法不再提供服务，若此时调用 receive方法会收到客户端异常`Cannot use receive() when a listener has been set”(设置监听器后不能再使用receive())，此时每条发向当前消费者的消且都会调用 MessageListener.received方法 

`具体演示代码如下:`   
```java
public class MyMessageListener implements MessageListener<String> {
    @Override
    public void received(Consumer<String> consumer, Message<String> msg) {
        System.out.println(consumer.getConsumerName() + " MyMessageListener receive msg" + msg.getValue());
    }

    @Override
    public void reachedEndOfTopic(Consumer<String> consumer) {
        MessageListener.super.reachedEndOfTopic(consumer);
        System.out.println("reachedEndOfTopic");
    }
}
```


refer   
1.https://pulsar.apache.org/docs/2.11.x/client-libraries-java/#consumer 
2.《Apache Pulsar原理解析与应用实践》   
