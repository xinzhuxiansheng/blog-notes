**正文**

[TOC]

# v1 doc

## 流程
`mm2.properties`

clusters = hyvm01, hyvm02
hyvm01.bootstrap.servers = vm01.com:9082
hyvm02.bootstrap.servers = vm02.com:9082

hyvm01->hyvm02.enabled = true
hyvm01->hyvm02.topics = .*
hyvm01->hyvm02.groups = .*
hyvm01->hyvm02.emit.checkpoints.interval.seconds = 10
hyvm01->hyvm02.emit.heartbeats = true
hyvm01->hyvm02.emit.checkpoints = true
hyvm01->hyvm02.refresh.topics = true
hyvm01->hyvm02.refresh.topics.interval.seconds = 10
hyvm01->hyvm02.refresh.groups = true
hyvm01->hyvm02.refresh.groups.interval.seconds = 10



1. 启动 mirror maker v2 进程
2. 创建相关topic
hyvm01 cluster 创建 `heartbeats ,  mm2-configs.hyvm02.internal , mm2-offset-syncs.hyvm02.internal , mm2-offsets.hyvm02.internal , mm2-status.hyvm02.internal `

hyvm02 cluster 创建 `heartbeats , mm2-configs.hyvm01.internal , mm2-offsets.hyvm01.internal , mm2-status.hyvm01.internal , hyvm01.checkpoints.internal , hyvm01.heartbeats`


3. hyvm02 cluster 会根据 hyvm01 cluster topic名称 自动创建 hyvm01.TopicName
eg: yzhoutp01 -> hyvm01.yzhoutp01

>参考文档
https://github.com/apache/kafka/tree/2.4/connect/mirror
https://blog.cloudera.com/a-look-inside-kafka-mirrormaker-2/
https://cloud.tencent.com/developer/article/1530716
https://cwiki.apache.org/confluence/display/KAFKA/KIP-382%3A+MirrorMaker+2.0
https://github.com/apache/kafka/pull/6295



# v2 doc

## 参考配置文件:
```java
clusters = hyvm01, hyvm02
hyvm01.bootstrap.servers = vm01.com:9082
hyvm02.bootstrap.servers = vm02.com:9082

hyvm01->hyvm02.enabled = true
hyvm01->hyvm02.topics = yzhoutp01
hyvm01->hyvm02.groups = .*
hyvm01->hyvm02.emit.checkpoints.interval.seconds = 10
hyvm01->hyvm02.emit.heartbeats = true
hyvm01->hyvm02.emit.checkpoints = true
hyvm01->hyvm02.refresh.topics = true
hyvm01->hyvm02.refresh.topics.interval.seconds = 10
hyvm01->hyvm02.refresh.groups = true
hyvm01->hyvm02.refresh.groups.interval.seconds = 10
hyvm01->hyvm02.sync.topic.acls.enabled = false
```

`根据配置项的名称，其实就能知道每个配置的含义`

## --cluster 启动方式



## 性能优化





## internal.topic 信息

### mm2-status.hyvm01.internal:

{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":2}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":5}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":4}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":5}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":4}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":2}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":5}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":4}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":2}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":3}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":5}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":7}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":9}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":1}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":4}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"RUNNING","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}
{"state":"UNASSIGNED","trace":null,"worker_id":"hyvm01->hyvm02","generation":6}





### mm2-offsets.hyvm01.internal:

{"offset":38622}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":0}
{"offset":133977}
{"offset":135109}
{"offset":135168}
{"offset":135229}
{"offset":135293}
{"offset":135354}
{"offset":135414}
{"offset":135474}
{"offset":135534}
{"offset":135540}
{"offset":137629}
{"offset":137694}
{"offset":137756}
{"offset":137818}
{"offset":137831}
{"offset":137894}
{"offset":137954}
{"offset":138014}
{"offset":138074}
{"offset":138134}
{"offset":138194}
{"offset":138254}
{"offset":138314}
{"offset":138374}
{"offset":138438}
{"offset":138499}
{"offset":138559}
{"offset":138683}
{"offset":138744}
{"offset":138804}
{"offset":138864}
{"offset":138923}
{"offset":138984}
{"offset":139044}
{"offset":139104}
{"offset":139164}
{"offset":139224}
{"offset":139284}
{"offset":139344}
{"offset":139404}
{"offset":139464}
{"offset":139524}
{"offset":139584}
{"offset":139644}
{"offset":139704}
{"offset":139764}
{"offset":139824}
{"offset":139998}
{"offset":140061}
{"offset":140122}
{"offset":140183}
{"offset":140243}
{"offset":140368}
{"offset":140429}
{"offset":140489}
{"offset":140552}
{"offset":141014}
{"offset":141074}
{"offset":141134}
{"offset":141194}
{"offset":141254}
{"offset":141305}
{"offset":145445}
{"offset":145477}
{"offset":145540}
{"offset":145600}
{"offset":145660}
{"offset":145720}
{"offset":145780}
{"offset":145840}
{"offset":145900}
{"offset":145960}
{"offset":146020}
{"offset":146080}
{"offset":146140}
{"offset":146200}
{"offset":146260}
{"offset":146320}
{"offset":146328}
{"offset":146508}


### mm2-configs.hyvm01.internal:

{"key":"PhGPpMby3qsIYgSJVXtRfZrlTdaCRC62P1xB1hditHg=","algorithm":"HmacSHA256","creation-timestamp":1578291262644}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorSourceConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorSourceConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorHeartbeatConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorCheckpointConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorCheckpointConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","task.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatTask","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorHeartbeatConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorCheckpointConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","task.assigned.groups":"KMOffsetCache-AT8406-DLP","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","task.class":"org.apache.kafka.connect.mirror.MirrorCheckpointTask","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorCheckpointConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"tasks":1}
{"tasks":1}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorSourceConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","task.class":"org.apache.kafka.connect.mirror.MirrorSourceTask","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorSourceConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","task.assigned.partitions":"yzhoutp01-0,heartbeats-0","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"tasks":1}
{"key":"LIcZSOfBySLzuQlK5j9oU8UX3uEDmkBE2WrfsvDODWo=","algorithm":"HmacSHA256","creation-timestamp":1578292387371}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorSourceConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorSourceConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorHeartbeatConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorCheckpointConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorCheckpointConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"key":"h3DY3hK2DgoB02VKdHBJgFaWQs4ZhIlIVeJq6PzTod4=","algorithm":"HmacSHA256","creation-timestamp":1578294910696}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorSourceConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorSourceConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorHeartbeatConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorCheckpointConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorCheckpointConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"key":"hLS23wgmqm4lCYaLVheN85VAUscPc1ZZbL4eDdZv0lM=","algorithm":"HmacSHA256","creation-timestamp":1578302735599}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorSourceConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorSourceConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorHeartbeatConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorCheckpointConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorCheckpointConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"key":"pT0z1FKP2L3R9UBSRXctPz6EoUufzvMKG2rjv/l2Vzs=","algorithm":"HmacSHA256","creation-timestamp":1578303838710}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorSourceConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorSourceConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorHeartbeatConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorHeartbeatConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}
{"properties":{"connector.class":"org.apache.kafka.connect.mirror.MirrorCheckpointConnector","emit.heartbeats.enabled":"false","source.cluster.producer.bootstrap.servers":"vm01.com:9082","sync.topic.acls.enabled":"false","refresh.topics.enabled":"false","topics":"yzhoutp01","source.cluster.alias":"hyvm01","refresh.groups.enabled":"false","groups":".*","source.cluster.bootstrap.servers":"vm01.com:9082","target.cluster.producer.bootstrap.servers":"vm02.com:9082","enabled":"true","target.cluster.admin.bootstrap.servers":"vm02.com:9082","target.cluster.alias":"hyvm02","target.cluster.consumer.bootstrap.servers":"vm02.com:9082","name":"MirrorCheckpointConnector","target.cluster.bootstrap.servers":"vm02.com:9082","sync.topic.configs.enabled":"false","source.cluster.admin.bootstrap.servers":"vm01.com:9082","source.cluster.consumer.bootstrap.servers":"vm01.com:9082"}}

