```
CREATE TABLE TABLE_NAME (
    `id` STRING COMMENT '',
    ... 
    `col87` STRING COMMENT '',
    `col88` BYTES COMMENT ''
  )
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