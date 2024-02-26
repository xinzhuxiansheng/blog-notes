

### 只处理 value
```
CREATE TABLE TABLE_NAME (
    `id` STRING COMMENT '',
    ... 
    `col87` STRING COMMENT '',
    `col88` BYTES COMMENT ''
  )
WITH (
	'connector' = 'kafka',
	'topic' = 'topicName',
	'scan.startup.mode' = 'earliest-offset',
	'properties.group.id' = 'gid',
	'properties.security.protocol' = 'SASL_PLAINTEXT',
	'properties.sasl.mechanism' = 'SCRAM-SHA-256',
	'properties.bootstrap.servers' = 'xxx.xxx.xxx.xxx:9092',
	'properties.sasl.jaas.config' = 'org.apache.kafka.common.security.scram.ScramLoginModule required username="xxxx" password="xxxx";',
	'value.format' = 'avro-confluent',
	'value.avro-confluent.url' = 'http://xxx.xxx.xxx.xxx:8091'
);
```