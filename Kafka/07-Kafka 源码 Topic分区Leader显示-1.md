--In Blog
--Tags: Kafka

# Kafka Topic分区Leader为-1 问题处理

>涉及Kafka是2.2.1版本

`关键词`
**1.** AR：分区中的所有副本统称为AR（Assigned Replicas）
**2.** ISR: 所有与Leader副本保持一定程度(replica.lag.time.max.ms)同步的副本(包括Leader副本在内)集合
**3.** replica.lag.time.max.ms： If a follower hasn't sent any fetch requests or hasn't consumed up to the leaders log end offset for at least this time, the leader will remove the follower from isr  默认值：10000

## 1.Topic分区的Leader = -1
Topic的某个分区只有1个副本在ISR，并且出现唯一的副本所在的Broker服务出现故障或者机器宕机，就会很容易出现该分区的Leader为-1；
例如：Topic：test01 分区只有1个，副本为2个
```shell
./kafka-topics.sh --zookeeper [zkurl] --topic [topic名称] --describe
```
![Test01 desc](http://118.126.116.71/blogimgs/kafka/Topic%E5%88%86%E5%8C%BALeader%E4%B8%BA-1/Topic%E5%88%86%E5%8C%BA%E7%AD%89%E4%BA%8E-101.png)
目前Leader：3， 若Isr：3,2 都出现Broker异常或者机器宕机，Leader：-1；

## 2.IDEA + KafkaManager(CMAK)复现
`环境准备`
Kafka: 3个Brokers，BrokerIds分别是：1,2,3   
test01 Partition 0 ,Isr:(3,2), Leader: 3;

### 2.1 Broker2脱离ISR
test01 Partition0分区 Isr的2是Follower，它会同步3的数据，所以开启Broker2的远程调试端口，我们在IDEA中Kafka的源码 `ReplicaFetcherThread.java的processPartitionData()方法`打上断点

`复现1`
1.关闭Broker2
2.再关闭Broker3

`另一种复现2 准备工作，虽然麻烦，对于开发来说，学一门远程调试技能`
**1.** 搭建本地调试Kafka源码环境，请参考[Kafka源码环境搭建](http://xinzhuxiansheng.com/articleDetail?id=4)
**2.** 开启远程调试端口，请参考[Kafka开启远程调试](http://xinzhuxiansheng.com/articleDetail?id=10)
**3.** 在IDEA中Kafka的源码 `ReplicaFetcherThread.java的processPartitionData()方法`打上断点
![processPartitionData方法断点](http://118.126.116.71/blogimgs/kafka/Topic%E5%88%86%E5%8C%BALeader%E4%B8%BA-1/Topic%E5%88%86%E5%8C%BA%E7%AD%89%E4%BA%8E-102.png)
**4.** 在IDEA 创建Producer发送数据(1s发一条即可)，因为2在有数据同步时候会执行 processPartitionData()
```java
Long i = 0L;
while (true) {
    producer.send(new ProducerRecord<String, String>("test01", i.toString()), new Callback() {
        @Override
        public void onCompletion(RecordMetadata recordMetadata, Exception e) {
            if (null == recordMetadata) {
                e.printStackTrace();
            }
        }
    });
    Thread.currentThread().sleep(1000L);
    System.out.println(i);
    i++;
}
```
**5.** 手动 Kill掉Broker3，之后 再去掉IDEA断点，继续执行
![processPartitionData方法断点](http://118.126.116.71/blogimgs/kafka/Topic%E5%88%86%E5%8C%BALeader%E4%B8%BA-1/Topic%E5%88%86%E5%8C%BA%E7%AD%89%E4%BA%8E-104.png)


## 3.Leader=-1 对Kafka Topic写入和消费有影响
请及时处理Leader为 -1的Topic，会直接影响业务方的写入和消费，及时做好Kafka Topic的ISR副本个数监控，<=1需要排查，
`排查思路`
**1.** Kafka broker日志检查，zk
**2.** 宿主机 CPU,Load,磁盘IO,网卡及网卡驱动(一定要好好评估网卡及网卡驱动是否有问题，我就栽这里好几回，物理机是双网卡，至少5次，因为网卡问题，流量上不去，导致部分副本脱离ISR)


## 4.如何处理或者恢复
`第一种`
**1.** 一般是因为机器宕机或者机器有问题，先启动Broker或者重启Broker，然后准备备机拷贝安装包，设置同样的BrokerId，速度恢复有问题节点。迅速恢复OfflinePartitions

`第二种 利用unclean.leader.election.enable 从非ISR副本集合，并且副本所在的Broker正常，选出新的Leader`
**2.** unclean.leader.election.enable参数使用
集群默认配置 unclean.leader.election.enable=false
此参数属于 TopicConfig，并且支持动态配置， 若出现Leader=-1 的Topic，请设置该Topic的unclean.leader.election.enable参数为true。支持非ISR的副本被全局为Leader。

> 此参数会丢失Topic数据，请谨慎使用
```java
./kafka-configs.sh --zookeeper [zkurl] --alter --entity-name [topic名称] --entity-type topics --add-config unclean.leader.election.enable=true
```
设置Topic的unclean.leader.election.enable 从而解决 分区Leader=-1 的情况

`第三种 直接修改zookeeper中该分区对应的state`
**3.** 据百度搜索结果 直接修改zookeeper中该分区对应的state（本操作，本人没有处理过也没看过代码，所以不清楚是否有效）

