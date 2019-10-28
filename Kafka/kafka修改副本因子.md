**正文**

<!-- TOC -->autoauto- [修改副本因子](#修改副本因子)auto- [扩展思路 (**重点**)](#扩展思路-重点)autoauto<!-- /TOC -->

>kafka的高可用 很大程度上基于副本选择，所以集群性能(网络，磁盘io，其他 一些都重要的机器指标)来决定一些topic的副本数,kafka的默认副本数是3.
在开发中经常会遇到修改副本(多数情况时增加)

## 修改副本因子
```shell
vim  increase-replication-factor.json

`添加下面内容`
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
### 执行
./kafka-reassign-partitions.sh --zookeeper xxx.xxx.xxx.xxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --execute

`执行结果`：
[root@realtime-kafka-2 bin]# ./kafka-reassign-partitions.sh --zookeeper xx.xx.xxx.xxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --execute
Current partition replica assignment

{"version":1,"partitions":[{"topic":"topic_name","partition":22,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":35,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":12,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":6,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":31,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":0,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":15,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":28,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":8,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":39,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":18,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":2,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":32,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":17,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":30,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":24,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":14,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":1,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":21,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":36,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":5,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":10,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":37,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":16,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":38,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":19,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":3,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":9,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":25,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":13,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":29,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":26,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":20,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":27,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":11,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":4,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":33,"replicas":[0],"log_dirs":["any"]},{"topic":"topic_name","partition":23,"replicas":[2],"log_dirs":["any"]},{"topic":"topic_name","partition":34,"replicas":[1],"log_dirs":["any"]},{"topic":"topic_name","partition":7,"replicas":[1],"log_dirs":["any"]}]}

Save this to use as the --reassignment-json-file option during rollback
Successfully started reassignment of partitions.





### 验证是否执行成功
./kafka-reassign-partitions.sh --zookeeper xxx.xxx.xxx.xxx:2181/testdocker --reassignment-json-file increase-replication-factor.json --verify

`执行结果`：
[root@realtime-kafka-2 bin]# ./kafka-reassign-partitions.sh --zookeeper 10.27.72.160:2181/testdocker --reassignment-json-file increase-replication-factor.json --verify
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

## 扩展思路 (**重点**)
任何分区的leader也是副本，那添加副本的时候，是否要进行Leader 重新选举，当然这个选举是通过动手设置副本的顺序进行，那么就拿上面的操作举例：
```shell
vim  increase-replication-factor.json
#将topic_name的0分区的 Replicas设置为[1,0,2]
{"topic":"topic_name","partition":0,"replicas":[1,0,2]},
```
`含义`： partition的leader选举会根据副本的列表数序，也就是 1 会被选择为Leader，假设 过去0分区的Leader是0，那么当命令执行后的操作流程
1. 1，2会同步0的副本的数据
2. 将1，2 选入到In Sync Replicas中，因为Leader的选举必须是从ISR中选举出
3. 当1在ISR中后，会自动将 1选为 0分区的 Leader

>所以，这个过程，我们只需要稍微多想想，就能在添加副本的同时，进行Leader切换，做到集群资源的合理分配