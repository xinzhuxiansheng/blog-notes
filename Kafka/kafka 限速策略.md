
相关命令: 
leader.replication.throttled.replicas
follower.replication.throttled.replicas


leader.replication.throttled.rate
follower.replication.throttled.rate


leader.reassignment.throttled.rate
follower.reassignment.throttled.rate


测试集群 214-218集群

### Broker节点限速
shell命令: 
```shell
#broker整体限制
./kafka-configs.sh --zookeeper 10.27.241.214:2181,10.27.241.216:2181,10.27.241.218:2181/virtual-kafka --alter --add-config 'leader.replication.throttled.rate=1048576,follower.replication.throttled.rate=1048576' --entity-type brokers --entity-name 0


./kafka-configs.sh --describe --zookeeper 10.27.241.214:2181,10.27.241.216:2181,10.27.241.218:2181/virtual-kafka --entity-type brokers

kafka-configs.sh --zookeeper 10.27.241.214:2181,10.27.241.216:2181,10.27.241.218:2181/virtual-kafka  --entity-type brokers --entity-name 0 --alter --delete-config follower.replication.throttled.rate,leader.replication.throttled.rate
```

### zk查看config存储路径
```shell
[zk: localhost:2181(CONNECTED) 21] get   /virtual-kafka/config/brokers/0
{"version":1,"config":{"leader.replication.throttled.rate":"104857600","follower.replication.throttled.rate":"104857600"}}
```
![kafka限速策略01](/kafka/images/kafka限速策略01.png)


### broker日志
![kafka限速策略broker01](/kafka/images/kafka限速策略broker01.png)


//*************************************************************************************


### Topic限速
```shell
#topic限速

```




1.需求： 当某个broker 宕机后恢复，限制某个Topic的关于Broker节点的Follower角色Fetch速度
2.需求： Topic 重分区限速



1. 如何能证明参数是生效的？  








>参考
https://www.cnblogs.com/huxi2b/p/8609453.html

https://www.cnblogs.com/lenmom/p/10301428.html