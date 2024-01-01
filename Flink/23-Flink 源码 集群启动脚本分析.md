## Flink 源码 集群启动脚本分析  

### 介绍    
Flink 集群的启动脚本在：flink-dist 子项目中，位于 flink-bin 下的 bin 目录：启动脚本为 `start-cluster.sh`, 该脚本会首先调用 config.sh 来获取 masters 和 workers， masters的信息，是从 conf/masters 配置文件中获取的，workers 是从 conf/workers 配置文件中获取的。然后分别：  

```shell
1. 通过 jobmanager.sh 来启动 JobManager
2. 通过 taskmanager.sh 来启动 TaskManager   
```
他们的内部，都通过 flink-daemon.sh 脚本来启动 JVM进程，分别 flink-daemon.sh 来启动：        
```
1. JobManager 的启动代号： standalonesession，实现类是 StandaloneSessionClusterEntrypoint   
2. taskManager 的启动代号： taskexecutor，实现类是 TaskManagerRunner    
```
  
