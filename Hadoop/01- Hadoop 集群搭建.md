# Hadoop 集群安装  

>hadoop version: 3.1.3 

## Hadoop 高可用集群规划
```bash
vm01           vm02         vm03
NameNode	      NameNode
JournalNode     JournalNode   JournalNode
DataNode        DataNode      DataNode
ZK
                ResourceManager ResourceManager
NodeManager	    NodeManager	  NodeManager
```

## 修改 hostname 
```shell
hostnamectl set-hostname vm01            # 使用这个命令会立即生效且重启也生效
```

## 配置免密操作    
vm01 节点 生成 ssh，并且将 vm01 ssh 分发到 其他节点    
```bash
生成ssh key：
$ ssh-keygen -t rsa  #（每个节点执行，敲三个回车，就会生成两个文件id_rsa（私钥）、id_rsa.pub（公钥））

vm01上操作互信配置，将公钥拷贝到要免密登录的目标机器上。
$ ssh-copy-id -i ~/.ssh/id_rsa.pub vm01
$ ssh-copy-id -i ~/.ssh/id_rsa.pub vm02
$ ssh-copy-id -i ~/.ssh/id_rsa.pub vm03
vm02~vm03 上同上面操作类似，完成互信配置
```


## 下载 Hadoop 安装包 
访问下载地址： `https://archive.apache.org/dist/hadoop/core/hadoop-3.1.3/`


## 将 hadoop-3.1.3.tar.gz上传到 vm01，并解压到/opt/module  
```shell
mkdir -p /opt/module

tar -zxvf hadoop-3.1.3.tar.gz -C /opt/module  
```
tar -zxvf hadoop-3.1.3.tar.gz -C /opt/module  

## 在/opt/module/hadoop-3.1.3 目录下创建 data 文件夹  
```shell
cd /opt/module/hadoop-3.1.3
mkdir data  
```

## 修改配置文件
（1）core-site.xml
vim /opt/module/hadoop-3.1.3/etc/hadoop/core-site.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>

<configuration>

    <!-- 指定NameNode的地址 -->
    <property>
        <name>fs.defaultFS</name>
        <value>hdfs://mycluster</value>
    </property>

    <!-- 指定hadoop数据的存储目录 -->
    <property>
        <name>hadoop.tmp.dir</name>
        <value>/opt/module/hadoop-3.1.3/data/tmp</value>
    </property>

    <!-- 配置HDFS网页登录使用的静态用户为root -->
    <property>
        <name>hadoop.http.staticuser.user</name>
        <value>root</value>
    </property>

    <property>
        <name>ha.zookeeper.quorum</name>
        <value>vm01:2181</value>
    </property>

    <property>
        <name>hadoop.proxyuser.root.hosts</name>
        <value>*</value>
    </property>
    <property>
        <name>hadoop.proxyuser.root.groups</name>
        <value>*</value>
    </property>

    <property>
        <name>hadoop.proxyuser.hive.hosts</name>
        <value>*</value>
    </property>
    <property>
        <name>hadoop.proxyuser.hive.groups</name>
        <value>*</value>
    </property>

</configuration>

```


（2）hdfs-site.xml
vim /opt/module/hadoop-3.1.3/etc/hadoop/hdfs-site.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>

<configuration>
    <!--namenode 数据存放位置-->
    <property>
        <name>dfs.name.dir</name>
        <value>/opt/module/hadoop-3.1.3/data/namenode_data</value>
    </property>

    <!--datanode 数据存放位置-->
    <property>
        <name>dfs.data.dir</name>
        <value>/opt/module/hadoop-3.1.3/data/datanode_data</value>
    </property>

    <!-- 完全分布式集群名称 -->
    <property>
        <name>dfs.nameservices</name>
        <value>mycluster</value>
    </property>

    <!-- 集群中NameNode节点都有哪些 -->
    <property>
        <name>dfs.ha.namenodes.mycluster</name>
        <value>nn1,nn2</value>
    </property>

    <!-- nn1的RPC通信地址 -->
    <property>
        <name>dfs.namenode.rpc-address.mycluster.nn1</name>
        <value>vm01:9000</value>
    </property>

    <!-- nn2的RPC通信地址 -->
    <property>
        <name>dfs.namenode.rpc-address.mycluster.nn2</name>
        <value>vm02:9000</value>
    </property>

    <!-- nn1的http通信地址 -->
    <property>
        <name>dfs.namenode.http-address.mycluster.nn1</name>
        <value>vm01:50070</value>
    </property>

    <!-- nn2的http通信地址 -->
    <property>
        <name>dfs.namenode.http-address.mycluster.nn2</name>
        <value>vm02:50070</value>
    </property>

    <!-- 指定NameNode元数据在JournalNode上的存放位置 -->
    <property>
        <name>dfs.namenode.shared.edits.dir</name>
        <value>qjournal://vm01:8485;vm02:8485;vm03:8485/mycluster</value>
    </property>

    <!-- 指定Journalnode本地磁盘的存放数据位置-->
    <property>
        <name>dfs.journalnode.edits.dir</name>
        <value>/opt/module/hadoop-3.1.3/data/journalnode</value>
    </property>

    <!-- 配置隔离机制，即同一时刻只能有一台服务器对外响应 -->
    <property>
        <name>dfs.ha.fencing.methods</name>
        <value>sshfence</value>
    </property>

    <!-- 使用隔离机制时需要ssh无秘钥登录-->
    <property>
        <name>dfs.ha.fencing.ssh.private-key-files</name>
        <value>/root/.ssh/id_rsa</value>
    </property>

    <!-- 声明journalnode服务器存储目录-->
    <property>
        <name>dfs.journalnode.edits.dir</name>
        <value>/opt/module/hadoop-3.1.3/data/journalnode</value>
    </property>

    <!-- 开启namenode故障转移自动切换-->
    <property>
        <name>dfs.ha.automatic-failover.enabled</name>
        <value>true</value>
    </property>

    <!-- 关闭权限检查-->
    <property>
        <name>dfs.permissions.enable</name>
        <value>false</value>
    </property>

    <!-- 访问代理类：client，mycluster，active配置失败自动切换实现方式-->
    <property>
        <name>dfs.client.failover.proxy.provider.mycluster</name>
        <value>org.apache.hadoop.hdfs.server.namenode.ha.ConfiguredFailoverProxyProvider</value>
    </property>

    <property>
        <name>dfs.webhdfs.enabled</name>
        <value>true</value>
    </property>
    
</configuration>
```

（3）mapred-site.xml
vim /opt/module/hadoop-3.1.3/etc/hadoop/mapred-site.xml

```xml
<?xml version="1.0"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>

<configuration>
    <!--指定mapreduce运行在yarn上-->
    <property>
        <name>mapreduce.framework.name</name>
        <value>yarn</value>
    </property>
    <!--配置任务历史服务器地址-->
    <property>
        <name>mapreduce.jobhistory.address</name>
        <value>vm01:10020</value>
    </property>
    <!--配置任务历史服务器web-ui地址-->
    <property>
        <name>mapreduce.jobhistory.webapp.address</name>
        <value>vm01:19888</value>
    </property>
    <!--开启uber模式-->
    <property>
        <name>mapreduce.job.ubertark.enabled</name>
        <value>true</value>
    </property>

    <property>
        <name>yarn.app.mapreduce.am.env</name>
        <value>HADOOP_MAPRED_HOME=/opt/module/hadoop-3.1.3</value>
    </property>
    <property>
        <name>mapreduce.map.env</name>
        <value>HADOOP_MAPRED_HOME=/opt/module/hadoop-3.1.3</value>
    </property>
    <property>
        <name>mapreduce.reduce.env</name>
        <value>HADOOP_MAPRED_HOME=/opt/module/hadoop-3.1.3</value>
    </property>
</configuration>
```

（4）yarn-site.xml
vim /opt/module/hadoop-3.1.3/etc/hadoop/yarn-site.xml

```xml
<?xml version="1.0"?>
<configuration>

    <!-- Site specific YARN configuration properties -->
    <!--开启yarn高可用-->
    <property>
        <name>yarn.resourcemanager.ha.enabled</name>
        <value>true</value>
    </property>

    <!--指定yarn集群在zookeeper上注册的节点名-->
    <property>
        <name>yarn.resourcemanager.cluster-id</name>
        <value>hayarn</value>
    </property>

    <!--指定俩个ResourceManager的名称-->
    <property>
        <name>yarn.resourcemanager.ha.rm-ids</name>
        <value>rm1,rm2</value>
    </property>

    <!--指定rm1的主机 -->
    <property>
        <name>yarn.resourcemanager.hostname.rm1</name>
        <value>vm02</value>
    </property>

    <!-- 指定rm2的主机-->
    <property>
        <name>yarn.resourcemanager.hostname.rm2</name>
        <value>vm03</value>
    </property>

    <!-- 开始yarn恢复机制-->
    <property>
        <name>yarn.resourcemanager.recovery.enabled</name>
        <value>true</value>
    </property>

    <!-- 配置zookeeper的地址-->
    <property>
        <name>yarn.resourcemanager.zk-address</name>
        <value>vm01:2181,vm02:2181,vm03:2181</value>
    </property>

    <!-- nodemanager获取数据的方法方式-->
    <property>
        <name>yarn.nodemanager.aux-services</name>
        <value>mapreduce_shuffle</value>
    </property>

    <!-- 配置执行ResourceManager恢复机制实现类-->
    <property>
        <name>yarn.resourcemanager.store.class</name>
        <value>org.apache.hadoop.yarn.server.resourcemanager.recovery.ZKRMStateStore</value>
    </property>

    <!-- 指定主resourcemanager的地址-->
    <property>
        <name>yarn.resourcemanager.hostname</name>
        <value>vm03</value>
    </property>

    <property>
        <name>yarn.resourcemanager.webapp.address.rm1</name>
        <value>vm02</value>
    </property>
    <property>
        <name>yarn.resourcemanager.scheduler.address.rm2</name>
        <value>vm03</value>
    </property>
    <property>
        <name>yarn.resourcemanager.webapp.address.rm2</name>
        <value>vm03</value>
    </property>

</configuration>
```


（5）workers
vi /opt/module/hadoop-3.1.3/etc/hadoop/workers

vm01
vm02
vm03


（6）hadoop-env.sh
vi /opt/module/hadoop-3.1.3/etc/hadoop/hadoop-env.sh

export JAVA_HOME=/data/jdk1.8.0_391


（7） sbin/start-dfs.sh、sbin/stop-dfs.sh 添加以下内容 
需放在脚本最上面  

```
HDFS_DATANODE_USER=root
HDFS_DATANODE_SECURE_USER=hdfs
HDFS_NAMENODE_USER=root
HDFS_SECONDARYNAMENODE_USER=root
HDFS_ZKFC_USER=root
HDFS_JOURNALNODE_USER=root
```


（8）start-yarn.sh、stop-yarn.sh添加以下内容
需放在脚本最上面

```
YARN_RESOURCEMANAGER_USER=root
HADOOP_SECURE_DN_USER=yarn
YARN_NODEMANAGER_USER=root
```


## 配置环境变量
vi /etc/profile

export HADOOP_HOME=/opt/module/hadoop-3.1.3
export HADOOP_CONF_DIR=$HADOOP_HOME/etc/hadoop
export PATH=$PATH:$HADOOP_HOME/bin:$HADOOP_HOME/sbin

source /etc/profile

## 分发hadoop安装文件到vm02和vm03
scp -r /opt/module/hadoop-3.1.3 root@vm02:/opt/module/
scp -r /opt/module/hadoop-3.1.3 root@vm03:/opt/module/




8.调整vm01、vm02、vm03的/etc/hosts主机名顺序

192.168.0.201 vm01 
192.168.0.202 vm02 
192.168.0.203 vm03 



9.检查zookeeper是否以及启动，没有启动则启动
# 检查
zk.sh status

# 启动
zk.sh start

10.启动journalnode(vm01、vm02、vm03都执行)
hdfs --daemon start journalnode
jps

11.格式化HDFS（只在vm01上执行）
```bash
hdfs namenode -format  # 注意，需要 3个节点的 journalnode 全部启动成功后，再执行
```

12.启动namenode（只在vm01上执行）
hdfs --daemon start namenode
jps

13.同步元数据 （在vm02执行）
hdfs namenode -bootstrapStandby

14.格式化ZKFC （在vm01执行）
hdfs zkfc -formatZK

15.启动hdfs（在vm01执行）
start-all.sh

# 检查
xcall.sh jps

16.查看两个NameNode的运行状态
hdfs haadmin -getServiceState nn1
hdfs haadmin -getServiceState nn2

17.测试
hadoop fs -ls /
http://vm02:50070

# mapreduce测试
cd /opt/module/hadoop-3.1.3
vi words.txt

hello flink
hello k8s
flink on k8s


hadoop fs -mkdir /tmp
hadoop fs -put words.txt /tmp/words.txt
hadoop jar /opt/module/hadoop-3.1.3/share/hadoop/mapreduce/hadoop-mapreduce-examples-3.1.3.jar wordcount /tmp/words.txt /tmp/wcoutput

# 检查
hadoop fs -ls /tmp
hadoop fs -ls /tmp/wcoutput
hadoop fs -cat /tmp/wcoutput/part-r-00000

18.停止hdfs
stop-all.sh
zk.sh stop


### Q&A 

1.
Caused by: java.lang.NoClassDefFoundError: javax/activation/DataSource
Caused by: java.lang.ClassNotFoundException: javax.activation.DataSource

解决方法：
```
cd ${HADOOP_HOME}/share/hadoop/yarn/lib
wget https://repo1.maven.org/maven2/javax/activation/activation/1.1.1/activation-1.1.1.jar  
``` 

