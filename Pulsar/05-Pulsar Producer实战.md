##  Pulsar Producer实战  

>Pulsar version: 2.11   

### 引言    
基于上一篇《Pulsar快速体验》blog中介绍standalone模式部署Pulsar，使用`pulsar-client`来模拟Producer，而本篇主要讲解通过Java Client写入消息。  


### 环境搭建    
博主选择在`Pulsar源码`项目中新增加一个模块`pulsar-yzhou`，用来编写Client API，这样方便调试及修改源码。如图所示： 
![producer01](http://img.xinzhuxiansheng.com/blogimgs/pulsar/producer01.png)        

**添加依赖**    
```
<!-- in your <properties> block -->
<pulsar.version>2.11.1</pulsar.version>

<!-- in your <dependencies> block -->
<dependency>
  <groupId>org.apache.pulsar</groupId>
  <artifactId>pulsar-client</artifactId>
  <version>${pulsar.version}</version>
</dependency>
```

### 构建Client
```java
public static void main(String[] args) throws PulsarClientException {
    PulsarClient client = PulsarClient.builder()
            .serviceUrl("pulsar://localhost:6650")
            .build();
```

如果你有多个节点，你可以在`serviceUrl`参数中配置多个broker地址。    
```java
PulsarClient client = PulsarClient.builder()
        .serviceUrl("pulsar://localhost:6650,localhost:6651,localhost:6652")
        .build();
```

>如果你的pulsar是standalone模式启动，默认情况下代理地址是`pulsar://localhost:6650`。    

在Pulsar官网中介绍，如果你想创建一个client，你可以使用loadConf()来配置参数，例如下面形式：  

![producer02](http://img.xinzhuxiansheng.com/blogimgs/pulsar/producer02.png)    

`loadConf`中有以下参数:     
* **serviceUrl**:必填参数。配置 Pulsar 服务访问的链接       
* **numioThreads**:处理服务端连接的线程个数。默认为1    
* **numListenerThreads**:主要用于消费者，处理消息监听和拉取的线程个数。默认为1
* **statsintervalSeconds**: 通过日志来打印客户端统计信息的时间间隔。默认为 60秒   
* **connectionsPerBroker**: 在客户端处理 Broker 请求时，每个 Broker 对应建立多少个连接。默认为1。   
* **memoryLimitBytes**:客户端中的内存限制参数。默认为0。Pulsar客户端底层的通信基于 Netty 构建，下面的这些参数是网络通信相关的参数               
* **operationTimeoutMs**:网络通信超时时间。默认为30000 毫秒     
* **keepAlivelntervalSeconds**: 每个客户端与服务端连接保持活动的间隔时间。在客户端底层会按照该参数周期性确认连接是否存活。默认为 30 秒  
* **connectionTimeoutMs**:与服务端建立连接时的最大等待时间。默认为 10000毫秒口requestTimeoutMs;完成一次请求的最大超时时间。默认为 60000 毫秒    
* **concurrentLookupRequest**: 允许在每个 Broker 连接上并行发送的 Lookup 请求的数量，用来防止代理过载。Lookup 请求用于查找管理某个主题的具体 Broker 地址
默认为5000个请求。      
* **maxLookupRequest**:一个 Broker 上允许的最大并发的 Lookup 请求数量。该参数与 concurrentLookupRequcst 共同生效。默认为 50000 个请求    
* **maxNumberOfRejectedRequestPerConnection**:当前连接关闭后，客户端在一定时间范围内(30秒)拒绝的最大请求数，超过该请求数后客户端会关闭旧的连接并创建新连接来连接不同的 Broker。默认为50个连接   
* **useTcpNoDelay**:这是一个网络通信底层参数，用于决定是否在连接上禁用 Nagle算法。默认为 True   


### 构建生产者  
#### 示例              
```java
    public static void main(String[] args) throws PulsarClientException {
        PulsarClient client = PulsarClient.builder()
                .serviceUrl("pulsar://192.168.175.129:6650")
                .build();

        // 构建生产者
        Producer<byte[]> producer = client.newProducer()
                .topic("persist_topic_1")
                .create();

        // 同步发送消息
        producer.send("发送同步消息".getBytes());
        // 异步发送消息
        producer.sendAsync("发送异步消息".getBytes());
        // 关闭生产者与客户端
        producer.close();
        client.close();
    }
```     
生产者构造器中传入topicName参数作为要写入的主题，在不指定命名空间时，默认写入的主题是`persistent://public/default/persist_topic_1`,其中`public`是默认租户，`default`是租户下的默认持久化命名空间。该主题在写入时如果还未被创建，则会根据服务端配置来决定是否创建该主体或创建什么样主题？ 以下列举服务端配置参数：       
* **allowAutoTopicCreation**: 决定当生产者或者消费者要连接主题时，是否自动创建该主题，默认是True。      
* **allowAutoTopicCreationType**: 自动创建主题时，创建分区主题或者非分区主题。 

>注意：非分区主题只能由单个broker提供服务。             

#### 发送数据    
生产者发送消息的方式有两种一一`同步发送`和`异步发送`，这与Kafka的Client有这异曲同工之处。不过有一点需特别注意区别： 
`根据Pulsar的文档介绍 producer是线程安全的，但是在Kafka Client的Producer并不是线程安全的。`     
![producer03](http://img.xinzhuxiansheng.com/blogimgs/pulsar/producer03.png)    

在使用同步方式发送消息时客户端程序会阻塞线程，并等待写人任务完成或者抛出异常。使用异步方式发送消息时,生产者将消息放人阻塞队列并立即返回，然后客户端将消息经后台发送到 Broker 中，这时如果客户端在发送过程中出现异常，则需要用户编写异步的异常处理逻辑。 
```java
producer.sendAsync("发送同步消息".getBytes())
        .thenAccept(msgId -> {
                System.out.println("发送成功");
        }).exceptionally(throwable -> {
                System.out.println("消息发送失败");
                return null;
        });
```     
同步发送与异步发送的区别在于对 Future 对象的不同处理上，同步发送是在异步发送逻辑的基础上，使用future.get() 等待异步执行结果完成。例如，等待队列已满时，生产者会被阻塞或立即失败，具体取决于传递给生产者的参数`maxPendingMessages`和`blockIfQueueFull`。         

若producer发送完数据后，可以使用Rest API来验证数据是否写入成功以及成功写入的消息数量。 以下是Rest API示例：     
```shell 
# 查看Topic信息
curl http://localhost:8080/admin/v2/persistent/public/default/
```
**output：**    
```
["persistent://public/default/persist_topic_1"]
```

```shell
# 使用/internalStats api查看消息写入数量
curl http://localhost:8080/admin/v2/persistent/public/default/persist_topic_1/internalStats
```
**output：**            
`numberOfEntries` 表示的是写入成功的消息数量。  
```
{
    "entriesAddedCounter": 1,
    "numberOfEntries": 1,
    "totalSize": 56,
    "currentLedgerEntries": 1,
    "currentLedgerSize": 56,
    "lastLedgerCreatedTimestamp": "2023-06-05T08:27:43.105+08:00",
    "waitingCursorsCount": 0,
    "pendingAddEntriesCount": 0,
    "lastConfirmedEntry": "28:0",
    "state": "LedgerOpened",
    "ledgers":
    [
        {
            "ledgerId": 28,
            "entries": 0,
            "size": 0,
            "offloaded": false,
            "underReplicated": false
        }
    ],
    "cursors":
    {},
    "schemaLedgers":
    [],
    "compactedLedger":
    {
        "ledgerId": -1,
        "entries": -1,
        "size": -1,
        "offloaded": false,
        "underReplicated": false
    }
}
```             

Pulsar 发送消息时，可以为每条消息指定键值。此键值在消息路由和主题压缩等场景中会被用到。在发送消息到分区主题时，该键值会作为路由的参考。在压缩主题中，Pulsar 可以为同一键值构建消息视图，从而自动从主题中清洗掉较老的数据，让用户可以更快地找到主题中的近期数据，相关的实现代码如下      
```java
Producer<KeyValue<String, String>> producer = client.newProducer(Schema.KeyValue(Schema.STRING, Schema.STRING))
        .topic("persist_topic_1")
        .create();
```

在构建Producer时，还可以配置以下参数：  
* **producerName**: 为生产者命名，若不指定名称则会自动创建一个唯一的名称,可通过Rest API `admin/v2/persistent/public/default/persist_topic_1/stats`接口可以查看当前的生产者列表。  
* **sendTimeoutMs**: 写入消息的超时时间，如果在该时间范围内未收到服务端的确认回复则抛出异常。默认是30000毫秒    
* **maxPendingMessages**: 保存一个生产者待处理消息的队列，该队列中存放的是尚未被服务端确认写入的消息数量，默认为1000，当该队列存满之后将通过`blockIfQueueFull`参数决定如何处理。        
* **maxPendingMessagesAcrossPartitions**: 在同一个主题下，不同分区内的多个生产者同用的最大待处理消息的总数量。与`maxPendingMessages`参数共同作用，最后生效的`maxPendingMessages`的值不得大于`maxPendingMessagesAcrossPartitions`除以分区数所得到的值。  
* **messageRoutingMode**: 针对分区主题的消息路由规则，决定一条消息实际被发到哪个分区内。        
* **hashingScheme**: 针对分区主题的消息路由规则，指定一条消息的哈希函数 
* **cryptoFailureAction**: 当加密失败时，生产者应该采取的发送行动。该参数的值FAIL 表示如果加密失败，则未加密的消息无法发送;该参数的值为 SEND 表示如加密失数，则发送未加密的消息。默认为 FAIL。  
* **batchingEnabled**: 是否允许批处理发送。
* **batchingMaxPublishDelayMicros**: 发送批处理消息的最大延迟时间。  
* **batchingMaxMessages**: 发送批处理消息的最大数量 
* **compressionType**: 消息压缩方式。        


#### 数据发送路由规则   
默认情况下，Pulsar 创建的主题是非分区主题，`这种非分区主题只能由单Broker提供服务`，该 Broker将独自处理来自不同生产者的所有消息，这极大地限制了主题的最大吞吐量。        

为了提高吞吐量，可以手动创建分区主题。分区主题逻辑上虽是一个主题，但实际上相当于多个非分区主题的组合。
```shell
# shell 创建3个分区的分区主题 
./pulsar-admin topics create-partitioned-topic persistent://public/default/partitioned-topic-test -p 3

# Rest API 创建3个分区的分区主题        
curl -X PUT http://localhost:8080/admin/v2/persistent/public/default/partitioned-topic-test/partitions -d '3'
```

例如拥有3个分区的分区主题`persistent://public/default/partitioned-topic-test`，通过接口查看可以发现实际创建出了3个分区，具体如下:       
```shell
curl http://localhost:8080/admin/v2/persistent/public/default
```
**output**      
["persistent://public/default/partitioned-topic-test-partition-0","persistent://public/default/partitioned-topic-test-partition-1","persistent://public/default/partitioned-topic-test-partition-2"]

分区主题可以跨越多个Broker节点，从而达到更高的吞吐量。与非分区主题类似，分区主题也可以使用Pulsar客户端发送到对应的服务端，消息要发送到分区主题时，必须指定路由模式以决定如何发送及具体发送到哪个分区。如果在创建新的生产者时没有指定任何路由模式，则使用循环路由模式(`RoundRobinDistribution`)        

在Pulsar中，每条消息都可以设置一个键值，键值是消息的一项关键属性。生产者在发送数据到服务端时，针对消息有没有键值会采取有不同的处理策略。例如你可以用有明确业务含义的字段作为键，比如用户ID，这样就可以保证拥有同一个用户ID 的消息进人同一个分区中。     

**轮询路由模式**        
轮询路由模式是一种跨越所有分区发送消息的模式，这是生产者针对分区主题默认采用的路由模式。根据用户是否指定消息的键值，有两种不同的路由策略        

`1` 如果没有提供消息的键值，生产者会按照轮询策略跨所有分区发布消息，以达到最大吞吐量。轮询不是针对单个消息进行的，轮询的边界应设置为与批处理延迟相同以确保批处理有效        
`2` 如果在消息上指定了一个键值，则分区的生产者会对该键值进行散列并将消息分配给特定的分区。针对求哈希值的方式，生产者提供了`hashingScheme`配置项来让用户选择不同的函数。 Pulsar 提供了以下3 种求哈希值的方式。   
**pulsar.JavaStringHash**: 使用 Java自带的java.lang.String#hashCode方法求哈希值         
**pulsar.Murmur3_32Hash**: 使用 Murmur3 中的哈希函数。MurmurHash是一种经过广泛测试且速度很快的非加密哈希函数，在 Murmur3中有该算法的实现                
**pulsarBoostHash**: 使用C++ Boost库中的散列函数。      

`示例`  
```java
Producer<byte[]> producer = client.newProducer()
    .topic("my-topic")
    .messageRoutingMode(MessageRoutingMode.RoundRobinPartition)
    .hashingScheme(HashingScheme.JavaStringHash) // 使用 Java 的 String hashCode
    .create();
```

**单分区模式**  
单分区模式(`UseSinglePartition`) 会将当前生产者的所有消息发布到单个分区。如果未提供消息的键值，则生产者会随机选择一个分区并将所有消息发布到该分区。如果在消息上绑定了一个健值，则分区的生产者会对该键值进行哈希运算并将消息分配给特定的分区。   

>使用单分区模式，每个生产者发送的消息写入Broker节点时，会保证逻辑上的循序性。   

这里有个另外的话题是`Producer在发送消息时也可以指定特定某个分区`，具体的方法是在创建 Producer时使用带索引的分区名称。示例：      
```java
Producer<byte[]> producer = client.newProducer()
    .topic("my-topic-partition-0") // 这里的 "0" 表示分区索引
    .create();
```

**自定义分区模式**      
自定义分区模式(`CustomParition`) 可以通过调用自定义的消息路由器来确定消息发送的分区，可以通过使用Java 客户端来实 MessageRouler接口进而创建自定义路由模式。      

`示例`     
实现自定义路由接口，其中满足以下场景：  
* 消息的字节长度小于10的消息发送到第一分区    
* 长度在10到100之间的消息发送到第二分区         
* 长度大于100的消息发送到第三分区  
* 若Topic的分区数不等于3时，会随机选择分区并发送消息      
```java
// 实现MessageRouter接口
public class MessageLengthRouter implements MessageRouter {
    static Random random = new Random();

    @Override
    public int choosePartition(Message<?> msg, TopicMetadata metadata) {
        int partitionNums = metadata.numPartitions();
        if (partitionNums != 3) {
            return random.nextInt(partitionNums);
        }
        if (msg.getData().length < 10) {
            return 0;
        } else if (msg.getData().length >= 10 && msg.getData().length < 100) {
            return 1;
        } else {
            return 2;
        }
    }
}

// 指定自定义路由
Producer<byte[]> producer = client.newProducer()
        .topic("my-topic")
        .messageRouter(new MessageLengthRouter())
        .create();
```     

**分批发送**   
在大数据技术中批处理通常用于提高吞吐量。在生产者和消费者中支持批量处理，并显著提开效率。目前，生产者客户端提供了批量发送消息的能力，通过启用`batchingEnabled`参数，就可以实现批量写入。 在批量写入时会不可避免地造成比单条消息是高的延迟，用户可以使用`batchingMaxPublishDelayMicros`(这批消息的第一条数据最大延迟时间)和`batchingMaxMessages`(这批消息的最大数量)参数来控制延迟情况。          
```java
Producer<byte[]> producer = client.newProducer()
        .topic("my-topic")
        .batchingMaxPublishDelay(10, TimeUnit.MILLISECONDS)
        .sendTimeout(10, TimeUnit.SECONDS)
        .enableBatching(true)
        .create();
```     
在Pulsar 生产者对消息进行批发送时，服务端会将一整批消息作为一个最小单元进行确认和存储。在消费者要使用消息时，会再将批消息拆分为单独的消息。     

**分块发送**   
除了分批发送外，Pulsar还提供了一种发送模式————`分块发送`，不同于分批发送，在分批发送模式下，Pulsar会将多条单独消息组合成一条消息来发送。分块消息是将一条超大的消息分成多个数据块并发送到服务端。当启用分块发送模式时，如果消息大小超过允许发送的最大限值(由`conf/brokerconf`中的`maxMessageSize`参数控制),则生产者会将原始消息拆分为多块。并将它们与分块元数据分别按顾序发布到代理。    

在服务端,分块消息的存储和普通消息的存储类似。消费者在使用该分块消息时，由消费者客户端提供统一的视图，这看起来和消费一条普通的未分块消息一样。在客户端的实现上。需要在本地内存中缓冲分块消息，并在所有分块消息都收集完后将它们组合成真实的消息。 

`示例`  
```java
Producer<byte[]> producer = client.newProducer()
    .topic("my-topic")
    .maxMessageSize(1024 * 1024) // 设置最大消息大小为 1MB
    .chunkingEnabled(true) // 启用分块
    .enableBatching(false)
    .create();
```

>要启用分块，您需要同时禁用批处理 (`enableBatching=false`)。    


### 拦截器              
在Pulsar生产者中还提供了一个拦截器功能，该功能可以让用户在发送消息前对消息进行定制化处理，例如修改和过滤。`该功能还可以在消息被服务端确认写人后提供一个回调函数，这在某些应用场景中可以方便追踪每条消息写人情况`。若想实现生产者拦截器需要用户实现`Producerinterceptor`接口。     
`示例`          
实现一个简单的大写字符转换拦截器                
```java
Producer<byte[]> producer = client.newProducer()
        .topic("my-topic")
        .intercept(new PulsarInterceptUpper())
        .create();
```


refer
1.https://pulsar.apache.org/docs/2.11.x/client-api-overview/    
2.https://pulsar.apache.org/docs/2.11.x/client-libraries-java/  
3.《Apache Pulsar原理解析与应用实践》
4.https://pulsar.apache.org/docs/2.11.x/client-libraries-java/#intercept-messages
