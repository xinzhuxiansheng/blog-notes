**正文**


[TOC]

### 同步失效分区
处于同步失效或功能失效(比如处于非活跃状态)的副本统称为失效副本，而包含失效副本的分区也就是为同步失效分区。      
通常情况下，在一个运行状态良好的kafka集群中，失效分区的个数应该为0。kafka本身提供了一个相关的指标来表征失效分区的个数，即UnderReplicatedPartotions,可以通过JMX访问来获取其值：
```shell
kafka.server:type=ReplicaManager,name=UnderReplicatePartitions
```
取值范围是大于等于0的整数。如果获取的UnderReplicatedPartitions值大于0，那么就需要对其进行告警，并进一步诊断其背后的真正的原因，有可能是某个broker的问题，也有可能引申到整个集群的问题，也许还要引入其他一些信息、指标等配合找出问题之所在。注意：如果kafka集群正在做分区重分配,这个值也会大于0。        
1. 如果集群中有多个broker的UnderReplicatedPartitions保持一个大于0的稳定性，则一般暗示集群中有broker处于下线状态。在这种情况下，这个broker中的分区个数与集群中所有UnderReplicatedPartitions(处于下线的broker是不会上报任何指标值得)之和是相等的。通常这类问题是由于机器硬件原因引起的，但也有可能是由于操作系统或JVM引起的，可以往这个方向继续做进一步的深入调查。      

2. 如果集群中存在broker的UnderReplicatedPartitions频繁变动，或者处于一个稳定的大于0的值(这里特指没有broker下线的情况)时，一般暗示集群出现了性能问题，通常这类问题很难诊断，不过我们可以一步将问题的范围缩小，比如先尝试确定这个性能问题是否只存在于集群的某个broker中，还是整个集群之上。如果确定集群中所有的under-replicated分区都在单个broker上，那么可以看出这个broker出现了问题。      进而可以针对这个单一的broker做专项调查，比如操作系统，GC，网络状态或磁盘状态(如iowait，ioutil等指标)。

3. 如果多个broker中都出现了under-replicated分区，则一般是整个集群的问题，但也有可能是单个broker出现问题，前者可以理解，后者又怎么解释呢？ 想象这样一种情况，如果某个broker在消息同步方面出了问题，那么其上的follower副本就无法及时有效地与其他broker上的leader副本进行同步， 这样一来就出现了多个broker都存在under-replicated分区的想象。有一种方向可以查看是否单个broker的问题(以及哪个broker出现了问题)，我们通过kafka-topic.sh脚本可以查看集群中所有的under-replicated分区。

`案例说明：`        
假设集群中有4个broker，编号为[0,1,2,3],相关的under-replicated分区信息如下：
```shell
bin/kafka-topics.sh --describe --zookeeper localhost:2181/kafka --under-replicated
Topic: topic- 1  Partition: 7 Leader: 0 Replicas: 0 , 1  Isr: 0 
Topic: topic- 1  Partition: 1 Leader: 2 Replicas: 1 , 2  Isr: 2 
Topic: topic- 2  Partition: 3 Leader: 3 Replicas: 1 , 3  Isr: 3 
Topic: topic- 2  Partition: 4 Leader: 0 Replicas: 0 , 1  Isr: 0 
Topic: topic- 3  Partition: 7 Leader: 0 Replicas: 0 , 1  Isr: 0 
Topic: topic- 3  Partition: 5 Leader: 3 Replicas: 1 , 3  Isr: 3 
Topic: topic- 4  Partition: 6 Leader: 2 Replicas: 1 , 2  Isr: 2 
TopiC: topic- 4  Partition: 2 Leader: 2 Replicas: 1 , 2  Isr: 2 
```

在这个案例中，我们可以看到所有的ISR集合中都出现编号为1的broker确实，进而可以将调查的重心迁移到这个broker上，如果通过上面的步骤没有定位到某个独立的broker，那么就需要针对整个集群层面做进一步的探究。
集群层面的问题一般也就是两个方面：资源瓶颈和负载不均衡。资源瓶颈指的是broker在某硬件资源的使用上遇到瓶颈，比如网络,cpu,i/o等层面，就以i/o来论，kafka中的消息都是存盘的，生产者线程将消息写入leader副本的性能和i/0有着直接的关联，follower副本的同步线程及消费者的消费线程又要通过i/o从磁盘中拉取消息，如果i/o层面出现瓶颈，那么势必影响全局的走向，与此同时消息的流入/流出又都需要和网络打交道。笔者建议硬件层面的指标可以关注CPU的使用率，网络流入/流入速度，磁盘的读/写速度，iowait，ioutil等，也可以适当的关注下文件句柄数，socket句柄数及内存等方面。



