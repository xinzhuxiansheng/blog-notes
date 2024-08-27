# Flink SQL - SQL Client - 搭建 SQL CLI 调试环境    

>Flink version: 1.17.2          

## 优化     
在之前的两篇 Blog `Flink 源码 - Standalone - 通过 CliFrontend 提交 Job 到 Standalone 集群` 和 `Flink 源码 - Standalone - 通过 CliFrontend 提交 Job 到 Standalone 集群` 中涉及到 Idea 配置启动项时手动对每个 devlib/下的 jar 添加到 classpath中，如下图：  
![standalonedebug04](http://img.xinzhuxiansheng.com/blogimgs/flink/standalonedebug04.png)    

上述操作不是最优解，我们可以在 Idea 中打开项目的`Project Structure`窗口，给某个 `Modules`添加 Dependencies。（具体选择哪个模块，可直接选择启动类所在的模块即可）   
![sqlclidebug01](http://img.xinzhuxiansheng.com/blogimgs/flink/sqlclidebug01.png)     

关于依赖添加的优化已介绍完了，接下来，介绍搭建 SQL CLI 调试环境。   

## 引言    
在之前的 Blog`Flink SQL - SQL Client - SQL CLI 的使用`中介绍了 Flink SQL CLI在开发 Flink SQL Job时的使用场景和收益，在实时计算平台化过程中，一种是平台功能不够完善，一种是使用成本高（例如 on yarn、on k8s 相对于 Standalone集群部署慢）等因素，写该篇 Blog 目的是想了解 Flink SQL CLI实现过程，其次是想开发一个`Flink SQL WEB CLI`集成到实时计算平台里面，也许有人会说这会不会与 SQL IDE的功能冲突，我个人理解是有些，更准确的说，SQL WEB CLI 丰富了 SQL IDE的功能。  

期待 `Flink SQL WEB CLI`       

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

## 配置 Idea 启动 SqlClient      
首先，我们在项目根目录下创建一个 `devsqlclientconf` 文件夹，将 `devcliconf`下的文件 copy 到 devsqlclientconf 文件夹下。(具体内容可访问 https://github.com/xinzhuxiansheng/flink/tree/yzhou/release-1.17 查看 devsqlclientconf/)            

### 1）配置 SqlClient 启动项    
配置启动类：`org.apache.flink.table.client.SqlClient`           
JDK: 1.8    
Module: `flink-sql-clients`   
VM options:   
```shell 
-Dlog.file=./log/flink-root-sql-client-local.log
-Dlog4j.configuration=./devsqlclientconf/log4j-cli.properties
-Dlog4j.configurationFile=./devsqlclientconf/log4j-cli.properties   
-Dlogback.configurationFile=./devsqlclientconf/logback.xml
```

Main class: org.apache.flink.table.client.SqlClient     
Program arguments:  --jar [flink-sql-client 模块编译打包后的 jar], 下面是本人的示例路径：`D:\Code\Java\flink-all\flink_release-1.17\flink-table\flink-sql-client\target\flink-sql-client-1.17-SNAPSHOT.jar`    
Environment variables: FLINK_CONF_DIR=./devsqlclientconf       

打开项目的`Project Structure`窗口，选择`flink-sql-clients`Module,再添加 Dependencies。     
![sqlclidebug01](http://img.xinzhuxiansheng.com/blogimgs/flink/sqlclidebug01.png)          

完整的示例，如下图：  
![sqlclidebug02](http://img.xinzhuxiansheng.com/blogimgs/flink/sqlclidebug02.png)     

### 2）Idea 中 启动 main()方法，测试 sql client    
>注意，在测试之前，我首先是在其他 vm 机器上启动了一个 Flink Standalone集群，这块在之前的 Blog 有介绍过，可以通过修改 flink-conf.yaml 来改变 Flink Job 提交的集群。  

启动 org.apache.flink.table.client.SqlClient#main()方法，下面是启动成功的示例图：  
![sqlclidebug03](http://img.xinzhuxiansheng.com/blogimgs/flink/sqlclidebug03.png)      

>另一个注意点是：devlib/ 目录下的 jar 不是一尘不变的，你需要根据你实际执行的 Sql，添加相应的依赖，再重新启动 main()方法;  

下面通过Flink SQL 示例来测试：    

首先是建表语句，这里有个很大的疑惑是，当 sql copy 到 Idea 的控制台时，会自动粘贴2此，但这不影响执行结果，这块后续会跟进下。    
```sql
--建表语句
CREATE TABLE `yzhou_test02` 
(
  `id` INT NOT NULL COMMENT '',
  `name` STRING NOT NULL COMMENT '',
  `address` STRING COMMENT '',
  `ext_field01` STRING COMMENT '',
  PRIMARY KEY (id) NOT ENFORCED
)
WITH
(
  'connector' = 'jdbc',
  'table-name' = 'yzhou_test02',
  'url' = 'jdbc:mysql://192.168.0.201:3306/yzhou_test',
  'username' = 'root',
  'password' = '123456'
);
```

等建表创建成功后，执行查询 SQL：  
```sql
--查询语句
select * from yzhou_test02;
```

执行结果，如下图：   
![sqlclidebug04](http://img.xinzhuxiansheng.com/blogimgs/flink/sqlclidebug04.png)  

## 总结   
SQL CLI调试环境已经搭建完成，接下来，深入了解它，为后续开发`Flink SQL WEB CLI`做技术调研准备。       

refer        
1.https://github.com/xinzhuxiansheng/flink/tree/yzhou/release-1.17          
