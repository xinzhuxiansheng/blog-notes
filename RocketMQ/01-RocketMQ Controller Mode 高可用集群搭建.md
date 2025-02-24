# RocketMQ Controller Mode 高可用集群搭建      
>RocketMQ Version: 5.3.1

## 部署   

## namesrv-nX.conf

```bash
[root@bigdata01 conf]# cat namesrv-n0.conf
listenPort = 9876
enableControllerInNamesrv = true

enableElectUncleanMaster = false
notifyBrokerRoleChanged = true
controllerType = jRaft
controllerStorePath = /root/rocketmq-5.3.1/data/jRaftController
jRaftGroupId = jRaft-Controller
jRaftServerId = 192.168.0.130:9880
jRaftInitConf = 192.168.0.130:9880,192.168.0.131:9880,192.168.0.132:9880
jRaftControllerRPCAddr = 192.168.0.130:9770,192.168.0.131:9770,192.168.0.132:9770
jRaftSnapshotIntervalSecs = 3600
```

* jRaftServerId 需要适配不同节点  


## broker-nX.conf

```bash
[root@bigdata01 conf]# cat broker-n0.conf
brokerClusterName = 351cluster
brokerName = broker-a
brokerId = -1
brokerRole = SLAVE
deleteWhen = 04
fileReservedTime = 48
enableControllerMode = true

controllerAddr = 192.168.0.130:9770;192.168.0.131:9770,192.168.0.132:9770
namesrvAddr=192.168.0.130:9876;192.168.0.131:9876;192.168.0.132:9876
allAckInSyncStateSet=true
listenPort=30911
storePathRootDir=/root/rocketmq-5.3.1/data/
storePathCommitLog=/root/rocketmq-5.3.1/data/commitlog
fetchNamesrvAddrByAddressServer = true
autoCreateTopicEnable=false
enablePropertyFilter=true
slaveReadEnable=true
```

## log
```bash
## 在 conf/ 目录下
sed -i 's|\${user.home}|/root/rocketmq/dledger/rocketmq-all-4.9.5-bin-release|g' *.xml
```



```bash

nohup sh bin/mqnamesrv -c /root/rocketmq-5.3.1/conf/namesrv-n0.conf > /dev/null 2>&1 &  

nohup ./mqbroker -c ../conf/broker-n0.conf &
```




refer       
1.https://github.com/apache/rocketmq/blob/develop/docs/cn/controller/deploy.md      
