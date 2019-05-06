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