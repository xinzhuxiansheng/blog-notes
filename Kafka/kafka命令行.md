**`正文`**
[TOC]

## 创建 topic
```shell
bin/kafka-topics.sh --create --zookeeper 127.0.0.1:2181 --replication-factor 1 --partitions 2 --topic topic名称
```

## 新建 consumer
```shell
bin/kafka-console-consumer.sh  --zookeeper 127.0.0.1:2181  --topic topic名称
```

## 新建 producer
```shell
bin/kafka-console-producer.sh --broker-list 127.0.0.1:9092 --topic topic名称
```

## 删除 topic
```shell
bin/kafka-topics.sh --delete --zookeeper 127.0.0.1:2181  --topic topic名称
```

## 查看topic详情
```shell
bin/kafka-topics.sh --zookeeper 127.0.0.1:2181 --topic topic名称 --describe
```

## 添加partition
```shell
./kafka-topics.sh --zookeeper xxx.xxx.xxx.xxx:2181/kafka-local --alter --topic top --partitions 24
```

## 修改topic的过期时间
```shell
./kafka-configs.sh --zookeeper xxx.xxx.xxx.xxx:2181/kafka-local --alter --entity-name topic_name --entity-type topics --add-config retention.ms=1800000
```




## 如何指定consumer参数消费
```shell
#先创建，编辑 consumer.properties 文件
#添加： client.id=1e357476ee124201a655c7175cb1750e
#执行kafka-console-consumer.sh  添加 --consumer.config参数 并指定consuemr.properties的目录地址
./kafka-console-consumer.sh --bootstrap-server xxx.xxx.xxx.xxx:9092  --zookeeper xxx.xxx.xxx.xxx:2181/kafka-local  --topic top --consumer.config  consumer.properties
```


earliest




