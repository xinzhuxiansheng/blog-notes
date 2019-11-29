**正文**

## Start Command



### Non-federation Mode

`Controller:`

java -Dlog4j.configuration=file:config/tools-log4j.properties -Xms3g -Xmx3g -Xmn512m -XX:NewSize=512m -XX:MaxNewSize=512m -XX:+AlwaysPreTouch -XX:+UseCompressedOops -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+DisableExplicitGC -XX:+PrintCommandLineFlags -XX:CMSInitiatingOccupancyFraction=80 -XX:SurvivorRatio=2 -XX:+PrintTenuringDistribution -XX:+PrintGCDetails -XX:+PrintGCDateStamps -XX:+PrintGCApplicationStoppedTime -XX:+PrintGCApplicationConcurrentTime -XX:+PrintGCTimeStamps -Xloggc:/tmp/ureplicator-controller/gc-ureplicator-controller.log -server -cp uReplicator-Controller/target/uReplicator-Controller-1.0.0-SNAPSHOT-jar-with-dependencies.jar com.uber.stream.kafka.mirrormaker.controller.ControllerStarter -enableFederated false -deploymentName <cluster1-cluster2> -helixClusterName uReplicator -mode customized -zookeeper zk1,zk2,zk3/ureplicator/<cluster1-cluster2> -port <port> -env <dc1>.<cluster1-cluster2> -instanceId 0 -hostname <hostname> -graphiteHost 127.0.0.1 -graphitePort 4756 -metricsPrefix ureplicator-controller -enableAutoWhitelist false -enableAutoTopicExpansion true -autoRebalanceDelayInSeconds 120 -autoRebalancePeriodInSeconds 120 -autoRebalanceMinIntervalInSeconds 600 -autoRebalanceMinLagTimeInSeconds 900 -autoRebalanceMinLagOffset 100000 -autoRebalanceMaxOffsetInfoValidInSeconds 1800 -autoRebalanceWorkloadRatioThreshold 1.5 -maxDedicatedLaggingInstancesRatio 0.2 -maxStuckPartitionMovements 3 -moveStuckPartitionAfterMinutes 20 -workloadRefreshPeriodInSeconds 300 -patternToExcludeTopics ^__.* -enableSrcKafkaValidation true -srcKafkaZkPath "src_zk/cluster1" -destKafkaZkPath "dst_zk/cluster2" -consumerCommitZkPath "" -maxWorkingInstances 0 -autoRebalanceDelayInSeconds 120 -refreshTimeInSeconds 600 -initWaitTimeInSeconds 120 -numOffsetThread 10 -blockingQueueSize 30000 -offsetRefreshIntervalInSec 300 -groupId <ureplicator-cluster1-cluster2> -backUpToGit false -localBackupFilePath /tmp/ureplicator-controller -localGitRepoClonePath /ureplicator-controller-bkp

`VM options:`
-Dlog4j.configuration=file:config/tools-log4j.properties
-Xms1g
-Xmx1g
-Xmn512m
-XX:NewSize=512m
-XX:MaxNewSize=512m
-XX:+AlwaysPreTouch
-XX:+UseCompressedOops
-XX:+UseConcMarkSweepGC
-XX:+UseParNewGC
-XX:+DisableExplicitGC
-XX:+PrintCommandLineFlags
-XX:CMSInitiatingOccupancyFraction=80
-XX:SurvivorRatio=2
-XX:+PrintTenuringDistribution
-XX:+PrintGCDetails
-XX:+PrintGCDateStamps
-XX:+PrintGCApplicationStoppedTime
-XX:+PrintGCApplicationConcurrentTime
-XX:+PrintGCTimeStamps
-Xloggc:D:/ureplicator-controller/gc-ureplicator-controller.log
-server

`Program arguments:`
-enableFederated
false
-deploymentName
vm01-vm02
-helixClusterName
uReplicator
-mode
customized
-zookeeper
vm01.com:2181
-port
9066
-env
dc1.vm01-vm02
-instanceId
0
-hostname
localpc.com
-graphiteHost
127.0.0.1
-graphitePort
2003
-metricsPrefix
ureplicator-controller
-enableAutoWhitelist
false
-enableAutoTopicExpansion
true
-autoRebalanceDelayInSeconds
120
-autoRebalancePeriodInSeconds
120
-autoRebalanceMinIntervalInSeconds
600
-autoRebalanceMinLagTimeInSeconds
900
-autoRebalanceMinLagOffset
100000
-autoRebalanceMaxOffsetInfoValidInSeconds
1800
-autoRebalanceWorkloadRatioThreshold
1.5
-maxDedicatedLaggingInstancesRatio
0.2
-maxStuckPartitionMovements
3
-moveStuckPartitionAfterMinutes
20
-workloadRefreshPeriodInSeconds
300
-patternToExcludeTopics
^__.*
-enableSrcKafkaValidation
true
-srcKafkaZkPath
vm01.com:2181/vmkafka010
-destKafkaZkPath
vm01.com:2181/vm02_kafka010
-consumerCommitZkPath
""
-maxWorkingInstances
0
-autoRebalanceDelayInSeconds
120
-refreshTimeInSeconds
600
-initWaitTimeInSeconds
120
-numOffsetThread
10
-blockingQueueSize
30000
-offsetRefreshIntervalInSec
300
-groupId
ureplicator-vm01-vm02
-backUpToGit
false
-localBackupFilePath
D:/ureplicator-controller
-localGitRepoClonePath
D:/ureplicator-controller-bkp


`Worker-3.0:`

java -Dlog4j.configuration=file:config/tools-log4j.properties -XX:MaxGCPauseMillis=100 -XX:InitiatingHeapOccupancyPercent=45 -verbose:gc -Xmx5g -Xms5g -XX:+UseG1GC -XX:+PrintGCDetails -XX:+PrintGCTimeStamps -XX:+PrintGCDateStamps -XX:+PrintTenuringDistribution -server -javaagent:./bin/libs/jmxtrans-agent-1.2.4.jar=config/jmxtrans.xml -cp uReplicator-Worker/target/uReplicator-Worker-1.0.0-SNAPSHOT-jar-with-dependencies.jar kafka.mirrormaker.MirrorMakerWorker --consumer.config config/consumer.properties --producer.config config/producer.properties --helix.config config/helix.properties --dstzk.config config/dstzk.propertiess


`配置Object的main方法`
在uReplicator-Worker module中没有Object的main方法，所以只能自己定义一个 Object的main
```java
object MirrorMakerWorkerMain {

  def main(args: Array[String]): Unit = {
    new MirrorMakerWorker().main(args);
  }
}
```
参考地址: https://github.com/uber/uReplicator/issues/268


`VM options:`

-Dlog4j.configuration=file:config/tools-log4j.properties
-XX:MaxGCPauseMillis=100
-XX:InitiatingHeapOccupancyPercent=45
-verbose:gc
-Xmx1g
-Xms1g
-XX:+UseG1GC
-XX:+PrintGCDetails
-XX:+PrintGCTimeStamps
-XX:+PrintGCDateStamps
-XX:+PrintTenuringDistribution
-server
-javaagent:./bin/libs/jmxtrans-agent-1.2.4.jar=config/jmxtrans.xml

`Program arguments:`

-enableFederated
false
-consumer_config
config/consumer.properties
-producer_config
config/producer.properties
-helix_config
config/helix.properties


### 添加whilelist 
post localhost:9066/topics/yzhoutest01  结果：Successfully add new topic: {topic: yzhoutest01, partition: 3, pipeline: null}
### 删除whilelist




### Federation Mode
`Manager:`

java -Dlog4j.configuration=file:config/tools-log4j.properties -Xms3g -Xmx3g -Xmn512m -XX:NewSize=512m -XX:MaxNewSize=512m -XX:+AlwaysPreTouch -XX:+UseCompressedOops -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+DisableExplicitGC -XX:+PrintCommandLineFlags -XX:CMSInitiatingOccupancyFraction=80 -XX:SurvivorRatio=2 -XX:+PrintTenuringDistribution -XX:+PrintGCDetails -XX:+PrintGCDateStamps -XX:+PrintGCApplicationStoppedTime -XX:+PrintGCApplicationConcurrentTime -XX:+PrintGCTimeStamps -Xloggc:/tmp/ureplicator-manager/gc-ureplicator-manager.log -server -cp uReplicator-Manager/target/uReplicator-Manager-1.0.0-SNAPSHOT-jar-with-dependencies.jar com.uber.stream.kafka.mirrormaker.manager.ManagerStarter -config config/clusters.properties -srcClusters cluster1,cluster2 -destClusters cluster3 -enableRebalance false -zookeeper zk1,zk2,zk3/ureplicator/testing-dc1 -managerPort <port> -deployment testing-dc1 -env dc1.testing-dc1 -instanceId <id> -controllerPort <port> -graphiteHost 127.0.0.1 -graphitePort 4756 -metricsPrefix ureplicator-manager -workloadRefreshPeriodInSeconds 300 -initMaxNumPartitionsPerRoute 1500 -maxNumPartitionsPerRoute 2000 -initMaxNumWorkersPerRoute 10 -maxNumWorkersPerRoute 80


`Controller:`

java -Dlog4j.configuration=file:config/tools-log4j.properties -Xms3g -Xmx3g -Xmn512m -XX:NewSize=512m -XX:MaxNewSize=512m -XX:+AlwaysPreTouch -XX:+UseCompressedOops -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+DisableExplicitGC -XX:+PrintCommandLineFlags -XX:CMSInitiatingOccupancyFraction=80 -XX:SurvivorRatio=2 -XX:+PrintTenuringDistribution -XX:+PrintGCDetails -XX:+PrintGCDateStamps -XX:+PrintGCApplicationStoppedTime -XX:+PrintGCApplicationConcurrentTime -XX:+PrintGCTimeStamps -Xloggc:/tmp/ureplicator-controller/gc-ureplicator-controller.log -server -cp uReplicator-Controller/target/uReplicator-Controller-1.0.0-SNAPSHOT-jar-with-dependencies.jar com.uber.stream.kafka.mirrormaker.controller.ControllerStarter -config config/clusters.properties -srcClusters cluster1,cluster2 -destClusters cluster3 -enableFederated true -deploymentName testing-dc1 -mode customized -zookeeper zk1,zk2,zk3/ureplicator/testing-dc1 -port <port> -env dc1.testing-dc1 -instanceId <id> -hostname <hostname> -graphiteHost 127.0.0.1 -graphitePort 4756 -metricsPrefix ureplicator-controller -enableAutoWhitelist false -enableAutoTopicExpansion true -autoRebalanceDelayInSeconds 120 -autoRebalancePeriodInSeconds 120 -autoRebalanceMinIntervalInSeconds 600 -autoRebalanceMinLagTimeInSeconds 900 -autoRebalanceMinLagOffset 100000 -autoRebalanceMaxOffsetInfoValidInSeconds 1800 -autoRebalanceWorkloadRatioThreshold 1.5 -maxDedicatedLaggingInstancesRatio 0.2 -maxStuckPartitionMovements 3 -moveStuckPartitionAfterMinutes 20 -workloadRefreshPeriodInSeconds 300 -patternToExcludeTopics ^__.* -enableSrcKafkaValidation true -consumerCommitZkPath "" -maxWorkingInstances 0 -autoRebalanceDelayInSeconds 120 -refreshTimeInSeconds 600 -initWaitTimeInSeconds 120 -numOffsetThread 10 -blockingQueueSize 30000 -offsetRefreshIntervalInSec 300 -backUpToGit false -localBackupFilePath /tmp/ureplicator-controller -localGitRepoClonePath /ureplicator-controller-bkp

`Worker:`

java -Dlog4j.configuration=file:config/tools-log4j.properties -XX:MaxGCPauseMillis=100 -XX:InitiatingHeapOccupancyPercent=45 -verbose:gc -Xmx5g -Xms5g -XX:+UseG1GC -XX:+PrintGCDetails -XX:+PrintGCTimeStamps -XX:+PrintGCDateStamps -XX:+PrintTenuringDistribution -server -javaagent:./bin/libs/jmxtrans-agent-1.2.4.jar=config/jmxtrans.xml -cp uReplicator-Worker/target/uReplicator-Worker-1.0.0-SNAPSHOT-jar-with-dependencies.jar kafka.mirrormaker.MirrorMakerWorker --cluster.config config/clusters.properties --consumer.config config/consumer.properties --producer.config config/producer.properties --helix.config config/helix.properties --dstzk.config config/dstzk.propertiess



## 异常：
g.apache.kafka.common.config.ConfigException: Invalid value smallest for configuration auto.offset.reset: String must be one of: latest, earliest, none

在config/consumer.properties 配置 auto.offset.reset=latest 


client.id = ureplicator-null-null-1
???


