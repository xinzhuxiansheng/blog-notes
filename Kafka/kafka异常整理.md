**正文**

[TOC]



1. Caused by: org.apache.kafka.common.errors.TimeoutException: Expiring 18 record(s) for nginx_test-18: 30053 ms has passed since batch creation plus linger time



2. Caused by: java.lang.Exception: Failed to send data to Kafka: The server disconnected before a response was received



3. java.lang.Exception: Failed to send data to Kafka: Failed to allocate memory within the configured max blocking time 60000 ms



4. java.lang.Exception: Failed to send data to Kafka: The server experienced an unexpected error when processing the 
   


5. server.log.2019-10-09-19:[2019-10-09 19:45:23,366] ERROR [ReplicaFetcherThread-0-4], Error for partition [rcm_no_shangtou,30] to broker 4:org.apache.kafka.common.errors.NotLeaderForPartitionException: This server is not the leader for that topic-partition. (kafka.server.ReplicaFetcherThread)


6. [2019-10-16 23:37:42,436] ERROR [KafkaApi-0] Error while responding to offset request (kafka.server.KafkaApis)
java.lang.NullPointerException



zookeeper


1. caught end of stream exception