
## RocketMQ dashboard搭建

### 下载源码
通过`git clone git@github.com:apache/rocketmq-dashboard.git` 下载dashboard源码，通过IntelliJ IDEA导入项目即可。

### 配置介绍
因`rocketmq-dashboard`是spring boot项目，故项目的配置在resources文件夹下的`application.yaml`。

dashboard服务默认namesrvAddrs: 127.0.0.1:9876,127.0.0.2:9876 所以在启动之前先启动navesrv、broker服务。



