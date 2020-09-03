**`正文`**
[TOC]

## 创建 topic
```shell
bin/kafka-topics.sh --create --zookeeper 127.0.0.1:2181 --replication-factor 1 --partitions 2 --topic topic名称

创建topic 可以指定配置:
示例：--config cleanup.policy=compact --config retention.ms=500
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


## 查看 __consumer_offsets
```shell
./kafka-console-consumer.sh --topic __consumer_offsets --bootstrap-server localhost:9092 --formatter "kafka.coordinator.group.GroupMetadataManager\$OffsetsMessageFormatter"  --from-beginning
```



## 如何指定consumer参数消费
```shell
#先创建，编辑 consumer.properties 文件
#添加： client.id=1e357476ee124201a655c7175cb1750e
#执行kafka-console-consumer.sh  添加 --consumer.config参数 并指定consuemr.properties的目录地址
./kafka-console-consumer.sh --bootstrap-server xxx.xxx.xxx.xxx:9092  --zookeeper xxx.xxx.xxx.xxx:2181/kafka-local  --topic top --consumer.config  consumer.properties
```

## 查看topic 是否具有权限
```shell
./kafka-acls.sh  --authorizer-properties zookeeper.connect=xx.xx.240.196:2181,xx.xx.240.197:2181,xx.xx.240.199:2181/kafka-sjz  --list  --topic 'test_create_form_api_01'  --username kafka --password xxxx@Kafka2016
```

## 查看consumer groupid 偏移量
```shell
./kafka-consumer-groups.sh --bootstrap-server xx.xx.4.167:9092 --describe --group yzhou2_app_sight_show_rcm_lf_202003300941  --command-config consumer.properties
```

## 查看 xxxxxxxxx.log消息文件
```shell
./kafka-run-class.sh kafka.tools.DumpLogSegments --files /data1/00000000001506204448.log --print-data-log
```


earliest




