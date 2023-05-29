## Pulsar 安装包目录介绍

>Pulsar version: 2.11   

### 引言
上一篇《Pulsar 快速体验》 我们了解`Run Pulsar Locally`为主题的实践，本篇来了解2.11版本的Pulsar解压后，各个目录的功能介绍。  


### 目录下文件介绍  
```
wget https://archive.apache.org/dist/pulsar/pulsar-2.11.1/apache-pulsar-2.11.1-bin.tar.gz       
tar xvfz apache-pulsar-2.11.1-bin.tar.gz
cd apache-pulsar-2.11.1
```     

![pbinDir01](http://img.xinzhuxiansheng.com/blogimgs/pulsar/pbinDir01.png)  

* **bing** 与Pulsar服务相关的命令行工具目录     
* **conf** 与Pulsar服务相关的配置文件目录       
* **examples** Pulsar Function示例程序目录 
* **instances** Pulsar Funcation运行过程中创建的中间文件目录     
* **lib** Pulsar服务依赖的JAR文件目录   

>下面展示的目录会多一些目录，例如 data，logs，packages-storage, 这是因为我用standalone方式启动Pulsar后，会在部署目录下自动创建以上目录。    

* **data** ZooKeeper和Bookkeeper数据存放的路径 在`conf/bookkeeper.conf`配置文件中，通过`ledgerDirectories`参数指定Bookkeeper的数据存储地址； 在`conf/zookeeper.conf`配置文件中，通过`dataDir`可以指定Zookeeper数据存储地址。    
* **logs** Pulsar服务端日志文件目录 可以通过`conf/log4j2.yaml`进行修改，要修改 Pulsar 的日志目录，你需要在log4j2.yaml文件中找到 File appender 的配置，然后修改其`fileName`和 `filePattern`属性。这两个属性分别定义了日志文件的路径和日志文件的滚动模式。    
* **packages-storage** 用于简化和集中管理 Pulsar 函数包和连接器的功能， 如果你要使用 Pulsar 函数或连接器，通常需要在 Pulsar 集群中的每个节点上都安装相应的函数包或连接器，而Pulsar在2.8.0版本开始引入了`packages storage`,你可以将函数包或连接器上传到 Pulsar 集群，然后 Pulsar 集群会自动将这些包分发到所有的节点。这样，你就可以在任何一个节点上运行这些函数或连接器，而不需要关心这些函数包或连接器是否已经被安装。      
 

### Pulsar命令行    
![pbinDir02](http://img.xinzhuxiansheng.com/blogimgs/pulsar/pbinDir02.png)  

* **pulsar** 初始化以及直接启动核心服务、组建的入口 
* **pulsar-admin开头** Pulsar服务管理工具，对主题、命名空间等组建进行命令行管理的入口   
* **pulsar-client** Pulsar客户端命令行工具，支持在命令行构建生产者与消费者  
* **pulsar-daemon** 对后台启动的Pulsar核心组件进行管理的工具，例如Broker、Bookie、Zookeeper、functions-worker   
* **pulsar-perf** Pulsar Broker性能测试工具 
* **bookeeper** Bookkeeper管理工具      
* **pulsar-managed-ledger-admin** 访问ManagedLedger二进制数据的工具，在Pulsar的早期版本中（1.20.0-incubating），Broker开始将ManagedLedger数据存储为二进制格式，为避免这部分关键数据损坏后无法恢复，Pulsar提供该工具来访问二进制数据。该工具依赖于Python或Protobuf、kazoo等python包，使用前需要用户自行配置安装      

### Pulsar配置文件  
在`conf`目录下存放了各类配置文件，按照用途不同可以分为`核心服务配置文件`和`非核心服务配置文件`两类。    

![pbinDir03](http://img.xinzhuxiansheng.com/blogimgs/pulsar/pbinDir03.png) 

Pulsar核心服务配置文件包括以下几种。    

* **broker.conf** 负责处理来自生产者的传人消息、将消息分派给消费者及在集群之间复制数据等，该配置文件中存放的是与 Broker 相关的配置参数  
* **bookkeeper.conf** BookKeeper 是一个分布式日志存储系统，Pulsar 使用它来持久化存储所有的消息，该配置文件定义了 Pulsar 在使用 BookKeeper 进行持久化存储时的各类参数    
* **zookeeper.conf** Zookeeper 可协助 Pulsar 处理大部分与基本配置和协调相关的任务。Pulsar 服务中 Zookeeper 的默认配置文件即 Pulsar 安装后生成的 zookeeper.conf文件  
* **proxy.conf** 该配置文件定义了 Pulsar 网关的配置参数。在独立部署一些组件时(Function Worker )，需要在这里配置服务连接的参数。口websocket.conf: 用来定义一些网络协议层配置，例如链接超时、服务绑定的域名和端口、通信加密方式等   
* **discovery.conf** 与 Pulsar 服务发现相关的配置functions_worker.yml: Pulsar Function Worker 中使用的配置  
* **presto** Pulsar SQL 服务所依赖的关键配置目录，第 8 章将详细介绍与此相关的配置方式和参数含义，所以这里就不赘述了 
* **filesystem_offload_core_site.xml** Pulsar 分层存储中用到的文件存储系统的链接信息    

除此上述配置文件之外，其他几个配置文件不用于 Pulsar 的核心服务，但是它们同样己的特殊应用场景，具体如下：    
* **client.conf** 包含 pulsar-client 和 pulsar-admin 客户端工具中的配置参数，可以在该配置文件中配置远程连接的 Broker 连接信息和鉴权信息     
* **log4j2.yaml** log4j 日志参数配置文件    
* **standalone.conf** 单机部署模式中用到的配置文件，整合了 broker.conf 和bookecper.conf中的关键参数，用于启动单独部署的服务     


### Pulsar管理工具  
为了方便对集群状态、命名空间、主题等对象进行管理,Pulsar 提供了丰富的管理工直可以通过REST服务对这些工具进行访同或者通过二进制安装目录下的 bin/pulsar-admini行管理。例如可以通过下面两种方式查看集群健康状态、主题列表和订阅列表  
```shell
# 查看 Broker 节点
./pulsar-admin brokers lfst cluster name

# 查看集群健康状态
./pulsar-admin brokers healthcheck

# 查看主题列表
./pulsar-admin topics list public/default

# 查看订阅列表  
./pulsar-admin topics subscriptiong persistent://public/default/topic-testl
``` 

>上述Shell还可以同等的通过HTTP接口来获取    

```shell
# 查看 Broker 节点
curl http;//localhost:8080/admin/v2/persistent/public/default/

# 查看集群健康状态
curl http://localhost:8080/admin/v2/brokers/health

# 查看主题列表
curl http://localhost:8080/admin/v2/persistent/public/default/

# 查看订阅列表 
curl http://localhost:8080/admin/v2/persistent/public/default/topic-test1/subscriptions 
``` 

事实上，二进制安装包中的命令都是通过封装对应的 REST接口来实现的，因此只要掌握上述任意一种集群管理的方式，就可以同时熟练使用另一种方式。     


refer   
1.《Apache Pulsar原理解析与应用实践》
