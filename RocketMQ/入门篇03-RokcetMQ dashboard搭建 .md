
## RocketMQ dashboard搭建

>因仓库的master分支将RocketMQ版本升级到5.1.0后，当RocketMQ Broker集群为4.9.5, 会导致Topic页面操作出现不兼容，现在将原master分支回退到 86bdb0636494bc23751f5df90fdb56a87b928ca7 commit(当时RocketMQ版本为4.9.3)， 如果希望兼容 4.x版本的RocketMQ，建议大家下载`https://github.com/xinzhuxiansheng/rocketmq-dashboard/tree/yzhou_master_493`

### 下载源码
通过`git clone git@github.com:apache/rocketmq-dashboard.git` 下载dashboard源码，通过IntelliJ IDEA导入项目即可。

### 配置介绍
因`rocketmq-dashboard`是spring boot项目，故项目的配置在resources文件夹下的`application.yaml`。

dashboard服务默认namesrvAddrs: 127.0.0.1:9876,127.0.0.2:9876 所以在启动之前先启动navesrv、broker服务。



