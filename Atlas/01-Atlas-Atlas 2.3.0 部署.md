# Atlas - 探索 Atlas 2.3.0 部署 & 集成 Hive SQL 

>Atlas version: 2.3.0, Jdk version: 1.8, Hadoop version: 3.2.0, MySQL version: 8.0.23, Hive version: 3.1.3, Hbase version: 2.2.7, Solr version: 8.11.1， Kafka version: 2.4.1, Zookeeper version: 3.6.4

## 引言    
把数据必做`资产`的话，Atlas 就像是它的记录本，记录数据之间的流转逻辑。   

该篇 Blog 主要是介绍 Atlas 2.3.0 以及相关组件的服务部署，并且集成到 Hive SQL，目标是当你在 Hive CLI 执行 Hive SQL 时，例如建表语句，insert 语句时，可以查看到表与表之间的关系。 博主最近的工作内容是完成的是 Kyuubi + Spark SQL 集成到 Atlas 去,先实现表级别，后续是字段级别。针对字段血缘，这里暂时不补充，后面会在 Kyuubi + Spark SQL 的相关 Blog 再做介绍。    

博主所在的团队是 All In Kubernetes （include bigdata servers），对于博主来说，一些复杂的组件要是之前用二进制搭建过，再将其 on Kubernetes 会比较简单，若涉及到一些自定义镜像也是一个不小的工作量。所以该篇 Blog 可以让你了解到 Atlas 配置，其他组件等。  

当然写服务 YAML 时，我必须要感谢 Claude Code。 :）   

下面就开始介绍 Atlas 的部署细节。   

## 环境说明 
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :-------- | :-------- | :--: | :--: | :--: |
| HDFS  | NameNode |  ✓  |    |    |
|       | DataNode |  ✓  |  ✓  |  ✓  |
|       | SecondaryNameNode |    |    |  ✓  |
| Yarn  | Resourcemanager |    |  ✓  |    |
|       | NodeManager |  ✓  |  ✓  |  ✓  |
| HistoryServer  | JobHistoryServer |  ✓  |    |    |
| Zookeeper  | QuorumPeerMain |  ✓  |     |    |
| Kafka      | Kafka          |  ✓  |     |    |
| Hive  | Hive |  ✓  |     |    |
| Hbase  | HMaster |  ✓  |     |    |
|        | HRegionServer  |  ✓  |   ✓  |   ✓ |
|  Solr  | Jar  |  ✓  |   ✓  |   ✓ |  
|  Atlas  | Atlas  |  ✓  |     |     |  
|  MySQL  | MySQL  |  Docker 部署  |     |     |  

## bigdata01 与其他节点配置免密登录     

### 1）创建 hadoop 用户 
1.用 root 启动 hadoop 服务，需要对 sbin/start-dfs.sh, sbin/start-yarn.sh 做一些特殊配置，例如下面配置 (仅是示例)，并且hadoop 服务不推荐使用 root 用户启动。所以下面我们创建 hadoop 用户来启动 hadoop 服务。 

`示例 不推荐使用 root 启动 hadoop`  
```bash
HDFS_DATANODE_USER=root
HDFS_DATANODE_SECURE_USER=hdfs
HDFS_NAMENODE_USER=root
HDFS_SECONDARYNAMENODE_USER=root
```

2.etc/hadoop/core-site.xml 的 `hadoop.proxyuser.hadoop.hosts`,`hadoop.proxyuser.hadoop.groups` 需要跟运行用户互相对应， 格式如下： hadoop.proxyuser.${user}。    

下面我们就创建 hadoop 用户。 

```bash
groupadd hadoop 
useradd -g hadoop hadoop
passwd hadoop
chown -R hadoop:hadoop /opt/module/hadoop-3.2.0
```

>此时，我想你必须有一个意识：无论做任何操作，都应保持 `/opt/module/hadoop-3.2.0` 目录下的文件或者文件夹 都属于 hadoop 用户。 

### 2）.生成 ssh key (所有节点都执行)    
在每个节点都执行以下命令，并且持续 `敲` 回车键即可，如下图所示：  
```bash
ssh-keygen -t rsa
```  
![deploy01](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy01.jpg)       

### 3）.复制公钥到远程服务器 (在 bigdata01, bigdata02 节点操作)
在 bigdata01,bigdata02 节点操作，这里是与下面的 `hadoop.sh` 相呼应。   
```bash
ssh-copy-id -i ~/.ssh/id_rsa.pub bigdata01
ssh-copy-id -i ~/.ssh/id_rsa.pub bigdata02
ssh-copy-id -i ~/.ssh/id_rsa.pub bigdata03
```   

确保 bigdata01，bigdata02 的 hadoop 用户可以免密登录其他节点。  

## 配置 xsync 脚本  

### 1）在全部节点安装 rsync 命令 
```bash
yum install -y rsync
```

### 2）在 bigdata01 节点编写 xsync 脚本 
vim /opt/shell/xsync，使用示例：xsync /path/xxx, 它会将当前文件或者文件夹，传输到其他机器，并且是相同目录。 

脚本内容如下：（`注意根据虚机 hostname 替换脚本中的 host`）
```bash
#!/bin/bash
#1. 判断参数个数
if [ $# -lt 1 ]
then
  echo Not Enough Arguement!
  exit;
fi
#2. 遍历集群所有机器
for host in bigdata01 bigdata02 bigdata03
do
  echo ====================  $host  ====================
  #3. 遍历所有目录，挨个发送
  for file in $@
  do
    #4. 判断文件是否存在
    if [ -e $file ]
    then
      #5. 获取父目录
      pdir=$(cd -P $(dirname $file); pwd)
      #6. 获取当前文件的名称
      fname=$(basename $file)
      ssh $host "mkdir -p $pdir"
      rsync -av $pdir/$fname $host:$pdir
    else
      echo $file does not exists!
    fi
  done
done
``` 

使用 `chmod +x xsync`命令，将其设置为可执行脚本。     

## JDK 安装 & 环境变量配置 & 批量执行 jps 

### 1）JDK 1.8 安装 & 配置环境变量  
下载 jdk1.8 安装包，使用以下命令解压并且指定解压目录,`注意: 不推荐将 JDK 安装在 /root 目录下，假设某个程序使用一般用户运行，默认情况下它是无权限访问 /root 目录下的 JDK，为了避免出现无权限问题，将 JDK 安装到 公共目录。`  
```bash
tar -zxf jdk-8u451-linux-x64.tar.gz -C /opt/module   
```  

在 `/etc/profile.d` 目录下创建 `my_env.sh` 用于设置环境变量，例如 JDK的环境变量设置如下：  
```bash
# java env
JAVA_HOME=/opt/module/jdk1.8.0_202
PATH=$PATH:$JAVA_HOME/bin:$JRE_HOME/bin
CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar:$JRE_HOME/lib
export JAVA_HOME JRE_HOME PATH CLASSPATH
```

再执行 `source /etc/profile` 生效环境变量配置。   

只所以这么做的原因是因为，在后续使用 ssh 跨机器执行 jps 命令时，若环境变量在 /etc/profile 中设置，则 jps 命令无法执行，ssh 跨机器执行 jps，会出现无法加载 /etc/profile 的环境变量。   

>在上面的章节，我们创建 xsync 脚本，现在我们就可以在 bigdata01 节点执行  
`/root/shell/xsync /opt/module/jdk1.8.0_451/`,    
`/root/shell/xsync /etc/profile.d/my_env.sh`。 

### 2）批量执行命令
* 像 hadoop，hbase等这种 Cluster 方式部署，需要多个机器节点，为了 `一个命令` 可同时查看多个机器节点的Java 进程的运行状态。所以有了这样的 `jpsall` 脚本。   

vim /opt/shell/jpsall   
脚本内容如下：
```bash
#!/bin/bash
for host in bigdata01 bigdata02 bigdata03
do
        echo =============== $host ===============
        ssh $host jps $@ | grep -v Jps
done
```
![deploy02](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy02.jpg)    
使用 `chmod +x jpsall`命令，将其设置为可执行脚本。     

* 注意: jpsall 脚本中，我们固定了 ssh $host jps,所以它只能执行 jps，那现在我们想在 bigdata01 节点对`所有节点`同时执行除 jps 以外，可自定义的命令参数。 这种场景也是很常见的。那下面就我们就介绍另一种变体 `xcall` 脚本。       

vim /opt/shell/xcall   
脚本内容如下：  
```bash
#! /bin/bash
for host in bigdata01 bigdata02 bigdata03
do
        echo --------- $i ----------
        ssh $host "$*"
done
``` 
使用 `chmod +x xcall`命令，将其设置为可执行脚本。xcall 脚本后面的执行命令若有空格，记得使用单引号包裹它们。例如：./xcall 'source /etc/profile'。    

>注意根据节点 HostName 调整脚本中的 HostName。  

## Hadoop 3.2.0 安装  
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :-------- | :-------- | :--: | :--: | :--: |
| HDFS  | NameNode |  ✓  |    |    |
|       | DataNode |  ✓  |  ✓  |  ✓  |
|       | SecondaryNameNode |    |    |  ✓  |
| Yarn  | Resourcemanager |    |  ✓  |    |
|       | NodeManager |  ✓  |  ✓  |  ✓  |
| HistoryServer  | JobHistoryServer |  ✓  |    |    |

### 1）下载安装包 & 设置 Hadoop 环境变量
访问 `https://hadoop.apache.org/release/3.2.0.html` 下载 `hadoop-3.2.0.tar.gz`。     
使用 `tar -zxf hadoop-3.2.0.tar.gz -C /opt/module`命令，解压 hadoop 安装包，并且指定解压路径。     

vim /etc/profile.d/my_env.sh ,添加以下内容： 
```bash
#HADOOP HOME
export HADOOP_HOME=/opt/module/hadoop-3.2.0
export PATH=$PATH:$HADOOP_HOME/bin
export PATH=$PATH:$HADOOP_HOME/sbin
```

### 2）修改 etc/hadoop/core-site.xml   
```xml
<!--指定NameNode的地址-->
<property>
    <name>fs.defaultFS</name>
    <value>hdfs://bigdata01:8020</value>
</property>
<!--指定hadoop数据的存储目录-->
<property>
    <name>hadoop.tmp.dir</name>
    <value>/opt/module/hadoop-3.2.0/data</value>
</property>
<property>
    <name>hadoop.proxyuser.hadoop.hosts</name>
    <value>*</value>
</property>
<property>
    <name>hadoop.proxyuser.hadoop.groups</name>
    <value>*</value>
</property>
<property>
    <name>hadoop.proxyuser.root.hosts</name>
    <value>*</value>
</property>
<property>
    <name>hadoop.proxyuser.root.groups</name>
    <value>*</value>
</property>
<!-- 配置HDFS网页登录使用的静态用户为summer-->
<property>
    <name>hadoop.http.staticuser.user</name>
    <value>hadoop</value>
</property>
```

>`注意：hadoop.http.staticuser.user` 参数，若不设置，后续在 Hdfs Web UI 创建文件夹或者上传文件时，可能会提示没有权限。    

### 3）修改 etc/hadoop/hdfs-site.xml
```xml
<!--jnnweb端访问地址-->
<property>
    <name>dfs.namenode.http-address</name>
    <value>bigdata01:9870</value>
</property>
<!--2nn web端访问地址-->
<property>
    <name>dfs.namenode.secondary.http-addresss</name>
    <value>bigdata03:9868</value>
</property>
```

### 4）修改 etc/hadoop/yarn-site.xml 
```xml
<!--指定MR走shuffle-->
<property>
    <name>yarn.nodemanager.aux-services</name>
    <value>mapreduce_shuffle</value>
</property>
<!--指定ResourceManager的地址-->
<property>
    <name>yarn.resourcemanager.hostname</name>
    <value>bigdata02</value>
</property>
<!--环境变量的继承-->
<property>
    <name>yarn.nodemanager.env-whitelist</name>
    <value>JAVA_HOME,HADOOP_COMMON_HOME,HADOOP_HDFS_HOME,HADOOP_COF_DIR,CLASSPATH_PREPEND_DISTCACHE,HADOOP_YARN_HOME,HADOOP_MAPRED_HOME</value>
</property>
<!--yarn容器允许分配的最大最小内存-->
<property>
    <name>yarn.scheduler.minimum-allocation-mb</name>
    <value>512</value>
</property>
<property>
    <name>yarn.scheduler.maximum-allocation-mb</name>
    <value>4096</value>
</property>
<!--yarn容器允许管理的物理内存大小-->
<property>
    <name>yarn.nodemanager.resource.memory-mb</name>
    <value>4096</value>
</property>
<!--关闭yarn对物理内存和虚拟内存的限制检查-->
<property>
    <name>yarn.nodemanager.pmem-check-enabled</name>
    <value>false</value>
</property>
<property>
    <name>yarn.nodemanager.vmem-check-enabled</name>
    <value>false</value>
</property>

<!--开启日志聚集功能-->
<property>
    <name>yarn.log-aggregation-enable</name>
    <value>true</value>
</property>
<!--设置日志聚集服务器地址-->
<property>
    <name>yarn.log.server.url</name>
    <value>http://bigdata01:19888/jobhistory/logs</value>
</property>
<!--设置日志保留时间为7天-->
<property>
    <name>yarn.log-aggregation.retain-seconds</name>
    <value>604800</value>
</property>
```

### 5）修改 etc/hadoop/mapred-site.xml  
```xml
<!--指定MapReduce程序运行在Yarn上-->
<property>
    <name>mapreduce.framework.name</name>
    <value>yarn</value>
</property>
<!--历史服务器端地址-->
<property>
    <name>mapreduce.jobhistory.address</name>
    <value>bigdata01:10020</value>
</property>
<!--历史服务器web端地址-->
<property>
    <name>mapreduce.jobhistory.webapp.address</name>
    <value>bigdata01:19888</value>
</property>
```

此时，Hadoop 服务的配置已经配置完成，需要执行 `/opt/shell/xsync /opt/module/hadoop-3.2.0` 将 hadoop-3.2.0 安装包同步到其他节点上去。  

### 6）配置 etc/hadoop/workers 
vim etc/hadoop/workers ，修改内容如下：  
```bash
bigdata01
bigdata02
bigdata03
```

### 7）在 bigdata01 节点编写hadoop 启动脚本 hadoop.sh
vim /opt/shell/hadoop.sh    

内容如下：  
```bash
#!/bin/bash
if [ $# -lt 1 ]
then
    echo "No Args Input..."
    exit ;
fi
case $1 in
"start")
        echo " =================== 启动 hadoop集群 ==================="

        echo " --------------- 启动 hdfs ---------------"
        ssh bigdata01 "/opt/module/hadoop-3.2.0/sbin/start-dfs.sh"
        echo " --------------- 启动 yarn ---------------"
        ssh bigdata02 "/opt/module/hadoop-3.2.0/sbin/start-yarn.sh"
        echo " --------------- 启动 historyserver ---------------"
        ssh bigdata01 "/opt/module/hadoop-3.2.0/bin/mapred --daemon start historyserver"
;;
"stop")
        echo " =================== 关闭 hadoop集群 ==================="

        echo " --------------- 关闭 historyserver ---------------"
        ssh bigdata01 "/opt/module/hadoop-3.2.0/bin/mapred --daemon stop historyserver"
        echo " --------------- 关闭 yarn ---------------"
        ssh bigdata02 "/opt/module/hadoop-3.2.0/sbin/stop-yarn.sh"
        echo " --------------- 关闭 hdfs ---------------"
        ssh bigdata01 "/opt/module/hadoop-3.2.0/sbin/stop-dfs.sh"
;;
*)
    echo "Input Args Error..."
;;
esac
```

使用 `chmod +x hadoop.sh`命令，将其设置为可执行脚本。     

### 8）格式化 hdfs 存储 (bigdata01)
在 bigdata01 机器上执行 `bin/hadoop namenode -format`, 看到输出的 log 结尾有 `successfully formatted` 字符串，则代表格式化成功。    

```bash
2025-09-17 16:49:55,449 INFO namenode.FSImage: Allocated new BlockPoolId: BP-569273220-192.168.0.161-1758098995441
2025-09-17 16:49:55,460 INFO common.Storage: Storage directory /opt/module/hadoop-3.2.0/data/dfs/name has been successfully formatted.
2025-09-17 16:49:55,464 INFO namenode.FSImageFormatProtobuf: Saving image file /opt/module/hadoop-3.2.0/data/dfs/name/current/fsimage.ckpt_0000000000000000000 using no compression
2025-09-17 16:49:55,519 INFO namenode.FSImageFormatProtobuf: Image file /opt/module/hadoop-3.2.0/data/dfs/name/current/fsimage.ckpt_0000000000000000000 of size 399 bytes saved in 0 seconds .
2025-09-17 16:49:55,531 INFO namenode.NNStorageRetentionManager: Going to retain 1 images with txid >= 0
2025-09-17 16:49:55,537 INFO namenode.NameNode: SHUTDOWN_MSG:
/************************************************************
SHUTDOWN_MSG: Shutting down NameNode at bigdata01/192.168.0.161
************************************************************/
[root@bigdata01 bin]#
```  

### 9）启动 Hadoop 服务  
在 bigdata01 节点，切换至 hadoop 用户，执行 `/opt/shell/hadoop.sh start`脚本即可。  

```bash
# 切换用户
su hadoop 

# 执行
/opt/shell/hadoop.sh start  
```  

输出结果：  
![deploy03](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy03.jpg)   

>特别注意：jpsall 脚本可以帮助我们快速查询到不同节点运行的服务情况，若出现进程没有启动成功，根据 logs 排查 也可以删除所有的 data目录，再重新在 bigdata01 节点格式化目录，再重新执行 `/opt/shell/hadoop.sh start` 命令。   

>这里还需另一个注意：不同用户执行 jps 查看到的进程不同的，所以执行 jpsall 脚本，推荐使用 root 执行，这样查看全部进程。    

### 10）组件 WEB 访问地址 
Hadoop Web UI: http://bigdata01:9870/   
Yarn UI: http://bigdata02:8088/ 
Yarn History UI: http://bigdata01:19888/jobhistory   

## Zookeeper & Kafka 2.4.1 单节点部署  
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :--------  | :-------- | :--: | :--: | :--: |
| Zookeeper  | QuorumPeerMain |  ✓  |     |    |
| Kafka      | Kafka          |  ✓  |     |    |

该篇 Blog 主要介绍 Atlas，所以 Zookeeper 和 Kafka 采用单节点部署，减少成本。    

### 1）Zookeeper 单节点部署 （bigdata01）
```bash
# 解压 & 指定目录
tar -zxf apache-zookeeper-3.6.4-bin.tar.gz -C /opt/module/    

# 重命名
mv apache-zookeeper-3.6.4-bin zookeeper-3.6.4  

# 重命名 zoo.cfg
mv conf/zoo_sample.cfg conf/zoo.cfg  

# 创建数据目录
mkdir data/

# 修改配置 
vim conf/zoo.cfg
# 需要修改的参数如下： 
dataDir=/opt/module/zookeeper-3.6.4/data

# 启动 Zookeeper  
bin/zkServer.sh start
```

### 2）Kafka 单节点部署 （bigdata01）
```bash
# 解压 & 指定目录
tar -zxf kafka_2.13-2.4.1.tgz -C /opt/module/  

# 创建数据目录
mkdir data/

# 修改配置 
vim config/server.properties 
# 需要修改的参数如下： 
log.dirs=/opt/module/kafka_2.13-2.4.1/data
zookeeper.connect=bigdata01:2181/kafka

# 启动 Kafka
bin/kafka-server-start.sh -daemon config/server.properties
```  

## MySQL 8.0.23 部署  
MySQL 8.0.23 采用 docker-compose 部署， 目录结构如下：  
```bash
[root@vm01 MySQL]# ls
docker-compose.yml  init.sh  volumes  
``` 

在 volumes 目录下创建 mysql/mysql-files, mysql/conf, mysql/logs, mysql/data。  

下面是 docker-compose.yml 内容：  
```yml
[root@vm01 MySQL]# cat docker-compose.yml
version: '3.7'
services:
  mysql:
    image: mysql:8.0.23
    container_name: mysql
    restart: always
    environment:
      MYSQL_ROOT_PASSWORD: "123456"
      TZ: Asia/Shanghai
    ports:
      - "3306:3306"
    deploy:
      resources:
        limits:
          memory: 1G
    volumes:
      - ./volumes/mysql/mysql-files:/var/lib/mysql-files
      - ./volumes/mysql/conf:/etc/mysql/conf.d
      - ./volumes/mysql/logs:/var/log/mysql
      - ./volumes/mysql/data:/var/lib/mysql
```  

`init.sh` 脚本的内容： 
```bash
[root@vm01 MySQL]# cat init.sh
#!/bin/bash

# 定义容器名称和 MySQL Root 密码
CONTAINER_NAME="mysql"
MYSQL_ROOT_PASSWORD="123456"
NEW_PASSWORD="123456"

# 进入 MySQL 容器并执行 MySQL 命令
docker exec -i $CONTAINER_NAME bash <<EOF
mysql -u root -p$MYSQL_ROOT_PASSWORD <<EOSQL
USE mysql;
SELECT host, user, plugin, authentication_string FROM mysql.user;
ALTER USER 'root'@'%' IDENTIFIED WITH mysql_native_password BY '$NEW_PASSWORD';
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '$NEW_PASSWORD';
FLUSH PRIVILEGES;
EOSQL
EOF

echo "MySQL 用户认证插件已更改，并已应用新的密码。"
```  

MySQL启动成功后，再执行 init.sh 教程，设置  MySQL 密码。    
打开 DataGrid 工具，提前创建好 Hive 的 MySQL 数据库，例如库名： `hive_metastore`。   

## Hive 3.1.3 部署   
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :--------  | :-------- | :--: | :--: | :--: |
| Hive  | Hive |  ✓  |     |    |

```bash
# 下载 hive 3.1.3 安装包
wget https://archive.apache.org/dist/hive/hive-3.1.3/apache-hive-3.1.3-bin.tar.gz

# 解压并且指定目录
tar -zxf apache-hive-3.1.3-bin.tar.gz -C /opt/module/  

# 重命名
mv /opt/module/apache-hive-3.1.3-bin/ /opt/module/hive-3.1.3   
```

### 1）配置 Hive 环境变量    
vim /etc/profile.d/my_env.sh, 配置 HIVE_HOME 环境变量。  

```bash
#HIVE_HOME
export HIVE_HOME=/opt/module/hive-3.1.3
export PATH=$PATH:$HIVE_HOME/bin
```

### 2）根据模板创建 hive 配置文件 
```bash
mv conf/hive-default.xml.template conf/hive-site.xml   
mv conf/hive-env.sh.template conf/hive-env.sh  
```

### 3）修改 conf/hive-site.xml   
```xml
<!--jdbc连接的URL-->
<property>
    <name>javax.jdo.option.ConnectionURL</name>
    <value>jdbc:mysql://192.168.0.201:3306/hive_metastore?useSSL=false&amp;useUnicode=true&amp;characterEncoding=UTF-8</value>
</property>
<!--jdbc连接的Driver-->
<property>
    <name>javax.jdo.option.ConnectionDriverName</name>
    <value>com.mysql.jdbc.Driver</value>
</property>
<!--jdbc连接的username-->
<property>
    <name>javax.jdo.option.ConnectionUserName</name>
    <value>root</value>
</property>
<!--jdbc连接的password-->
<property>
    <name>javax.jdo.option.ConnectionPassword</name>
    <value>123456</value>
</property>
<!--Hive默认在HDFS的工作目录-->
<property>
    <name>hive.metastore.warehouse.dir</name>
    <value>/user/hive/warehouse</value>
</property>
<!--指定hiveserver2连接的端口号-->
<property>
    <name>hive.server2.thrift.port</name>
    <value>10000</value>
</property>
<!--指定hiveserver2连接的host-->
<property>
    <name>hive.server2.thrift.bind.host</name>
    <value>bigdata01</value>
</property>
<!--指定存储元数据要连接的地址-->
<!--
<property>
    <name>hive.metastore.uris</name>
    <value>thrift://bigdata01:9083</value>
</property>
-->
<!--元数据存储授权-->
<property>
    <name>hive.metastore.event.db.notification.aapi.auth</name>
    <value>false</value>
</property>
<!--Hive元数据存储版本的验证-->
<property>
    <name>hive.metastore.schema.verification</name>
    <value>false</value>
</property>
<!--hiveserver2的高可用参数,开启此参数可以提高hive:server2的启动速度-->
<property>
    <name>hive.server2.active.passive.ha.enable</name>
    <value>true</value>
</property>
```  

### 4）初始化 Hive MySQL 元数据 
在 hive-3.1.3/lib 目录下添加 `mysql-connector-java` 驱动包。    
```bash
wget https://repo1.maven.org/maven2/mysql/mysql-connector-java/8.0.30/mysql-connector-java-8.0.30.jar 
```

```shell
bin/schematool -initSchema -dbType mysql -verbose    
``` 

输出的结果示例：   
```bash 

# 省略部分 log

0: jdbc:mysql://192.168.0.201:3306/hive_metas> /*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */
No rows affected (0.001 seconds)
0: jdbc:mysql://192.168.0.201:3306/hive_metas> /*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */
No rows affected (0.001 seconds)
0: jdbc:mysql://192.168.0.201:3306/hive_metas> /*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */
No rows affected (0.001 seconds)
0: jdbc:mysql://192.168.0.201:3306/hive_metas> !closeall
Closing: 0: jdbc:mysql://192.168.0.201:3306/hive_metastore?useSSL=false&useUnicode=true&characterEncoding=UTF-8
beeline>
beeline> Initialization script completed
schemaTool completed
[root@bigdata01 hive-3.1.3]#
``` 

查看 MySQL DB 情况，如下图所示：  
![deploy04](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy04.jpg)    

### 5）启动 Hive CLI 
在 bigdata01 的 hive 目录下使用 root 用户执行 `bin/hive`, 此时你可能会看到如下异常信息：  
```bash
Caused by: org.apache.hadoop.ipc.RemoteException(org.apache.hadoop.security.AccessControlException): Permission denied: user=root, access=EXECUTE, inode="/tmp":hadoop:supergroup:drwxrwx---
``` 

可以使用 hadoop 用户将 /tmp 权限设置为 777,命令如下：  
```bash
# 切换到 hadoop用户
su hadoop 

# 设置 /tmp 目录权限  
hdfs dfs -chmod 777 /tmp
```

再执行 bin/hive 命令即可。   
![deploy05](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy05.jpg)    

>注意：在后续的测试过程中，可能还会存在路径权限问题，可在 `/etc/profile.d/my_env.sh` 中设置 `export HADOOP_USER_NAME=hadoop`，使用 source 命令生效后，再重新启动 Hive Cli。      

### 6）示例演示  
在 Hive Cli 执行下面建表语句，再执行 `show tables;` 可查看表列表，使用 `show create table test_user;` 查看建表语句。  
建表示例 SQL 
```sql
CREATE TABLE test_user (
`id` STRING COMMENT '编号',
`name` STRING COMMENT '姓名',
`province_id` STRING COMMENT '省份 ID',
`province_name` STRING COMMENT '省份名称'
) COMMENT '用户表'
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\t';
```   
示例执行结果：   
![deploy06](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy06.jpg)     

输入 `insert into table test_user values('1','zhangsan','001','北京');` 向 test_user 表中插入一条数据。  

我们可以从 Yarn Web UI 找到该SQL 执行的痕迹：  
![deploy15](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy15.jpg)   

输入 Select 语句查询数据。   
```bash
hive> select * from test_user;
OK
1       zhangsan        001     北京
Time taken: 0.097 seconds, Fetched: 1 row(s)
hive>
```  

### 7）发现中文乱码 
在 Hive CLi 输入 `desc test_user`  查看表信息时， 发现注释列乱码， 如下所示：  
```bash
hive> desc test_user;
OK
id                      string                  ??
name                    string                  ??
province_id             string                  ?? ID
province_name           string                  ????
Time taken: 0.058 seconds, Fetched: 4 row(s)
hive>
```  

我们来解决它：  
在 hive 的 MySQL 数据库中执行以下 SQL ：  
```sql
#修改字段注释字符集
alter table COLUMNS_V2 modify column COMMENT varchar(256) character set utf8;
#修改表注释字符集
alter table TABLE_PARAMS modify column PARAM_VALUE varchar(20000) character set utf8;
#修改分区参数,支持分区建用中文表示
alter table PARTITION_PARAMS modify column PARAM_VALUE varchar(20000) character set utf8;
alter table PARTITION_KEYS modify column PKEY_COMMENT varchar(20000) character set utf8;
#修改索引名注释,支持中文表示
alter table INDEX_PARAMS modify column PARAM_VALUE varchar (4000) character set utf8;
#修改视图,支持视图中文
ALTER TABLE TBLS modify COLUMN VIEW_EXPANDED_TEXT mediumtext CHARACTER SET utf8;
ALTER TABLE TBLS modify COLUMN VIEW_ORIGINAL_TEXT mediumtext CHARACTER SET utf8;
```

将上面的 test_user 表删除，再创建，就可以看到中文了：  
输出结果：  
```bash
OK
id                      string                  编号
name                    string                  姓名
province_id             string                  省份 ID
province_name           string                  省份名称
Time taken: 0.035 seconds, Fetched: 4 row(s)
```


## Hbase 2.2.7 部署 
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :--------  | :-------- | :--: | :--: | :--: |
| Hbase  | HMaster |  ✓  |     |    |
|        | HRegionServer  |  ✓  |   ✓  |   ✓ |

```bash
# 下载 hbase 2.2.7 安装包
wget https://archive.apache.org/dist/hbase/2.2.7/hbase-2.2.7-bin.tar.gz  

# 解压并指定目录
tar -zxf hbase-2.2.7-bin.tar.gz -C /opt/module
```  

### 1）配置 Hbase 环境变量 
vim /etc/profile.d/my_env.sh, 配置 HBASE_HOME 环境变量。  

```bash
#HBASE_HOME
export HBASE_HOME=/opt/module/hbase-2.2.7
export PATH=$PATH:$HBASE_HOME/bin
```

### 2）修改 conf/hbase-env.sh 配置文件  
将 `HBASE_MANAGES_ZK` 参数改成 false，设置 `HBASE_DISABLE_HADOOP_CLASSPATH_LOOKUP`为 true， 如下所示：  
```bash
# Tell HBase whether it should manage it's own instance of ZooKeeper or not.
export HBASE_MANAGES_ZK=false
export HBASE_DISABLE_HADOOP_CLASSPATH_LOOKUP="true"
```

### 3）修改 conf/hbase-site.xml 配置文件   
给 Hbase 的 Zookeeper Path 添加了 `hbase` 父节点。避免出现 Zookeeper 数据无法管理。 
```bash
<property>
    <name>hbase.rootdir</name>
    <value>hdfs://bigdata01:8020/HBase</value>
</property>
<property>
    <name>hbase.cluster.distributed</name>
    <value>true</value>
</property>
<property>
    <name>hbase.zookeeper.quorum</name>
    <value>bigdata01</value>
</property>
<property>
    <name>hbase.zookeeper.property.clientPort</name>
    <value>2181</value>
</property>
<property>
    <name>zookeeper.znode.parent</name>
    <value>/hbase</value>
</property>
``` 

### 4）修改 conf/regionservers 配置文件  
```bash
bigdata01
bigdata02
bigdata03
``` 

### 5）使用 xsync 工具将 hbase-2.2.7 目录 和 /etc/profile.d/my_env.sh 
使用 xsync 脚本将 hbase-2.2.7 目录 和 /etc/profile.d/my_env.sh 分发到其他节点，再 使用 xcall 'source /etc/profile' 生效 HBASE_HOME 环境变量。    

### 6）启动 Hbase 
在 bigdata01 节点的 Hbase 目录下，执行 `bin/start-hbase.sh` 启动 Hbase Cluster。  

输出结果：  
![deploy07](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy07.jpg)   

### 7）组件 WEB 访问地址 
Hbase Web UI：http://bigdata01:16010/


## Solr 8.11.1 部署 
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :--------  | :-------- | :--: | :--: | :--: |
|  Solr  | Jar  |  ✓  |   ✓  |   ✓ |  

```bash
# 下载 solr 8.11.1 安装包  
wget https://archive.apache.org/dist/solr/solr/8.11.1/solr-8.11.1.tgz  

# 解压并指定目录
tar -zxf solr-8.11.1.tgz -C /opt/module/
```

### 1）创建 solr 用户 
创建 solr 用户。  
```bash
groupadd solr
useradd -g solr solr
passwd solr
chown -R solr:solr /opt/module/solr-8.11.1
``` 
### 2）修改 bin/solr.in.sh 配置文件 
修改 ZK_HOST 参数，这里需要特别注意：因为ZK_HOST 配置了 znode parent 为 solr ，`所以需要提前创建好 Zookeeper path`。  

如下所示：  
```bash  
ZK_HOST="bigdata01:2181/solr" 
```

### 3）使用 xsync 工具将 solr-8.11.1 目录拷贝到其他节点  

### 4）启动 Solr   
我创建了 solr 用户，但是我的 shell 是 root 用户，所以我希望启动 solr 的时候，带上 用户信息，需要在`所有节点上`执行以下命令：  
```bash
sudo -i -u solr /opt/module/solr-8.11.1/bin/solr start
```

### 5）组件 WEB 访问地址  
Solr Web UI：http://bigdata01:8983/    
![deploy08](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy08.jpg)  

## Atlas 2.3.0 部署   
| 服务名称      | 子服务 | 服务器 bigdata01  | 服务器 bigdata02  | 服务器bigdata03  |
| :--------  | :-------- | :--: | :--: | :--: |
|  Atlas  | Atlas  |  ✓  |     |     |  

从 Atlas 官网可以了解到，Atlas 是不提供安装包，需要我们自行下载源码进行编译打包成安装。   

### 1）下载源码 & 编译打包 
从 `https://atlas.apache.org/#/Downloads` 下载 2.3.0 版本的源码， 在已经存在的 Maven 环境中执行 `mvn clean -DskipTests package -Pdist` 命令进行打包。  

>博主是在 Linux 环境进行编译打包，这样可以省去一些脚本 (py,sh) 需要执行 dos2unix。    

下面是打包的示例结果：  
![deploy09](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy09.jpg)   

最终的打包产物会在 `distro/target` 目录下。    
```bash
[root@vm01 apache-atlas-sources-2.3.0]# ls -l distro/target/
total 1036684
-rw-r--r--. 1 root root     29312 Sep 20 22:58 apache-atlas-2.3.0-atlas-index-repair.zip
-rw-r--r--. 1 root root 509947394 Sep 20 22:58 apache-atlas-2.3.0-bin.tar.gz
-rw-r--r--. 1 root root     29407 Sep 20 22:58 apache-atlas-2.3.0-classification-updater.zip
-rw-r--r--. 1 root root  10648866 Sep 20 22:58 apache-atlas-2.3.0-falcon-hook.tar.gz
-rw-r--r--. 1 root root  14591704 Sep 20 22:58 apache-atlas-2.3.0-hbase-hook.tar.gz
-rw-r--r--. 1 root root  12645619 Sep 20 22:58 apache-atlas-2.3.0-hive-hook.tar.gz
-rw-r--r--. 1 root root  12616269 Sep 20 22:58 apache-atlas-2.3.0-impala-hook.tar.gz
-rw-r--r--. 1 root root  12894980 Sep 20 22:58 apache-atlas-2.3.0-kafka-hook.tar.gz
-rw-r--r--. 1 root root 400537602 Sep 20 22:58 apache-atlas-2.3.0-server.tar.gz
-rw-r--r--. 1 root root  15910561 Sep 20 22:58 apache-atlas-2.3.0-sources.tar.gz
-rw-r--r--. 1 root root  10635890 Sep 20 22:58 apache-atlas-2.3.0-sqoop-hook.tar.gz
-rw-r--r--. 1 root root  60932000 Sep 20 22:58 apache-atlas-2.3.0-storm-hook.tar.gz
drwxr-xr-x. 2 root root         6 Sep 20 22:58 archive-tmp
-rw-r--r--. 1 root root    101914 Sep 20 22:58 atlas-distro-2.3.0.jar
drwxr-xr-x. 2 root root      4096 Sep 20 22:58 bin
drwxr-xr-x. 5 root root      4096 Sep 20 22:58 conf
drwxr-xr-x. 2 root root        28 Sep 20 22:58 maven-archiver
drwxr-xr-x. 3 root root        22 Sep 20 22:58 maven-shared-archive-resources
drwxr-xr-x. 2 root root        55 Sep 20 22:58 META-INF
-rw-r--r--. 1 root root      4220 Sep 20 22:58 rat.txt
drwxr-xr-x. 3 root root        22 Sep 20 22:58 test-classes
[root@vm01 apache-atlas-sources-2.3.0]#
```

我们将 `apache-atlas-2.3.0-server.tar.gz, apache-atlas-2.3.0-hive-hook.tar.gz` 拷贝到 bigdata01 节点，并且对它们进行解压到 /opt/module 目录下。   

```bash
# 解压并且指定目录 
tar -zxf apache-atlas-2.3.0-server.tar.gz -C /opt/module/
# 重命名
mv apache-atlas-2.3.0/ atlas-2.3.0

# 解压并且指定目录
tar -zxf apache-atlas-2.3.0-hive-hook.tar.gz -C /opt/module 
# 重命名
mv apache-atlas-hive-hook-2.3.0/ atlas-hive-hook-2.3.0/
``` 

### 2）修改 conf/atlas-env.sh 配置文件 
在 atlas-env.sh 配置文件中添加 `HBASE_CONF_DIR` 路径。   
```bash
export HBASE_CONF_DIR=/opt/module/hbase/conf
```

### 2）修改 conf/atlas-application.properties 配置文件，接入 hbase，solr，kafka 组件 

vim conf/atlas-application.properties ， 需要修改：   
```bash
atlas.graph.storage.hostname= 指向 hbase 的 zk 地址 
atlas.graph.index.search.solr.zookeeper-url 指向 solr 的 Zookeeper 地址
atlas.notification.embedded= 设置成 false
atlas.kafka.data=${sys:atlas.home}/data/kafka  该行配置可以注释掉 
atlas.kafka.zookeeper.connect=localhost:9026  指向 kafka 的 Zookeeper 地址 
atlas.kafka.bootstrap.servers=localhost:9027  指向 Kafka 的 Broker 地址 
atlas.rest.address=http://localhost:21000 atlas rest api 地址 
atlas.audit.hbase.zookeeper.quorum=localhost:2181 指向 habse 的 Zookeeper 地址 
```

配置示例：   (`hbase 的 znode parent 不填写`，博主有些搞不懂为啥 hbase 不可以直接填写 znode parent 部分)    
```bash
atlas.graph.storage.hostname=bigdata01:2181
atlas.graph.index.search.solr.zookeeper-url=bigdata01:2181/solr

#########  Notification Configs  #########
atlas.notification.embedded=false
#atlas.kafka.data=${sys:atlas.home}/data/kafka
atlas.kafka.zookeeper.connect=bigdata01:2181/kafka
atlas.kafka.bootstrap.servers=bigdata01:9092

atlas.rest.address=http://bigdata01:21000

atlas.audit.hbase.zookeeper.quorum=bigdata01:2181
``` 

### 3）Atlas 集成 Hive SQL  
* 将以下配置，添加到 conf/atlas-application.properties 配置文件中 
```bash
######### Hive Hook Configs #######
atlas.hook.hive.synchronous=false
atlas.hook.hive.numRetries=3
atlas.hook.hive.queueSize=10000
atlas.cluster.name=primary
```  

* 修改 hive 目录下的 conf/hive-env.sh 配置文件  
将 `HIVE_AUX_JARS_PATH` 参数指向 atlas hive hook 目录。如下所示：      
```bash
export HIVE_AUX_JARS_PATH=/opt/module/atlas-hive-hook-2.3.0/hook/hive  
```

* 将 atlas 目录下 conf/atlas-application.properties 配置文件拷贝 hive 安装目录下的 `/opt/module/hive-3.1.3/conf/`    

### 4）在 Solr 创建 3个 index  
首先切换到 solr 用户，再执行以下命令：   
```bash
bin/solr create -c vertex_index -shards 3 -replicationFactor 2
bin/solr create -c edge_index -shards 3 -replicationFactor 2
bin/solr create -c fulltext_index -shards 3 -replicationFactor 2
```
![deploy10](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy10.jpg)  
  
### 5）启动 Atlas 
在 bigdata01 节点的 Atlas目录下执行 `bin/atlas_start.py` 启动 Atlas。     

输出结果：  
![deploy13](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy13.jpg)

### 6）组件 WEB 地址 
Atlas Web UI：http://bigdata01:21000/   
![deploy14](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy14.jpg)   

### 7）启动 Atlas 遇到的问题  
atlas 启动过程并不是一帆风顺的， 下面列举：    

#### 在启动过程中可能会遇到 `HBASE_CONF_DIR` 关键字问题，如下图所示：    
![deploy11](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy11.jpg)     

这里将 `/etc/profile.d/my_env.sh` 添加 `export HBASE_CONF_DIR=/opt/module/hbase-2.2.7/conf`。    

#### java.util.zip.ZipException: error in opening zip file  
该异常意识因为 Maven打包的jar ，并不完整，进入 atlas 目录下的 `server/webapp/atlas/WEB-INF/lib`，你会发现 `org.restlet.ext.servlet.jar`,`org.restlet.jar` 文件大小有问题。     

![deploy12](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy12.jpg)   
 
atlas.?-?.out 文件内容：       
```bash
2025-09-21 00:49:47,099 WARN  - [main:] ~ Failed startup of context o.e.j.w.WebAppContext@62dae245{/,file:///opt/module/atlas-2.3.0/server/webapp/atlas/,UNAVAILABLE}{/opt/module/atlas-2.3.0/server/webapp/atlas} (WebAppContext:533)
java.util.zip.ZipException: error in opening zip file
        at java.util.zip.ZipFile.open(Native Method)
        at java.util.zip.ZipFile.<init>(ZipFile.java:231)
        at java.util.zip.ZipFile.<init>(ZipFile.java:156)
        at java.util.jar.JarFile.<init>(JarFile.java:168)
        at java.util.jar.JarFile.<init>(JarFile.java:105)
        at sun.net.www.protocol.jar.URLJarFile.<init>(URLJarFile.java:93)
        at sun.net.www.protocol.jar.URLJarFile.getJarFile(URLJarFile.java:69)
        at sun.net.www.protocol.jar.JarFileFactory.get(JarFileFactory.java:162)
        at sun.net.www.protocol.jar.JarURLConnection.connect(JarURLConnection.java:128)
        at sun.net.www.protocol.jar.JarURLConnection.getJarFile(JarURLConnection.java:89)
        at org.eclipse.jetty.webapp.MetaInfConfiguration.getTlds(MetaInfConfiguration.java:445)
        at org.eclipse.jetty.webapp.MetaInfConfiguration.scanForTlds(MetaInfConfiguration.java:361)
        at org.eclipse.jetty.webapp.MetaInfConfiguration.scanJars(MetaInfConfiguration.java:172)
        at org.eclipse.jetty.webapp.MetaInfConfiguration.preConfigure(MetaInfConfiguration.java:106)
        at org.eclipse.jetty.webapp.WebAppContext.preConfigure(WebAppContext.java:488)
        at org.eclipse.jetty.webapp.WebAppContext.doStart(WebAppContext.java:523)
        at org.eclipse.jetty.util.component.AbstractLifeCycle.start(AbstractLifeCycle.java:72)
        at org.eclipse.jetty.util.component.ContainerLifeCycle.start(ContainerLifeCycle.java:169)
        at org.eclipse.jetty.server.Server.start(Server.java:408)
        at org.eclipse.jetty.util.component.ContainerLifeCycle.doStart(ContainerLifeCycle.java:110)
        at org.eclipse.jetty.server.handler.AbstractHandler.doStart(AbstractHandler.java:97)
        at org.eclipse.jetty.server.Server.doStart(Server.java:372)
        at org.eclipse.jetty.util.component.AbstractLifeCycle.start(AbstractLifeCycle.java:72)
        at org.apache.atlas.web.service.EmbeddedServer.start(EmbeddedServer.java:110)
        at org.apache.atlas.Atlas.main(Atlas.java:133)
```

安装包里面的异常jar 
```bash
[root@bigdata01 lib]# ls -lt |grep restlet
-rw-r--r--. 1 root root      8011 Jul 11 23:22 org.restlet-2.4.3.jar
-rw-r--r--. 1 root root      8011 Jul 11 23:22 org.restlet.ext.servlet-2.4.3.jar
```

正确的 jar
```bash
[root@bigdata01 atlas-2.3.0]# ls -lt |grep restlet
-rw-r--r--. 1 root root  23350 Sep 21 01:44 org.restlet.ext.servlet.jar
-rw-r--r--. 1 root root 715773 Sep 21 01:44 org.restlet.jar
```

所以，我们需要将 异常的jar 删除，；并且将正确的 jar再 拷贝到 lib 目录去。     



## Hive 元数据导入 & Hive CLI 验证 Atlas 
进入 `atlas-hive-hook-2.3.0` 文件夹的 hook-bin 目录，执行 ./import-hive.sh 脚本。  

输出结果： 
```bash
[root@bigdata01 hook-bin]# ./import-hive.sh
Using Hive configuration directory [/opt/module/hive-3.1.3/conf]
Log file for import is /var/log/atlas/import-hive.log
log4j:WARN No such property [maxFileSize] in org.apache.log4j.PatternLayout.
log4j:WARN No such property [maxBackupIndex] in org.apache.log4j.PatternLayout.
Enter username for atlas :- admin
Enter password for atlas :-
Loading class `com.mysql.jdbc.Driver'. This is deprecated. The new driver class is `com.mysql.cj.jdbc.Driver'. The driver is automatically registered via the SPI and manual loading of the driver class is generally unnecessary.
Hive Meta Data imported successfully!
```

在上面的章节中，我们已经将 Atlas 集成到了 Hive ，现在我们启动 Hive Cli 执行 建表语句，多表查询时，Atlas 又是如何展示的。   

示例 SQL 如下：(该示例 摘自尚硅谷的 Atlas视频教程, 示例表即使没有数据也不影响血缘关系的展示)   
```bash
CREATE TABLE dwd_order_info (
 `id` STRING COMMENT '订单号',
 `final_amount` DECIMAL(16,2) COMMENT '订单最终金额',
 `order_status` STRING COMMENT '订单状态',
 `user_id` STRING COMMENT '用户 id',
 `payment_way` STRING COMMENT '支付方式',
 `delivery_address` STRING COMMENT '送货地址',
 `out_trade_no` STRING COMMENT '支付流水号',
 `create_time` STRING COMMENT '创建时间',
 `operate_time` STRING COMMENT '操作时间',
 `expire_time` STRING COMMENT '过期时间',
 `tracking_no` STRING COMMENT '物流单编号',
 `province_id` STRING COMMENT '省份 ID',
 `activity_reduce_amount` DECIMAL(16,2) COMMENT '活动减免金额',
 `coupon_reduce_amount` DECIMAL(16,2) COMMENT '优惠券减免金额',
 `original_amount` DECIMAL(16,2) COMMENT '订单原价金额',
 `feight_fee` DECIMAL(16,2) COMMENT '运费',
 `feight_fee_reduce` DECIMAL(16,2) COMMENT '运费减免'
) COMMENT '订单表'
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\t';


CREATE TABLE dim_base_province (
 `id` STRING COMMENT '编号',
 `name` STRING COMMENT '省份名称',
 `region_id` STRING COMMENT '地区 ID',
 `area_code` STRING COMMENT '地区编码',
 `iso_code` STRING COMMENT 'ISO-3166 编码，供可视化使用',
 `iso_3166_2` STRING COMMENT 'IOS-3166-2 编码，供可视化使用'
) COMMENT '省份表'
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\t';


CREATE TABLE `ads_order_by_province` (
 `dt` STRING COMMENT '统计日期',
 `province_id` STRING COMMENT '省份 id',
 `province_name` STRING COMMENT '省份名称',
 `area_code` STRING COMMENT '地区编码',
 `iso_code` STRING COMMENT '国际标准地区编码',
 `iso_code_3166_2` STRING COMMENT '国际标准地区编码',
 `order_count` BIGINT COMMENT '订单数',
 `order_amount` DECIMAL(16,2) COMMENT '订单金额'
) COMMENT '各省份订单统计'
ROW FORMAT DELIMITED FIELDS TERMINATED BY '\t';

insert into table ads_order_by_province
select
 '2021-08-30' dt,
 bp.id,
 bp.name,
 bp.area_code,
 bp.iso_code,
 bp.iso_3166_2,
 count(*) order_count,
 sum(oi.final_amount) order_amount
from dwd_order_info oi
left join dim_base_province bp
on oi.province_id=bp.id
group by bp.id,bp.name,bp.area_code,bp.iso_code,bp.iso_3166_2;
```  

然后它的血缘关系如下：  
![deploy16](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy16.jpg)  


## 遗留问题
Atlas 实践过程中发现， 当 ENTITIES 选择 Basic 或 选择 Advanced 时，展示的列表数量不一致。这让我有些摸不着头脑，后续有发现问题，再跟大伙同步。  
![deploy17](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy17.jpg)  

![deploy18](http://img.xinzhuxiansheng.com/blogimgs/atlas/deploy18.jpg)  

refer           
1.【尚硅谷】大数据技术之Atlas数据治理 https://www.bilibili.com/video/BV1jA411F76d/?spm_id_from=333.788.videopod.episodes&vd_source=89b3f4bd088c6355f40a00df74cf8ffd       