```
WITH
  (
    'properties.sasl.mechanism' = 'SCRAM-SHA-256',
    'value.avro-confluent.subject' = 'xxx-value',
    'properties.security.protocol' = 'SASL_PLAINTEXT',
    'scan.startup.mode' = 'earliest-offset',
    'key.fields' = 'id',
    'properties.bootstrap.servers' = '192.168.xxx.xxx:9092',
    'properties.sasl.jaas.config' = 'org.apache.kafka.common.security.scram.ScramLoginModule required username="admin" password="xxx";',
    'connector' = 'kafka',
    'key.avro-confluent.url' = 'http://192.168.xxx.xxx:8081',
    'value.format' = 'avro-confluent',
    'key.format' = 'avro-confluent',
    'value.fields-include' = 'EXCEPT_KEY',
    'topic' = 'xxxx',
    'properties.group.id' = 'testGroup',
    'value.avro-confluent.url' = 'http://192.168.xxx.xxx:8081'
  )
```
flink kafka scan =.startup.mode的几个选项

    group-offsets: start from committed offsets in ZK / Kafka brokers of a specific consumer group.
    earliest-offset: start from the earliest offset possible.
    latest-offset: start from the latest offset.
    timestamp: start from user-supplied timestamp for each partition.
    specific-offsets: start from user-supplied specific offsets for each partition.

        默认选项是group-offset，表示从ZK/kafka代理中最后提交的偏移量中消费
        如果指定了时间戳，则需要另一个配置选项’scan.startup.timestamp-millis’ =
        '1648817042000’来指定特定时间戳
        如果指定了特定偏移量，则需要另一个配置选项scan.start .
        specific-offset为每个分区指定特定的启动偏移量，例如一个选项值分区：0，偏移量：42；分区：1，偏移：300表示分区0的偏移42，分区1的偏移300



1.'scan.startup.mode' 
* earliest-offset 
* latest-offset   
