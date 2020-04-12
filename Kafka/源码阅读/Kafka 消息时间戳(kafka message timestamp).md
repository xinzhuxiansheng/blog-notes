
## Kafka消息的时间戳
在消息中增加了一个时间戳字段和时间戳类型。目前支持的时间戳类型有两种： CreateTime 和 LogAppendTime 前者表示producer创建这条消息的时间；后者表示broker接收到这条消息的时间(严格来说，是leader broker将这条消息写入到log的时间)
 
## 为什么要加入时间戳？
`引入时间戳主要解决3个问题：`

    日志保存(log retention)策略：Kafka目前会定期删除过期日志(log.retention.hours，默认是7天)。判断的依据就是比较日志段文件(log segment file)的最新修改时间(last modification time)。倘若最近一次修改发生于7天前，那么就会视该日志段文件为过期日志，执行清除操作。但如果topic的某个分区曾经发生过分区副本的重分配(replica
    reassigment)，那么就有可能会在一个新的broker上创建日志段文件，并把该文件的最新修改时间设置为最新时间，这样设定的清除策略就无法执行了，尽管该日志段中的数据其实已经满足可以被清除的条件了。
    日志切分(log rolling)策略：与日志保存是一样的道理。当前日志段文件会根据规则对当前日志进行切分——即，创建一个新的日志段文件，并设置其为当前激活(active)日志段。其中有一条规则就是基于时间的(log.roll.hours，默认是7天)，即当前日志段文件的最新一次修改发生于7天前的话，就创建一个新的日志段文件，并设置为active日志段。所以，它也有同样的问题，即最近修改时间不是固定的，一旦发生分区副本重分配，该值就会发生变更，导致日志无法执行切分。（注意：log.retention.hours及其家族与log.rolling.hours及其家族不会冲突的，因为Kafka不会清除当前激活日志段文件）
    流式处理(Kafka streaming)：流式处理中需要用到消息的时间戳

## 消息格式的变化
1 增加了timestamp字段，表示时间戳
2 增加了timestamp类型字段，保存在attribute属性低位的第四个比特上，0表示CreateTime；1表示LogAppendTime(低位前三个比特保存消息压缩类型)
 
## 客户端消息格式的变化
ProducerRecord：增加了timestamp字段，允许producer指定消息的时间戳，如果不指定的话使用producer客户端的当前时间
ConsumerRecord：增加了timestamp字段，允许消费消息时获取到消息的时间戳
 
 
ProducerResponse: 增加了timestamp字段，如果是CreateTime返回-1；如果是LogAppendTime，返回写入该条消息时broker的本地时间
 
## 如何使用时间戳？
Kafka broker config提供了一个参数：log.message.timestamp.type来统一指定集群中的所有topic使用哪种时间戳类型。用户也可以为单个topic设置不同的时间戳类型，具体做法是创建topic时覆盖掉全局配置：
```shell
bin/kafka-topics.sh --zookeeper localhost:2181 --create --topic test --partitions 1 --replication-factor 1 --config message.timestamp.type=LogAppendTime
```
另外， producer在创建ProducerRecord时可以指定时间戳: 
```shell
record = new ProducerRecord<String, String>("my-topic", null, System.currentTimeMillis(), "key", "value");
```

## Kafka内部如何处理时间戳？ 
![kafka消息时间戳流程图](images/kafka%20message%20timestamp.png)

 值得一提的是上图中的”指定阈值“ —— 有时候我们需要实现这样的场景：比如某条消息如果在5分钟内还不能被创建出来那么就不再需要创建了，直接丢弃之。Kafka提供了log.message.timestamp.difference.max.ms和message.timestamp.difference.max.ms参数来实现这样的需求，当然只对CreateTime类型的时间戳有效，如果是LogAppendTime则该参数无效。
 
## 基于时间戳的功能
1 根据时间戳来定位消息：之前的索引文件是根据offset信息的，从逻辑语义上并不方便使用，引入了时间戳之后，Kafka支持根据时间戳来查找定位消息
2 基于时间戳的日志切分策略
3 基于时间戳的日志清除策略

 
## 基于时间戳的消息定位
自0.10.0.1开始，Kafka为每个topic分区增加了新的索引文件：基于时间的索引文件：<segment基础位移>.timeindex，索引项间隔由index.interval.bytes确定。
具体的格式是时间戳+位移
时间戳记录的是该日志段当前记录的最大时间戳
位移信息记录的是插入新的索引项时的消息位移信息
该索引文件中的每一行元组(时间戳T，位移offset)表示：该日志段中比T晚的所有消息的位移都比offset大。
 
由于创建了额外的索引文件，所需的操作系统文件句柄平均要增加1/3（原来需要2个文件，现在需要3个），应调整系统文件句柄的参数。


摘自 https://www.maiyewang.com/archives/7812