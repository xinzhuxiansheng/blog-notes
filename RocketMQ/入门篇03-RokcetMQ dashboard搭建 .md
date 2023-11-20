## RocketMQ dashboard 源码搭建

>因仓库的master分支将RocketMQ版本升级到5.1.0后，当RocketMQ Broker集群为4.9.5, 会导致Topic页面操作出现不兼容，现在将原master分支回退到 86bdb0636494bc23751f5df90fdb56a87b928ca7 commit(当时RocketMQ版本为4.9.3)， 如果希望兼容 4.x版本的RocketMQ，也可导入`https://github.com/xinzhuxiansheng/rocketmq-dashboard/tree/yzhou_master_493`分支，且jdk 版本 >= 1.8

### 引言 
摘自官网介绍​

"RocketMQ Dashboard is a tool for managing RocketMQ, providing various statistical information on events and performance of clients and applications, and supporting visualized tools to replace command line operations such as topic configuration and broker management."

### 介绍    
Feature Overview    
* OPS: Modify nameserver address; use VIPChannel 
* Dashboard: Check broker, topic message volume   
* Cluster: Cluster distribution, broker configuration, runtime information   
* Topic: Search, filter, delete, update/add topics, message routing, send messages, reset consumption points            
* Consumer: Search, delete, add/update consumer groups, terminals, consumption details, configuration       
* Message: Message records, private messages, message trace, etc. message details           

### 下载源码
通过`git clone git@github.com:apache/rocketmq-dashboard.git` 下载dashboard源码，通过IntelliJ IDEA导入项目即可。         

### 配置介绍
因`rocketmq-dashboard`是spring boot项目，故项目的配置在resources文件夹下的`application.yaml`。     

dashboard服务默认`namesrvAddrs: 127.0.0.1:9876,127.0.0.2:9876` 所以在启动之前先启动navesrv、broker服务, 完成以上即可。 

refer  
1.https://rocketmq.apache.org/docs/deploymentOperations/04Dashboard 