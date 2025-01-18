# RocketMQ Dledger 高可用集群搭建      
>RocketMQ Version: 4.9.5  

本次搭建，以RocketMQ 默认配置dledger集群，将broker服务分别部署在不同服务器上。              
集群规划如下：                  
| hostname      |    nameserver | broker  |
| :-------- | --------:| :--: |
| vm01  | NameServer | broker-n0  |
| vm02  | NameServer | broker-n1  |
| vm03  | NameServer | broker-n2  |

>注意：若搭建测试环境，先判断是否要调整 jvm 参数 

## 调整 jvm 参数 

### 调整 NameServer jvm 参数 
修改 `bin/runserver.sh`，调整里面的jvm内存配置。找到下面这一行调整下内存        
```bash
JAVA_OPT="${JAVA_OPT} -server -Xms512m -Xmx512m -Xmn256m -XX:MetaspaceSize=128m -XX:MaxMetaspaceSize=320m"
```

### 调整 Broker jvm 参数 
修改`bin/runbroker.sh`的JVM参数，修改为512m:        
```bash
JAVA_OPT="${JAVA_OPT} -server -Xms512m -Xmx512m"
```


## 设置 logs (所有节点)
```shell
sed -i 's|\${user.home}|/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release|g' *.xml
```

## 启动 NameServer 

```bash
nohup ./mqnamesrv &
nohup bin/mqnamesrv -c /root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/conf/namesrv.properties > /dev/null 2>&1 &
```

>`> /dev/null 2>&1` 表示不追加 log 

关闭        
```bash
sh mqshutdown namesrv
```

## 启动 Broker 

### 配置讲解  

### broker01 

vim /conf/dledger/broker-n0.conf  
```bash
brokerClusterName= RaftCluster
brokerName=RaftNode00
listenPort=30911
namesrvAddr=vm01:9876;vm02:9876;vm03:9876
storePathRootDir=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/
storePathCommitLog=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/commitlog
storePathConsumeQueue=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/consumequeue
storePathIndex=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/index
storeCheckpoint=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/checkpoint
abortFile=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/abort
enableDLegerCommitLog=true
dLegerGroup=RaftNode00
dLegerPeers=n0-vm01:40911;n1-vm02:40911;n2-vm03:40911

# 值必须唯一
dLegerSelfId=n0
sendMessageThreadPoolNums=16
```   


### broker02

vim /conf/dledger/broker-n1.conf  
```bash
brokerClusterName= RaftCluster
brokerName=RaftNode01
listenPort=30911
namesrvAddr=vm01:9876;vm02:9876;vm03:9876
storePathRootDir=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/
storePathCommitLog=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/commitlog
storePathConsumeQueue=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/consumequeue
storePathIndex=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/index
storeCheckpoint=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/checkpoint
abortFile=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/abort
enableDLegerCommitLog=true
dLegerGroup=RaftNode01
dLegerPeers=n0-vm01:40911;n1-vm02:40911;n2-vm03:40911

# 值必须唯一
dLegerSelfId=n1
sendMessageThreadPoolNums=16
```   


### broker03

vim /conf/dledger/broker-n1.conf  
```bash
brokerClusterName= RaftCluster
brokerName=RaftNode02
listenPort=30911
namesrvAddr=vm01:9876;vm02:9876;vm03:9876
storePathRootDir=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/
storePathCommitLog=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/commitlog
storePathConsumeQueue=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/consumequeue
storePathIndex=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/index
storeCheckpoint=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/checkpoint
abortFile=/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release/storeDledger/abort
enableDLegerCommitLog=true
dLegerGroup=RaftNode02
dLegerPeers=n0-vm01:40911;n1-vm02:40911;n2-vm03:40911

# 值必须唯一
dLegerSelfId=n2
sendMessageThreadPoolNums=16
```   

### 启动 
```bash
# 切换到RocketMQ运行包根目录

# 在rmq1服务器上，启动命令
nohup ./mqbroker -c ../conf/dledger/broker-n0.conf &

# 在rmq2服务器上，启动命令
nohup ./mqbroker -c ../conf/dledger/broker-n1.conf &

# 在rmq3服务器上，启动命令
nohup ./mqbroker -c ../conf/dledger/broker-n2.conf &
```