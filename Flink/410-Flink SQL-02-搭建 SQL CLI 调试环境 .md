# Flink SQL - 搭建 SQL CLI 调试环境    

>Flink version: 1.17.2       

## 引言   


## 了解 sql-client.sh 脚本 和 SqlClient 进程 
通过`/bin/sql-client.sh`脚本会启动一个 SqlClient 进程，可通过`jps`命令查看。   
```bash
[root@vm01 lib]# jps
118673 StandaloneSessionClusterEntrypoint
65489 QuorumPeerMain
118966 TaskManagerRunner
65993 Kafka
11770 SqlClient
15503 Jps
```     

像 SqlClient 常驻后台java 进程使用，省去了调试`sql-client.sh`脚本过程（例如添加脚本首尾添加 set -x、set +x 又或者修改脚本 添加 echo 打印完整脚本），可通过`ps -ef|grep SqlClient`命令查看进程启动的完整命令，内容如下：           
```shell
[root@vm01 lib]# ps -ef|grep SqlClient  
root      11770  62445 28 21:21 pts/0    00:00:05 /data/jdk1.8.0_391/bin/java -Dlog.file=/root/yzhou/flink/flink1172/flink-1.17.2/log/flink-root-sql-client-vm01.log -Dlog4j.configuration=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j-cli.properties -Dlog4j.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j-cli.properties -Dlogback.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/logback.xml -classpath /root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-cep-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-connector-files-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-connector-jdbc-3.1.2-1.17.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-csv-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-json-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-scala_2.12-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-sql-connector-kafka-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-sql-connector-mysql-cdc-2.4.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-api-java-uber-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-planner-loader-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-runtime-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-udf-1.0-SNAPSHOT.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-1.2-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-core-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-slf4j-impl-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/mysql-connector-j-8.0.33.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-dist-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/opt/flink-python-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/opt/flink-sql-gateway-1.17.2.jar::/opt/module/hadoop-3.1.3/etc/hadoop::/root/yzhou/flink/flink1172/flink-1.17.2/opt/flink-sql-client-1.17.2.jar org.apache.flink.table.client.SqlClient --jar /root/yzhou/flink/flink1172/flink-1.17.2/opt/flink-sql-client-1.17.2.jar      
```  

>特别注意，Idea 配置 SqlClient的调试环境，是基于之前 Blog `Flink 源码 - Standalone - 通过 CliFrontend 提交 Job 到 Standalone 集群`中`devcliconf`目录下的配置，例如将 devcliconf/ 目录 拷贝一份，命名为 devsqlclientconf，其目的是为了保证配置隔离开，避免在做一些测试时影响到其他服务调试。        

## 配置 Idea 启动 SqlClient      
首先，我们在项目根目录下创建一个 `devconf` 文件夹，将 `devconf`下的文件 copy 到 devcliconf 文件夹下。 







refer        
1.https://github.com/xinzhuxiansheng/flink/tree/yzhou/release-1.17    



1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/table/sqlclient/      
2.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/table/functions/systemfunctions/   
3.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/table/functions/udfs/      
4.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/table/sql/overview/        

