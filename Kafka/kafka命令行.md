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

## 修改副本因子 
```shell
vim  increase-replication-factor.json

#添加下面内容
{"version":1,
"partitions":[
{"topic":"topic_name","partition":0,"replicas":[0,1,2]},
{"topic":"topic_name","partition":1,"replicas":[0,1,2]},
{"topic":"topic_name","partition":2,"replicas":[0,1,2]},
{"topic":"topic_name","partition":3,"replicas":[0,1,2]},
{"topic":"topic_name","partition":4,"replicas":[0,1,2]},
{"topic":"topic_name","partition":5,"replicas":[0,1,2]},
{"topic":"topic_name","partition":6,"replicas":[0,1,2]},
{"topic":"topic_name","partition":7,"replicas":[0,1,2]},
{"topic":"topic_name","partition":8,"replicas":[0,1,2]},
{"topic":"topic_name","partition":9,"replicas":[0,1,2]},
{"topic":"topic_name","partition":10,"replicas":[0,1,2]},
{"topic":"topic_name","partition":11,"replicas":[0,1,2]},
{"topic":"topic_name","partition":12,"replicas":[0,1,2]},
{"topic":"topic_name","partition":13,"replicas":[0,1,2]},
{"topic":"topic_name","partition":14,"replicas":[0,1,2]},
{"topic":"topic_name","partition":15,"replicas":[0,1,2]},
{"topic":"topic_name","partition":16,"replicas":[0,1,2]},
{"topic":"topic_name","partition":17,"replicas":[0,1,2]},
{"topic":"topic_name","partition":18,"replicas":[0,1,2]},
{"topic":"topic_name","partition":19,"replicas":[0,1,2]},
{"topic":"topic_name","partition":20,"replicas":[0,1,2]},
{"topic":"topic_name","partition":21,"replicas":[0,1,2]},
{"topic":"topic_name","partition":22,"replicas":[0,1,2]},
{"topic":"topic_name","partition":23,"replicas":[0,1,2]},
{"topic":"topic_name","partition":24,"replicas":[0,1,2]},
{"topic":"topic_name","partition":25,"replicas":[0,1,2]},
{"topic":"topic_name","partition":26,"replicas":[0,1,2]},
{"topic":"topic_name","partition":27,"replicas":[0,1,2]},
{"topic":"topic_name","partition":28,"replicas":[0,1,2]},
{"topic":"topic_name","partition":29,"replicas":[0,1,2]},
{"topic":"topic_name","partition":30,"replicas":[0,1,2]},
{"topic":"topic_name","partition":31,"replicas":[0,1,2]},
{"topic":"topic_name","partition":32,"replicas":[0,1,2]},
{"topic":"topic_name","partition":33,"replicas":[0,1,2]},
{"topic":"topic_name","partition":34,"replicas":[0,1,2]},
{"topic":"topic_name","partition":35,"replicas":[0,1,2]},
{"topic":"topic_name","partition":36,"replicas":[0,1,2]},
{"topic":"topic_name","partition":37,"replicas":[0,1,2]},
{"topic":"topic_name","partition":38,"replicas":[0,1,2]},
{"topic":"topic_name","partition":39,"replicas":[0,1,2]}
]
}
# 执行
./kafka-reassign-partitions.sh --zookeeper xxx.xxx.xxx.xxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --execute

#执行结果：
[root@realtime-kafka-2 bin]# ./kafka-reassign-partitions.sh --zookeeper xx.xxx.xx.xxxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --execute
Current partition replica assignment

{"version":1,"partitions":[{"topic":"topic_name","partition":22,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":35,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":12,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":6,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":31,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":0,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":15,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":28,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":8,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":39,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":18,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":2,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":32,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":17,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":30,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":24,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":14,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":1,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":21,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":36,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":5,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":10,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":37,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":16,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":38,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":19,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":3,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":9,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":25,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":13,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":29,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":26,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":20,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":27,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":11,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":4,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":33,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":23,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":34,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":7,"replicas":[1],"log_dirs":["any"]}]}

Save this to use as the --reassignment-json-file option during rollback
Successfully started reassignment of partitions.





# 验证是否执行成功
./kafka-reassign-partitions.sh --zookeeper xxx.xxx.xxx.xxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --verify

#执行结果：
[root@realtime-kafka-2 bin]# ./kafka-reassign-partitions.sh --zookeeper xxx.xxx.xxx.xxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --verify
Status of partition reassignment: 
Reassignment of partition topic_name-22 is still in progress
Reassignment of partition topic_name-35 is still in progress
Reassignment of partition topic_name-12 is still in progress
Reassignment of partition topic_name-6 is still in progress
Reassignment of partition topic_name-31 is still in progress
Reassignment of partition topic_name-0 is still in progress
Reassignment of partition topic_name-15 is still in progress
Reassignment of partition topic_name-28 is still in progress
Reassignment of partition topic_name-8 is still in progress
Reassignment of partition topic_name-39 is still in progress
Reassignment of partition topic_name-18 is still in progress
Reassignment of partition topic_name-2 is still in progress
Reassignment of partition topic_name-32 is still in progress
Reassignment of partition topic_name-17 is still in progress
Reassignment of partition topic_name-30 is still in progress
Reassignment of partition topic_name-24 is still in progress
Reassignment of partition topic_name-14 is still in progress
Reassignment of partition topic_name-1 is still in progress
Reassignment of partition topic_name-21 is still in progress
Reassignment of partition topic_name-36 is still in progress
Reassignment of partition topic_name-5 is still in progress
Reassignment of partition topic_name-10 is still in progress
Reassignment of partition topic_name-37 is still in progress
Reassignment of partition topic_name-16 is still in progress
Reassignment of partition topic_name-38 is still in progress
Reassignment of partition topic_name-19 is still in progress
Reassignment of partition topic_name-3 is still in progress
Reassignment of partition topic_name-9 is still in progress
Reassignment of partition topic_name-25 is still in progress
Reassignment of partition topic_name-13 is still in progress
Reassignment of partition topic_name-29 is still in progress
Reassignment of partition topic_name-26 is still in progress
Reassignment of partition topic_name-20 is still in progress
Reassignment of partition topic_name-27 is still in progress
Reassignment of partition topic_name-11 is still in progress
Reassignment of partition topic_name-4 is still in progress
Reassignment of partition topic_name-33 is still in progress
Reassignment of partition topic_name-23 is still in progress
Reassignment of partition topic_name-34 completed successfully
Reassignment of partition topic_name-7 is still in progress


```




earliest




