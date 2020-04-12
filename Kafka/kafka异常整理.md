**正文**

[TOC]


## broker

1. Caused by: java.lang.Exception: Failed to send data to Kafka: The server disconnected before a response was received



2. java.lang.Exception: Failed to send data to Kafka: Failed to allocate memory within the configured max blocking time 60000 ms



3. java.lang.Exception: Failed to send data to Kafka: The server experienced an unexpected error when processing the 
   


4. server.log.2019-10-09-19:[2019-10-09 19:45:23,366] ERROR [ReplicaFetcherThread-0-4], Error for partition [rcm_no_shangtou,30] to broker 4:org.apache.kafka.common.errors.NotLeaderForPartitionException: This server is not the leader for that topic-partition. (kafka.server.ReplicaFetcherThread)


5. [2019-10-16 23:37:42,436] ERROR [KafkaApi-0] Error while responding to offset request (kafka.server.KafkaApis)
java.lang.NullPointerException



7. org.apache.kafka.common.errors.TimeoutException: Failed to update metadata after 60000 ms.
   检查连接参数是否正确


8. java.lang.Exception: Failed to send data to Kafka: Failed to allocate memory within the configured max blocking time 60000 ms.
  at org.apache.flink.streaming.connectors.kafka.FlinkKafkaProducerBase.checkErroneous(FlinkKafkaProducerBase.java:375)
  at org.apache.flink.streaming.connectors.kafka.FlinkKafkaProducer010.invoke(FlinkKafkaProducer010.java:352)
  at org.apache.flink.streaming.api.operators.StreamSink.processElement(StreamSink.java:56)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.pushToOperator(OperatorChain.java:579)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:554)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:534)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:718)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:696)
  at org.apache.flink.streaming.api.operators.TimestampedCollector.collect(TimestampedCollector.java:51)
  at com.xinzhuxiansheng.uas.job.AppLogJob$1.processElement(AppLogJob.java:74)
  at com.xinzhuxiansheng.uas.job.AppLogJob$1.processElement(AppLogJob.java:71)
  at org.apache.flink.streaming.api.operators.ProcessOperator.processElement(ProcessOperator.java:66)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.pushToOperator(OperatorChain.java:579)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:554)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:534)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:718)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:696)
  at org.apache.flink.streaming.api.operators.StreamFilter.processElement(StreamFilter.java:40)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.pushToOperator(OperatorChain.java:579)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:554)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:534)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:718)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:696)
  at org.apache.flink.streaming.api.operators.TimestampedCollector.collect(TimestampedCollector.java:51)
  at com.xinzhuxiansheng.uas.entity.impl.AppLogEntity$Flater.flatMap(AppLogEntity.java:87)
  at com.xinzhuxiansheng.uas.entity.impl.AppLogEntity$Flater.flatMap(AppLogEntity.java:61)
  at org.apache.flink.streaming.api.operators.StreamFlatMap.processElement(StreamFlatMap.java:50)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.pushToOperator(OperatorChain.java:579)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:554)
  at org.apache.flink.streaming.runtime.tasks.OperatorChain$CopyingChainingOutput.collect(OperatorChain.java:534)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:718)
  at org.apache.flink.streaming.api.operators.AbstractStreamOperator$CountingOutput.collect(AbstractStreamOperator.java:696)
  at org.apache.flink.streaming.api.operators.StreamMap.processElement(StreamMap.java:41)


9. [2020-04-11 22:16:39,746] ERROR [ReplicaFetcherThread-0-0], Error for partition [mis_attention_700031_online,1] to broker 0:org.apache.kafka.common.errors.UnknownServerException: The server experienced an unexpected error when processing the request (kafka.server.ReplicaFetcherThread)


## 客户端
1. Caused by: org.apache.kafka.common.errors.TimeoutException: Expiring 18 record(s) for nginx_test-18: 30053 ms has passed since batch creation plus linger time

2. java.lang.Exception: Failed to send data to Kafka: This server is not the leader for that topic-partition.


3. Group coordinator vm01.com:9092 (id: 2147483647 rack: null) is unavailable or invalid, will attempt rediscovery





zookeeper


1. caught end of stream exception






[2019-11-01 16:10:05,687] ERROR [ReplicaFetcherThread-0-2], Error for partition [uas_rcm,42] to broker 2:org.apache.kafka.common.errors.NotLeaderForPartitionException: This server is not the leader for that topic-partition. (kafka.server.ReplicaFetcherThread)

这个错已经明确 是从RelicaFetcherThread线程 报出来的
这里涉及到副本同步机制实现

kafka.server的 ReplicaManager.scala



