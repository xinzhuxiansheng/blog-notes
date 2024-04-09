## Flink on K8s Hadoop 集群安装 

>前置工作，需将 k8s01,k8s02,k8s03 都配置ssh 免密登录  
注意.ssh 权限 

```
chmod 700 /home/user/.ssh
chmod 600 /home/user/.ssh/authorized_keys 
```

1.Hadoop高可用集群规划
k8s01           k8s02         k8s03
NameNode	      NameNode
JournalNode     JournalNode   JournalNode
DataNode        DataNode      DataNode
ZK
                ResourceManager ResourceManager
NodeManager	    NodeManager	  NodeManager
 
2.从网盘下Hadoop安装包


3.将hadoop-3.1.3.tar.gz上传到k8s01，并解压到/opt/module 
tar -zxvf hadoop-3.1.3.tar.gz -C /opt/module  

4.在/opt/module/hadoop-3.1.3目录下创建data文件夹  
cd /opt/module/hadoop-3.1.3

mkdir data  

5.修改配置文件
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
        <value>k8s01:2181,k8s02:2181,k8s03:2181</value>
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
        <value>k8s01:9000</value>
    </property>

    <!-- nn2的RPC通信地址 -->
    <property>
        <name>dfs.namenode.rpc-address.mycluster.nn2</name>
        <value>k8s02:9000</value>
    </property>

    <!-- nn1的http通信地址 -->
    <property>
        <name>dfs.namenode.http-address.mycluster.nn1</name>
        <value>k8s01:50070</value>
    </property>

    <!-- nn2的http通信地址 -->
    <property>
        <name>dfs.namenode.http-address.mycluster.nn2</name>
        <value>k8s02:50070</value>
    </property>

    <!-- 指定NameNode元数据在JournalNode上的存放位置 -->
    <property>
        <name>dfs.namenode.shared.edits.dir</name>
        <value>qjournal://k8s01:8485;k8s02:8485;k8s03:8485/mycluster</value>
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
        <value>k8s01:10020</value>
    </property>
    <!--配置任务历史服务器web-ui地址-->
    <property>
        <name>mapreduce.jobhistory.webapp.address</name>
        <value>k8s01:19888</value>
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
        <value>k8s02</value>
    </property>

    <!-- 指定rm2的主机-->
    <property>
        <name>yarn.resourcemanager.hostname.rm2</name>
        <value>k8s03</value>
    </property>

    <!-- 开始yarn恢复机制-->
    <property>
        <name>yarn.resourcemanager.recovery.enabled</name>
        <value>true</value>
    </property>

    <!-- 配置zookeeper的地址-->
    <property>
        <name>yarn.resourcemanager.zk-address</name>
        <value>k8s01:2181,k8s02:2181,k8s03:2181</value>
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
        <value>k8s03</value>
    </property>

    <property>
        <name>yarn.resourcemanager.webapp.address.rm1</name>
        <value>k8s02</value>
    </property>
    <property>
        <name>yarn.resourcemanager.scheduler.address.rm2</name>
        <value>k8s03</value>
    </property>
    <property>
        <name>yarn.resourcemanager.webapp.address.rm2</name>
        <value>k8s03</value>
    </property>

</configuration>
```


（5）workers
vi /opt/module/hadoop-3.1.3/etc/hadoop/workers

k8s01
k8s02
k8s03


（6）hadoop-env.sh
vi /opt/module/hadoop-3.1.3/etc/hadoop/hadoop-env.sh

export JAVA_HOME=/usr/local/jdk


（7）start-dfs.sh、stop-dfs添加以下内容 
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


6.配置环境变量
vi /etc/profile

export HADOOP_HOME=/opt/module/hadoop-3.1.3
export HADOOP_CONF_DIR=$HADOOP_HOME/etc/hadoop
export PATH=$PATH:$HADOOP_HOME/bin:$HADOOP_HOME/sbin


source /etc/profile

7.分发hadoop安装文件到k8s02和k8s03
rsync -av /opt/module/hadoop-3.1.3 root@k8s02:/opt/module/
rsync -av /opt/module/hadoop-3.1.3 root@k8s03:/opt/module/

rsync -av /etc/profile root@k8s02:/etc/profile
rsync -av /etc/profile root@k8s03:/etc/profile

# 使环境变量生效
source /etc/profile

8.调整k8s01、k8s02、k8s03的/etc/hosts主机名顺序

192.168.0.140 k8s01 master01.k8s.io   k8s-master-01
192.168.0.141 k8s02 master02.k8s.io   k8s-master-02
192.168.0.142 k8s03 node01.k8s.io     k8s-node-01
192.168.0.143 node02.k8s.io   k8s04   k8s-node-02
192.168.0.144 node03.k8s.io   k8s05   k8s-node-03
192.168.0.145 node04.k8s.io   k8s06   k8s-node-06
192.168.0.149 master.k8s.io   k8s-vip



9.检查zookeeper是否以及启动，没有启动则启动
# 检查
zk.sh status

# 启动
zk.sh start

10.启动journalnode(k8s01、k8s02、k8s03都执行)
hdfs --daemon start journalnode
jps

11.格式化HDFS（只在k8s01上执行）
hdfs namenode -format

12.启动namenode（只在k8s01上执行）
hdfs --daemon start namenode
jps

13.同步元数据 （在k8s02执行）
hdfs namenode -bootstrapStandby

14.格式化ZKFC （在k8s01执行）
hdfs zkfc -formatZK

15.启动hdfs（在k8s01执行）
start-all.sh

# 检查
xcall.sh jps

16.查看两个NameNode的运行状态
hdfs haadmin -getServiceState nn1
hdfs haadmin -getServiceState nn2

17.测试
hadoop fs -ls /
http://k8s02:50070

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

