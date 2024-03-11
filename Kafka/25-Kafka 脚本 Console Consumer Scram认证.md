## Kafka 脚本 Console Consumer Scram 认证 

### 1.背景    
使用 `kafka-console-consumer.sh` 读取带有 Scram 认证的 kafka 集群。 

### 2.操作步骤    

创建`consumer.properties`,内容如下：    
```shell
security.protocol=SASL_PLAINTEXT
sasl.mechanism=SCRAM-SHA-512
sasl.jaas.config=org.apache.kafka.common.security.scram.ScramLoginModule required username="admin" password="123456";
```

运行脚本    
```shell
./kafka-console-consumer.sh --bootstrap-server x.x.x.x:9092 --topic test --consumer.config ../config/consumer.properties
```