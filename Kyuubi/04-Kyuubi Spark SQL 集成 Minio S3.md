# Kyuubi - Kyuubi Spark SQL 集成 Minio S3   

>Jdk version: 1.8, Hadoop version: 3.2.0, Hive version: 3.1.3, Spark version: 3.5.5, Kyuubi version: 1.10.2    

## 引言  
在上一篇名为 “Atlas - 探索 Atlas 2.3.0 部署 & 集成 Hive SQL” 公众号文章中花了较大的篇幅介绍了 Hadoop，Hive，Hbase 等组件的搭建步骤，它们是探索 Atlas 功能必不可少的一部分。这段时间博主也一直在排查 Atlas 相关问题，一部分问题会涉及修改一些 Atlas，Kyuubi 源码。关于修改的细节，博主也会在后续的文章中介绍它们。  

`下面列举了一个 Hive Table 的 lineage 示例图：`     

![hiveons301](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons301.jpg)    

>对于 “环境” ，我仍然需要优化。     

目前搭建的 Atlas 基础环境与博主实际工作的环境相差甚远，其中比较明显差异化的是 HDFS、YARN、Hive SQL，而实际工作中使用的是支持 S3协议的对象存储、 Kubernetes 以及 Hive Metastore + Kyuubi + Spark SQL。  

关于这部分架构的转变，博主花了不少时间来学习。例如我们使用 Spark SQL 来查询 Hive 表，
集成 Apache Celeborn，Blaze 组件优化查询效率，Hive 表的数据不再存储在 HDFS中，而是对象存储中，例如华为的 OBS 对象存储，在后面的实践中博主采用 Minio 存储 Hive 表数据。  

例如：`Hive 建库`,`建表` 语句需要配置 LOCATION 参数用来定义数据存储的 S3 路径地址。   
```bash
# 建库
create database yzhou_db LOCATION  "s3a://hive/warehouse/yzhou_db";        

# 建表 
CREATE TABLE yzhou_db.dim_base_province (
 `id` STRING COMMENT '编号',
 `name` STRING COMMENT '省份名称',
 `region_id` STRING COMMENT '地区 ID',
 `area_code` STRING COMMENT '地区编码',
 `iso_code` STRING COMMENT 'ISO-3166 编码，供可视化使用',
 `iso_3166_2` STRING COMMENT 'IOS-3166-2 编码，供可视化使用'
) COMMENT '省份表'
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\t'
LOCATION 's3a://hive/warehouse/yzhou_db/dim_base_province';  
```   

下面博主会循序渐进的介绍如何集成 S3，这里主要分4个部分介绍： 
* Hadoop 集成 S3   
* Hive 集成 S3 
* Spark SQL 集成 S3  
* Kyuubi 提交 Spark SQL 

>为了保证搭建顺序的完整性，该篇的 Hadoop，Hive 基于上一篇 ““Atlas - 探索 Atlas 2.3.0 部署 & 集成 Hive SQL” 公众号文章中的基础环境。 其他组件，例如 Kyuubi，Spark，Minio 本篇会详细介绍部署过程。    

## Minio Standalone 部署  
Minio 是否是 Cluster 并不是重点，单节点足够测试使用。   

### 1）下载 minio 
访问 `https://www.min.io/download?platform=linux&arch=amd64` 下载二进制 minio   
```bash
wget https://dl.min.io/aistor/minio/release/linux-amd64/minio
chmod +x minio
./minio --version
```

### 2）编写 start.sh 启动脚本 
将 minio 拷贝到 /usr/local/bin/ 目录下， 编写 start.sh 脚本，注意设置 minio data 存放目录，内容如下：  
```bash
nohup /usr/local/bin/minio server --address :9000 --console-address :9001 /root/minio/data > /root/minio/minio.log &     
```  

### 3）设置 access.key 和 secret.key  
access.key,secret.key 分别对应的 Minio Web UI 的账号/密码， 它们是通过环境变量设置的，如下所示：  
```bash 
vim /etc/profile  

export MINIO_ACCESS_KEY=minio
export MINIO_SECRET_KEY=minio123
```  

### 4）启动 minio  
```bash
sh start.sh 
```

浏览器访问 `192.168.0.135:9001` ，登录 Minio Web UI。 用户名和密码分别是 minio，minio123 。   

![hiveons302](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons302.jpg)   

### 5）创建名为 hive 的 Bucket  
如下所示, 创建名为 hive 的Bucket。在 hive bucket 创建名为 `warehouse` path，用于存放 Hive Data。      
![hiveons303](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons303.jpg)   

此时，minio standalone 已经部署完毕。   

## Hadoop 集成 S3    

### 添加 S3 依赖 jar 
首先 `/opt/module/hadoop-3.2.0` 安装目录下的 `/opt/module/hadoop-3.2.0/share/hadoop/tools/lib` 路径自带了 S3 相关的 jar，如下所示：  
```bash
[root@bigdata01 lib]# ls |grep aws
aws-java-sdk-bundle-1.11.375.jar
hadoop-aws-3.2.0.jar   
```

将 `hadoop-aws-3.2.0.jar`,`aws-java-sdk-bundle-1.11.375.jar` 拷贝 /opt/module/hadoop-3.2.0/lib 目录下。   

### 修改 etc/hadoop/core-site.xml 配置文件  
在 etc/hadoop/core-site.xml 配置文件中添加以下配置， 9000 是 minio 服务端口；
```bash
<property>
    <name>fs.s3a.access.key</name>
    <value>minio</value>
</property>
<property>
    <name>fs.s3a.secret.key</name>
    <value>minio123</value>
</property>
<property>
    <name>fs.s3a.connection.ssl.enabled</name>
    <value>false</value>
</property>
<property>
    <name>fs.s3a.path.style.access</name>
    <value>true</value>
</property>
<property>
    <name>fs.s3a.endpoint</name>
    <value>http://192.168.0.135:9000</value>
</property>
<property>
    <name>fs.s3a.impl</name>
    <value>org.apache.hadoop.fs.s3a.S3AFileSystem</value>
</property>

```

配置完成后，使用 `/opt/shell/hadoop.sh stop`,`/opt/shell/hadoop.sh start` 重启 Hadoop 集群。 此时我们就可以使用 hdfs 命令中查看 minio 的文件了。 命令格式如下：  
```bash
# 切换 hadoop 用户
su hadoop

# [bucket] 是 桶的名称
hdfs dfs -ls s3a://[bucket]/ 
```

示例操作 如下所示：  
![hiveons304](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons304.jpg)


>注意：此时的 hdfs 命令仍然可以查看 hdfs 的路径，如下所示：  
```bash
[hadoop@bigdata01 hadoop-3.2.0]$ hdfs dfs -ls /
Found 4 items
drwxr-xr-x   - hadoop supergroup          0 2025-09-21 15:19 /HBase
drwxr-xr-x   - hadoop supergroup          0 2025-09-26 00:38 /test
drwxrwxrwx   - hadoop supergroup          0 2025-09-21 20:09 /tmp
drwxr-xr-x   - hadoop supergroup          0 2025-09-25 23:56 /user
```

这是因为我们没有设置 `fs.defaultFS` 参数为 S3 的一个 bucket。  修改 etc/hadoop/core-site.xml 配置文件的 fs.defaultFS 为 s3a 
```bash
<property>
  <name>fs.defaultFS</name>
  <value>s3a://[default-bucket]/</value>
</property>
```  

## Hive 集成 S3 

### 添加 S3 依赖 jar 
同 hadoop 配置 S3 一样，从 `/opt/module/hadoop-3.2.0/share/hadoop/tools/lib` 路径下将 `hadoop-aws-3.2.0.jar`,`aws-java-sdk-bundle-1.11.375.jar` 拷贝 /opt/module/hive-3.1.3/lib 下 。  

### 修改 conf/hive-site.xml  
修改 hive 目录下 conf/hive-site.xml 配置文件，添加以下内容：  
```bash
<property>
    <name>fs.s3a.access.key</name>
    <value>minio</value>
</property>
<property>
    <name>fs.s3a.secret.key</name>
    <value>minio123</value>
</property>
<property>
    <name>fs.s3a.connection.ssl.enabled</name>
    <value>false</value>
</property>
<property>
    <name>fs.s3a.path.style.access</name>
    <value>true</value>
</property>
<property>
    <name>fs.s3a.endpoint</name>
    <value>http://192.168.0.135:9000</value>
</property>
<property>
    <name>fs.s3a.impl</name>
    <value>org.apache.hadoop.fs.s3a.S3AFileSystem</value>
</property>
```

### 启动 Hive Cli  
在 hive 目录下执行 `bin/hive` 命令，进入 Hive Cli 命令行，现在我们通过 Hive Cli 创建 S3 存储的 Hive 库 和 Hive 表。 下面是示例：   

#### 创建名为 yzhou_db database  
```bash
create database yzhou_db location  "s3a://hive/warehouse/yzhou_db";  
```

输出结果如下：   
![hiveons305](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons305.jpg)

创建成功后，访问 Minio Web UI (http://192.168.0.135:9001/) 查看目录信息, 如下图所示，在 /hive/warehouse 目录下多了一个 `yzhou_db` 文件夹。   
![hiveons306](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons306.jpg)  

#### 创建名为 dim_base_province table 
```bash
CREATE TABLE yzhou_db.dim_base_province (
 `id` STRING COMMENT '编号',
 `name` STRING COMMENT '省份名称',
 `region_id` STRING COMMENT '地区 ID',
 `area_code` STRING COMMENT '地区编码',
 `iso_code` STRING COMMENT 'ISO-3166 编码，供可视化使用',
 `iso_3166_2` STRING COMMENT 'IOS-3166-2 编码，供可视化使用'
) COMMENT '省份表'
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\t'
LOCATION 's3a://hive/warehouse/yzhou_db/dim_base_province';
``` 

输出结果：   
![hiveons307](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons307.jpg)   

创建成功后，访问 Minio Web UI (http://192.168.0.135:9001/) 查看目录信息, 如下图所示，在 /hive/warehouse/yzhou_db 目录下多了一个 `dim_base_province` 文件夹。   
![hiveons308](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons308.jpg)     


#### 上传数据
为了让 `dim_base_province` 表有数据，可以创建一个名为 'base_province.txt' 文件，将以下内容拷贝到文件。  
```bash
1	北京	1	110000	CN-11	CN-BJ
2	天津	1	120000	CN-12	CN-TJ
3	山西	1	140000	CN-14	CN-SX
4	内蒙古	1	150000	CN-15	CN-NM
5	河北	1	130000	CN-13	CN-HE
6	上海	2	310000	CN-31	CN-SH
7	江苏	2	320000	CN-32	CN-JS
8	浙江	2	330000	CN-33	CN-ZJ
9	安徽	2	340000	CN-34	CN-AH
10	福建	2	350000	CN-35	CN-FJ
11	江西	2	360000	CN-36	CN-JX
12	山东	2	370000	CN-37	CN-SD
14	台湾	2	710000	CN-71	CN-TW
15	黑龙江	3	230000	CN-23	CN-HL
16	吉林	3	220000	CN-22	CN-JL
17	辽宁	3	210000	CN-21	CN-LN
18	陕西	7	610000	CN-61	CN-SN
19	甘肃	7	620000	CN-62	CN-GS
20	青海	7	630000	CN-63	CN-QH
21	宁夏	7	640000	CN-64	CN-NX
22	新疆	7	650000	CN-65	CN-XJ
23	河南	4	410000	CN-41	CN-HA
24	湖北	4	420000	CN-42	CN-HB
25	湖南	4	430000	CN-43	CN-HN
26	广东	5	440000	CN-44	CN-GD
27	广西	5	450000	CN-45	CN-GX
28	海南	5	460000	CN-46	CN-HI
29	香港	5	810000	CN-91	CN-HK
30	澳门	5	820000	CN-92	CN-MO
31	四川	6	510000	CN-51	CN-SC
32	贵州	6	520000	CN-52	CN-GZ
33	云南	6	530000	CN-53	CN-YN
13	重庆	6	500000	CN-50	CN-CQ
34	西藏	6	540000	CN-54	CN-XZ
```

再将该文件通过 Minio Web UI 上传到 `dim_base_province` 文件夹下。   
![hiveons309](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons309.jpg)   

此时，我们继续在 Hive Cli 执行 `select * from yzhou_db.dim_base_province;` 语句查询 dim_base_province 表数据。     
![hiveons310](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons310.jpg)    

此时，Hive 集成 S3 也已完成。  

## Spark SQL on Yarn 替换 Hive SQL   
Spark 需要集成 Hive Metastore 服务，所以需要在 hive 安装目录下启动 metastore 服务。  

### Hive Metastore 部署 
在 hive 目录下执行 `nohup bin/hive --service metastore -p 9083 2>&1 >/dev/null &` 命令启动；  

### Spark 部署 
访问 `https://archive.apache.org/dist/spark/spark-3.5.5/` 下载 Spark 3.5.5 安装包。  

```bash 
# 下载 spark 3.5.5
wget https://archive.apache.org/dist/spark/spark-3.5.5/spark-3.5.5-bin-hadoop3.tgz  

# 解压并且指定目录 
tar -zxf spark-3.5.5-bin-hadoop3.tgz -C /opt/module 
```

### 配置 hive-site.xml 配置文件  
将 hive 安装目录下的 conf/hive-site.xml 拷贝到 Spark conf/ 目录下。  
```bash
cp /opt/module/hive-3.1.3/conf/hive-site.xml /opt/module/spark-3.5.5-bin-hadoop3/conf/
```  

>注意：我们启动了 Hive Metastore ，所以需要在 `hive-site.xml` 配置文件中添加 `hive.metastore.uris`，示例如下：  
```bash
<property>
    <name>hive.metastore.uris</name>
    <value>thrift://bigdata04:9083</value>
</property>
```

### 配置 spark-env.sh 配置文件  
```bash
# 重命名
mv spark-env.sh.template spark-env.sh   
``` 

在 `spark-env.sh` 配置文件中，添加 `JAVA_HOME`,`HADOOP_CONF_DIR` 环境变量。 示例如下：  
```bash
export JAVA_HOME=/opt/module/jdk1.8.0_451
export HADOOP_CONF_DIR=/opt/module/hadoop-3.2.0/etc/hadoop
```  

### 添加 S3 依赖 jar   
`这里与 hadoop 配置的 S3 不一样了`， Spark 3.5.5 需要 `hadoop-aws-3.3.4.jar`,`aws-java-sdk-bundle-1.12.730.jar`。 这里需要特别注意是版本不同了。 将它们放入到 /opt/module/spark-3.5.5-bin-hadoop3/jars 下 。     

### 启动 Spark SQL Cli & 查询 S3 Hive 表   
在 Spark 安装目录下执行 `bin/spark-sql` 命令，执行以下 SQL，查询数据。 因为集成了 Hive Metastore，所以我们在 Spark SQL 可以看到上面章节创建的 `yzhou_db`库，`dim_base_province`表。    

```bash
show databases;

select * from yzhou_db.dim_base_province;
```

![hiveons311](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons311.jpg)            

到这里，Spark on Yarn 也查询到 S3 Hive 表了。   

## Kyuubi 提交 Spark SQL 查询 S3 Hive 表      
在 Spark 生态圈里，Kyuubi 是一个出镜率很高的项目了，Kyuubi 是一个分布式和多租户网关，用于在 Lakehouse 上提供 Serverless SQL，可连接包括 Spark、Flink、Hive、JDBC 等引擎，并对外提供 Thrift、Trino 等接口协议供灵活对接。Kyuubi 可提供高可用、服务发现、租户隔离、统一认证、生命周期管理等一系列特性。 

`其实我最喜欢的是它提供了标准的 JDBC 和 ODBC 接口，它架起了 其他服务与 Spark 之间的桥梁，降低了 Spark Job 提交复杂度。` 通用性和易用性都得到很好的体现。    

接下来，介绍 Apache Kyuubi 的部署  

## 部署 Kyuubi 服务   
注意：Kyuubi 部署的节点需要先部署 Spark，上面的章节，已经介绍了 Spark的部署，这里就不再多做赘述。    

访问 `https://www.apache.org/dyn/closer.lua/kyuubi/kyuubi-1.10.2/apache-kyuubi-1.10.2-bin.tgz` 下载 Kyuubi 1.10.2 安装包  

```bash
# 解压并且指定目录
tar -zxvf apache-kyuubi-1.10.2-bin.tgz -C /opt/module  

# 重命名
mv apache-kyuubi-1.10.2-bin  kyuubi-1.10.2-bin 
```

### 修改 conf/kyuubi-env.sh 配置文件  
```bash
# 重命名 kyuubi-env.sh.template 为 kyuubi-env.sh 
mv conf/kyuubi-env.sh.template conf/kyuubi-env.sh   

vim conf/kyuubi-env.sh 
# 内容如下：
export JAVA_HOME=/opt/module/jdk1.8.0_451
export SPARK_HOME=/opt/module/spark-3.5.5-bin-hadoop3-kyuubi  
export HADOOP_CONF_DIR=/opt/module/hadoop-3.2.0/etc/hadoop
```

### 修改 conf/kyuubi-defaults.conf 配置文件  
```bash
# 重命名 kyuubi-defaults.conf.template 为 kyuubi-defaults.conf 
mv conf/kyuubi-defaults.conf.template conf/kyuubi-defaults.conf  

# 在 kyuubi-defaults.conf 配置文件中，添加以下配置  
spark.hadoop.fs.s3a.access.key=minio
spark.hadoop.fs.s3a.secret.key=minio123
spark.hadoop.fs.s3a.endpoint=http://192.168.0.135:9000
spark.hadoop.fs.s3a.impl=org.apache.hadoop.fs.s3a.S3AFileSystem
```  

### 修改 conf/log4j2.xml 配置文件
```bash
# 重命名 log4j2.xml.template 为 log4j2.xml  
mv conf/log4j2.xml.template conf/log4j2.xml
```

### 启动 Kyuubi 服务
```bash
bin/kyuubi start 
```

输出结果如下：  
![hiveons312](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons312.jpg)  

此时，可以浏览器访问 `http://bigdata04:10099` 打开 Kyyuubi Web UI：  
![hiveons313](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons313.jpg)      

### 创建 Kyuubi Engine 
执行 beeline 命令   
注意：jdbc:hive2://bigdata04:10009/ 指向的是 Kyuubi 服务的端口, -n 是指向用户   
```bash
bin/beeline -u 'jdbc:hive2://bigdata04:10009/;#kyuubi.engine.type=SPARK_SQL;spark.master=yarn;spark.submit.deployMode=cluster' -n hadoop
```   

>博主在 Spark 安装目录已经配置了 conf/hive-site.xml, 所以就不需要在命令行添加 metastore 相关参数。     

启动完后，可以看到 `0: jdbc:hive2://bigdata04:10009/>` 待输入的提示符。 示例输出如下：    
```bash
2025-09-26 02:55:13.313 INFO KyuubiSessionManager-exec-pool: Thread-67 org.apache.kyuubi.shaded.zookeeper.ZooKeeper: Session: 0x100002266820001 closed
2025-09-26 02:55:13.316 INFO KyuubiSessionManager-exec-pool: Thread-67 org.apache.kyuubi.operation.LaunchEngine: Processing hadoop's query[e4421b0e-4123-40ec-b614-7564266e42ed]: RUNNING_STATE -> FINISHED_STATE, time taken: 47.994 seconds
Connected to: Spark SQL (version 3.5.5)
Driver: Kyuubi Project Hive JDBC Client (version 1.10.2)
Beeline version 1.10.2 by Apache Kyuubi
0: jdbc:hive2://bigdata04:10009/>        
```

输出结果：  
![hiveons314](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons314.jpg)    

![hiveons315](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons315.jpg)   

此时，我们可以在 Kyuubi Web UI 上看到 Session，Engine 信息。     
`Seesion:`   
![hiveons316](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons316.jpg)   

`Engine:`  
![hiveons317](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons317.jpg)  

注意：当 Cli 终端结束后，Seesion 会立马结束，但是 engine 还会再运行一段时间（默认会存活 30分钟）。      
若需要修改 engine 存活时间，可修改 Kyuubi 配置文件 `kyuubi-defaults.conf`  
```bash
kyuubi.session.idle.timeout.ms=1800000 
```  

在 Yarn Web UI (http://bigdata02:8088/cluster/apps) 可以看到 Spark Application 正在运行。  
![hiveons318](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons318.jpg)  

### 使用 Kyuubi 提交 Spark SQL  
执行 `show databases;`  
![hiveons319](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons319.jpg)     

执行 `select * from yzhou_db.dim_base_province;` 
![hiveons320](http://img.xinzhuxiansheng.com/blogimgs/kyuubi/hiveons320.jpg)   

到这里，该篇介绍的内容就结束了，博主循循渐进的操作完了各种组件可查询 S3 Hive 表。 估计你也可能会想到，后续我们会在改变它们，因为我们不需要 Yarn，不需要 二进制部署。  

因为我们需要在 Kubernetes 上操作它们了。  