## Spark - Spark History 部署   

>Spark version: 3.5.5  

### 重命名 spark-defaults.conf.template 为 spark-defaults.conf 
```bash 
mv conf/spark-defaults.conf.template conf/spark-defaults.conf
``` 

### 修改 conf/spark-defaults.conf 配置文件  
vim conf/spark-defaults.conf 
```bash
# 内容如下：
spark.eventLog.enabled=true
spark.eventLog.compress=true
spark.eventLog.dir=hdfs://bigdata01:8020/tmp/logs/spark/logs
spark.history.fs.logDirectory=hdfs://bigdata01:8020//tmp/logs/spark/logs
spark.yarn.historyServer.address=http://bigdata04:18080
```

### 修改 conf/spark-env.sh 配置文件 
vim conf/spark-env.sh
```bash
export SPARK_HISTORY_OPTS="-Dspark.history.ui.port=18080 -Dspark.history.fs.logDirectory=hdfs://bigdata01:8020/tmp/logs/spark/logs"
```

### 启动 Spark History  
使用 hdfs 命令创建好 `/tmp/logs/spark/logs` 目录，命令如下：  
```bash
[hadoop@bigdata01 hadoop-3.2.0]$ hdfs dfs -mkdir -p /tmp/logs/hadoop/logs     
``` 

执行 `sbin/start-history-server.sh` 命令，启动 Spark History。           

TODO 添加一个 Spark Job 示例     
