**正文**

>max.connections.per.ip=3  设置每个ip 连接broker的时候，最多连接数（针对所有ip）
 max.connections.per.ip.overrides=xxx.xxx.xxx.xx:100 设置某个ip或者多个ip的连接数，这里会覆盖默认配置(max.connections.per.ip=3) ,类似于白名单一样，单独设置

 
 当超过配置的tcp连接数时：

`broker端`
[2019-10-29 23:45:57,482] INFO Rejected connection from /xxx.xxx.xx.xx, address already has the configured maximum of 3 connections. (kafka.network.Acceptor)


`client端`
2019-10-29 23:45:55  [ Selector.java:345 ] - [ DEBUG ]  Connection with vm01.com/xxx.xxx.xx.xx disconnected
java.io.EOFException: null
	at org.apache.kafka.common.network.NetworkReceive.readFromReadableChannel(NetworkReceive.java:83) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.NetworkReceive.readFrom(NetworkReceive.java:71) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.KafkaChannel.receive(KafkaChannel.java:154) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.KafkaChannel.read(KafkaChannel.java:135) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.Selector.pollSelectionKeys(Selector.java:323) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.Selector.poll(Selector.java:283) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.NetworkClient.poll(NetworkClient.java:260) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.internals.ConsumerNetworkClient.clientPoll(ConsumerNetworkClient.java:360) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.internals.ConsumerNetworkClient.poll(ConsumerNetworkClient.java:224) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.internals.ConsumerNetworkClient.poll(ConsumerNetworkClient.java:192) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.internals.ConsumerNetworkClient.poll(ConsumerNetworkClient.java:163) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.internals.AbstractCoordinator.ensureActiveGroup(AbstractCoordinator.java:243) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.internals.ConsumerCoordinator.ensurePartitionAssignment(ConsumerCoordinator.java:345) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.KafkaConsumer.pollOnce(KafkaConsumer.java:977) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.consumer.KafkaConsumer.poll(KafkaConsumer.java:937) [kafka-clients-0.10.0.0.jar:?]
	at com.javamain.kafkaClient.consumer.localpc_consumer_main.main(localpc_consumer_main.java:29) [classes/:?]


 

 java.io.EOFException: null
	at org.apache.kafka.common.network.NetworkReceive.readFromReadableChannel(NetworkReceive.java:83) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.NetworkReceive.readFrom(NetworkReceive.java:71) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.KafkaChannel.receive(KafkaChannel.java:154) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.KafkaChannel.read(KafkaChannel.java:135) ~[kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.Selector.pollSelectionKeys(Selector.java:323) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.common.network.Selector.poll(Selector.java:283) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.NetworkClient.poll(NetworkClient.java:260) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.producer.internals.Sender.run(Sender.java:229) [kafka-clients-0.10.0.0.jar:?]
	at org.apache.kafka.clients.producer.internals.Sender.run(Sender.java:134) [kafka-clients-0.10.0.0.jar:?]
	at java.lang.Thread.run(Thread.java:748) [?:1.8.0_202]
2019-11-02 11:06:21  [ NetworkClient.java:463 ] - [ DEBUG ]  Node -1 disconnected.