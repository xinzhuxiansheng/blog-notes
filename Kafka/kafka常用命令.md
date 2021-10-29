--In Blog
--Tags: Kafka,Kafka 2.2.1

# Kafka 常用命令

## kafka-topics.sh

### 创建 topic

```shell
./kafka-topics.sh --create --zookeeper [zkurl] --replication-factor 1 --partitions 1 --topic [topic名称]

#创建topic 可以指定配置:
#示例：
--config cleanup.policy=compact --config retention.ms=500
```

### 删除 topic
```shell
./kafka-topics.sh --delete --zookeeper [zkurl]  --topic [topic名称]
```

### 查看topic详情
```shell
./kafka-topics.sh --zookeeper [zkurl] --topic [topic名称] --describe
```

### 添加partition
```shell
./kafka-topics.sh --zookeeper [zkurl] --alter --topic [topic名称] --partitions [分区数]
```




## kafka-console-consumer.sh

### 新建 console consumer

```shell
./kafka-console-consumer.sh  --zookeeper [zkurl]  --topic [topic名称]
```
### 查看 __consumer_offsets

```shell
./kafka-console-consumer.sh --topic __consumer_offsets --bootstrap-server [bootstrapservers] --formatter "kafka.coordinator.group.GroupMetadataManager\$OffsetsMessageFormatter"  --from-beginning
```

## 如何指定consumer参数消费

```shell
#先创建，编辑 consumer.properties 文件
#添加： client.id=1e357476ee124201a655c7175cb1750e
#执行kafka-console-consumer.sh  添加 --consumer.config参数 并指定consuemr.properties的目录地址
./kafka-console-consumer.sh --bootstrap-server [bootstrapservers]  --zookeeper [zkurl] --topic top --consumer.config  consumer.properties
```

## 设置某个consumerGroupId offset 从某个时间点消费

```shell
./kafka-consumer-groups.sh --bootstrap-server [bootstrapservers] --command-config consumer.properties --group [groupid] --reset-offsets --topic [topic名称] --to-datetime 2020-09-07T00:00:00.000 --execute
```

## kafka-console-producer.sh

### 新建 console producer
```shell
./kafka-console-producer.sh --broker-list [zkurl] --topic [topic名称]
```


## 修改topic的过期时间
```shell
./kafka-configs.sh --zookeeper [zkurl] --alter --entity-name [topic名称] --entity-type topics --add-config retention.ms=1800000
```

## 查看topic 是否具有权限
```shell
./kafka-acls.sh  --authorizer-properties zookeeper.connect=[zkurl]  --list  --topic [topic名称] 
```

## kafka-consumer-groups.sh

###  查看consumer groupid 偏移量

```shell
./kafka-consumer-groups.sh --bootstrap-server [bootstrapservers] --describe --group [groupid]  --command-config consumer.properties
```

## 查看 xxxxxxxxx.log消息文件

```shell
./kafka-run-class.sh kafka.tools.DumpLogSegments --files /data1/00000000001506204448.log --print-data-log
```


## Kafka 压测脚本使用
