**正文**

[TOC]

## kafka mirror maker
2台机器 vm01,vm02

### 操作步骤

`vm01` 

zk:vm01.com:2181/vmkafka010
topic:yzhoutest01,yzhoutest02

`创建 consumer.properties:`
bootstrap.servers=vm01.com:9093
zookeeper.connect=vm01.com:2181/vmkafka010
group.id=groupIdMirror
client.id=sourceMirror
auto.offset.reset=largest
heartbeat.interval.ms=30000
session.timeout.ms=100000
partition.assignment.strategy=roundrobin
max.poll.records=20000
receive.buffer.bytes=4194304
max.partition.fetch.bytes=10485760



`创建 producer.properties:`
bootstrap.servers=vm02.com:9092
client.id=sinkMirror
buffer.memory = 268435456 
batch.size = 104857
acks=0
linger.ms=10
max.request.size = 10485760
send.buffer.bytes = 10485760 
compression.type=snappy



vm02

zk:vm01.com:2181/vm02_kafka010
topic:yzhoutest01


./kafka-mirror-maker.sh --consumer.config consumer.properties --producer.config producer.properties --whitelist 'yzhoutest01'




## 参数
[root@localhost bin]# ./kafka-mirror-maker.sh -help
Option                                 Description                           
------                                 -----------                           
--abort.on.send.failure <Stop the      Configure the mirror maker to exit on 
  entire mirror maker when a send        a failed send. (default: true)      
  failure occurs>                                                            
--blacklist <Java regex (String)>      Blacklist of topics to mirror. Only   
                                         old consumer supports blacklist.    
--consumer.config <config file>        Embedded consumer config for consuming
                                         from the source cluster.            
--consumer.rebalance.listener <A       The consumer rebalance listener to use
  custom rebalance listener of type      for mirror maker consumer.          
  ConsumerRebalanceListener>                                                 
--help                                 Print this message.                   
--message.handler <A custom message    Message handler which will process    
  handler of type                        every record in-between consumer and
  MirrorMakerMessageHandler>             producer.                           
--message.handler.args <Arguments      Arguments used by custom message      
  passed to message handler              handler for mirror maker.           
  constructor.>                                                              
--new.consumer                         Use new consumer in mirror maker.     
--num.streams <Integer: Number of      Number of consumption streams.        
  threads>                               (default: 1)                        
--offset.commit.interval.ms <Integer:  Offset commit interval in ms (default:
  offset commit interval in              60000)                              
  millisecond>                                                               
--producer.config <config file>        Embedded producer config.             
--rebalance.listener.args <Arguments   Arguments used by custom rebalance    
  passed to custom rebalance listener    listener for mirror maker consumer  
  constructor as a string.>                                                  
--whitelist <Java regex (String)>      Whitelist of topics to mirror.