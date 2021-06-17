--In Blog
--Tags: Kafka

# Kafka Broker的Controller选举

>涉及Kafka是2.2.1版本

# KafkaController启动
/* start kafka controller */
kafkaController = new KafkaController(config, zkClient, time, metrics, brokerInfo, brokerEpoch, tokenManager, threadNamePrefix)
kafkaController.startup()



1. 谁来做Leader
2. Leader挂掉了，怎么被Follower感知到








val brokerInfo = createBrokerInfo
val brokerEpoch = zkClient.registerBroker(brokerInfo)


def registerBroker(brokerInfo: BrokerInfo): Long = {
    val path = brokerInfo.path
    val stat = checkedEphemeralCreate(path, brokerInfo.toJsonBytes)
    info(s"Registered broker ${brokerInfo.broker.id} at path $path with addresses: ${brokerInfo.broker.endPoints}, czxid (broker epoch): ${stat.getCzxid}")
    stat.getCzxid
}


1. 先获取broker的zk path路径
2. 注册broker id    
2.1 zookeeper.create() ->  CreateMode.EPHEMERAL 创建server.properties => "zookeeper.connect" 配置参数
2.2 设置 broker信息 注册到 "/brokers/ids/[brokerId]" 值为 {"listener_security_protocol_map":{"PLAINTEXT":"PLAINTEXT"},"endpoints":["PLAINTEXT://127.0.0.1:9092"],"jmx_port":9999,"host":"127.0.0.1","timestamp":"1623430046477","port":9092,"version":4}



yzhou path: /brokers/ids/0
yzhou dataStr: {"listener_security_protocol_map":{"PLAINTEXT":"PLAINTEXT"},"endpoints":["PLAINTEXT://127.0.0.1:9092"],"jmx_port":9999,"host":"127.0.0.1","timestamp":"1623430046477","port":9092,"version":4}


```java
val response = retryRequestUntilConnected(
MultiRequest(Seq(
    CreateOp(path, null, defaultAcls(path), CreateMode.EPHEMERAL),
    SetDataOp(path, data, 0)))
)
```
3. kafkaController.startup()



4. zkData
在zkData 中定义了zookeeper的path


`KafkaZkClient.getControllerId()`       


5. KafkaZkClient.getControllerEpoch()


## AsyncRequest的抽象
type关键字声明为某个类或特质的成员的类型，类本身可以是抽象的，而特质本来从定义上讲就是抽象的，不过类和特质在Scala中都不叫抽象类型。Scala的抽象类型永远都是某个类或特质的成员，比如 下面`type Response <: AsyncResponse`

>补充： <: 强调的是“必须是” xxx的子类型。它作为通用参数的绑定

```java
sealed trait AsyncRequest {
  /**
   * This type member allows us to define methods that take requests and return responses with the correct types.
   * See ``ZooKeeperClient.handleRequests`` for example.
   */
  type Response <: AsyncResponse
  def path: String
  def ctx: Option[Any]
}
```





# Reference
1. [KIP-39 Pinning controller to broker](https://cwiki.apache.org/confluence/display/KAFKA/KIP-39+Pinning+controller+to+broker)
2. [KIP-143: Controller Health Metrics](https://cwiki.apache.org/confluence/display/KAFKA/KIP-143%3A+Controller+Health+Metrics)
3. [KIP-237: More Controller Health Metrics](https://cwiki.apache.org/confluence/display/KAFKA/KIP-237%3A+More+Controller+Health+Metrics)
4. [KIP-291: Separating controller connections and requests from the data plane](https://cwiki.apache.org/confluence/display/KAFKA/KIP-291%3A+Separating+controller+connections+and+requests+from+the+data+plane)