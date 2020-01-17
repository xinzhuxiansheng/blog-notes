
# kafka mirror maker 2.0

## 简介

github 2.4 分支以上(包含2.4分支)版本

请参考：  `仔细学习KIP-382文档，信息量巨大`
https://cwiki.apache.org/confluence/display/KAFKA/KIP-382%3A+MirrorMaker+2.0#KIP-382:MirrorMaker2.0-Config,ACLSync

## 部署
特别介绍2种部署方式:
1. Running a dedicated MirrorMaker cluster
2. Running MirrorMaker in a Connect cluster


### Running a dedicated MirrorMaker cluster

1. 自行下载released包 /  github源码打包
2. 了解并熟悉 mm2.properties 配置项内容
参考如下配置项： `都是挺重要的参数，请务必有`

```java
#集群alias
clusters = hyvm01, hyvm02
#集群连接地址
hyvm01.bootstrap.servers = vm01.com:9082
hyvm02.bootstrap.servers = vm02.com:9082
#集群是否开启同步，同步topic ，groups 
hyvm01->hyvm02.enabled = true
hyvm01->hyvm02.topics = yzhoutp01
hyvm01->hyvm02.groups = .*

hyvm02->hyvm01.enabled = true
hyvm02->hyvm01.topics = yzhoutp01
hyvm02->hyvm01.groups = .*

#集群同步的 emit，refresh的间隔时间 
hyvm01->hyvm02.emit.checkpoints.interval.seconds = 10
hyvm01->hyvm02.refresh.groups = true
hyvm01->hyvm02.refresh.groups.interval.seconds = 10

hyvm02->hyvm01.emit.checkpoints.interval.seconds = 10
hyvm02->hyvm01.refresh.groups = true
hyvm02->hyvm01.refresh.groups.interval.seconds = 10
```


3. 运行
```shell
config/mm2.properties --clusters hyvm01 hyvm02
```


### Running MirrorMaker in a Connect cluster
后续补充