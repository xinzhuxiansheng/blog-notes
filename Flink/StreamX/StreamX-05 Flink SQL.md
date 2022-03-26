## StreamX跑通Flink SQL作业

### 建表语句

表需要提前创建
```

```



### SQL语句
```shell
CREATE TABLE user_log (
    user_id Bigint,
    item_id Bigint,
    catgory_id Bigint,
    behavior varchar,
    ts varchar
  ) WITH (
    'connector' = 'kafka',
    'properties.group.id' = 'testGroup_1',
    'properties.enable.auto.commit' = 'false',
    'topic' = 'yzhoutp01',
    'properties.bootstrap.servers' = '192.168.xxx.xxx:9092',
    'properties.security.protocol' = 'SASL_PLAINTEXT',
    'properties.sasl.mechanism' = 'SCRAM-SHA-256',
    'properties.sasl.jaas.config' = 'org.apache.kafka.common.security.scram.ScramLoginModule required username='admin' password='adminpwd';',
    'scan.startup.mode' = 'earliest-offset',
    'format' = 'json'
  );

  CREATE TABLE user_log_mysql (
    user_id Bigint,
    item_id Bigint,
    catgory_id Bigint,
    behavior varchar,
    ts varchar
    ) WITH (
    'connector' = 'jdbc',
    'url' = 'jdbc:mysql://10.168.xxx.xxx:3306/yzhouTest',
    'table-name' = 'user_log',
    'username' = 'yzhou',
    'password' = '12345678'
  );

  INSERT INTO user_log_mysql select * from user_log;
```

### pom导入 

```xml
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-kafka_2.11</artifactId>
    <version>1.14.2</version>
</dependency>

<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-jdbc_2.11</artifactId>
    <version>1.14.2</version>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-json</artifactId>
    <version>1.14.2</version>
</dependency>
<dependency>
    <groupId>mysql</groupId>
    <artifactId>mysql-connector-java</artifactId>
    <version>5.1.48</version>
</dependency>


```