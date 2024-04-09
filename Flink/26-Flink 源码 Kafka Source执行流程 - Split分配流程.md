# Flink 源码 Kafka Source执行流程 - Split分配流程       

## Split 分配流程       
`Split 分配流程`是由`KafkaSourceEnumerator` 实现, 它是`SplitEnumerator`接口的实现类, 运行在JobMaster中;              

整个Split的分配流程如下图所示:          







refer   
1.https://github.com/apache/flink-connector-kafka       
