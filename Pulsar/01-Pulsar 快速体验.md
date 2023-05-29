## Apache Pulsar 快速体验

>Pulsar version: 2.11

### 引言    
Apache Pulsar作为消息中间件中一员，本篇blog主要讲解Pulsar的快速体验以及命令行方式写入和消费数据。    

### 单机安装Pulsar  
目前Pulsar已经支持到3.x版本，本篇主要讲解2.11，首先通过Plusar官网下载二进制安装包（https://pulsar.apache.org/download/），还需注意Pulsar安装的环境要求，请参考官方文档：    https://github.com/apache/pulsar/blob/master/README.md#pulsar-runtime-java-version-recommendation ，若出现`Unrecognized VM option 'UseZGC'`异常描述，将环境中JDK版本升级值`JDK 17`。  

```shell
wget https://archive.apache.org/dist/pulsar/pulsar-2.11.1/apache-pulsar-2.11.1-bin.tar.gz       

tar xvfz apache-pulsar-2.11.1-bin.tar.gz

cd apache-pulsar-2.11.1
```

**standalone模式启动**  
Pulsar官方文档也提供standalone模式操作介绍（https://pulsar.apache.org/docs/2.11.x/getting-started-standalone/#start-a-pulsar-standalone-cluster） ，这里请注意官方中 `bin/pulsar standalone`，并不是后台运行模式。pulsar也提供了`后台运行` ，该模式下用户结束当前终端进程，并不会中断pulsar运行。      

```shell
./pulsar-daemon start standalone
./pulsar-daemon stop standalone
```

完成以上操作即可。  


### shell模拟生产和消费

**生产**    
```shell
./pulsar-client produce topic-test --messages "Hello Pulsar"
``` 
`output`:  
![getStart01](http://img.xinzhuxiansheng.com/blogimgs/pulsar/getStart01.png)   

>此处特别注意：Pulsar默认的持久化与Kafka（默认7天）不同，Pulsar 会自动删除没有活跃生产者和消费者的主题。具体的超时时间取决于 `conf/broker.conf` 配置中的`brokerDeleteInactiveTopicsMode`、`brokerDeleteInactiveTopicsMaxInactiveDurationSeconds`、`brokerServicePurgeInactiveFrequencyInSeconds=60`。      
![getStart02](http://img.xinzhuxiansheng.com/blogimgs/pulsar/getStart02.png)    

>此时需注意：`topic-test`是自动创建的，这里先简单介绍下，Pulsar`conf/broker.conf` allowAutoTopicCreation决定是否自动创建主题，默认是自动创建，也同时根据`allowAutoTopicCreationType`决定创建哪种类型的主题，默认为创建非分区主题， 这里提前跟大家说下，Pulsar的主题区分`分区主题`和`非分区主题`, 后面的Blog会介绍它。   
![getStart04](http://img.xinzhuxiansheng.com/blogimgs/pulsar/getStart04.png) 
 

此时我们可以使用`Admin`Shell工具查看创建的主题和消息写入情况，在Pulsar中，在不指定主题所属的租户与命名空间的情况下，默认使用的主题为 `persistent://public/default/topic-test`。使用`pulsar-admin`Shell工具运行如下，可以查看租户为public且命名空间为default的所有主题列表。在写入成功的情况下可以看到终端输出中列出了持久化主题的完整路径名称。 
```shell
# 若发送时间与执行下面间隔超过 60s，Pulsar默认会将topic-test主题删除，所以你可能执行该命令后无任何返回，请注意这点, 可配置brokerDeleteInactiveTopicsMaxInactiveDurationSeconds参数
[root@localhost bin]# ./pulsar-admin topics  list public/default
persistent://public/default/topic-test
```

还可以通过`Admin`Shell管理工具查看生产者写入的消息等详细信息。使用如下命令可以查看`一个主题内的统计状态信息`。该命令的返回值为一个较为复杂的JSON字符串。    
```shell
./pulsar-admin topics stats persistent://public/default/topic-test  
```
`output`    
![getStart03](http://img.xinzhuxiansheng.com/blogimgs/pulsar/getStart03.png)    

其中`msgInCounter`代表写入的消息数量，`storageSize`代表当前写入的消息大小。 


**消费**            
```shell
./pulsar-client consume topic-test -s "subscription_test" --subscription-position Earliest
``` 
`output`        

```
2023-05-29T19:48:00,645+0800 [pulsar-client-io-1-1] INFO  org.apache.pulsar.client.impl.ConsumerImpl - [topic-test][subscription_test] Subscribed to topic on localhost/127.0.0.1:6650 -- consumer: 0
----- got message -----
key:[null], properties:[], content:Hello Pulsar
2023-05-29T19:48:08,225+0800 [pulsar-client-io-1-1] INFO  org.apache.pulsar.client.impl.ConsumerImpl - [topic-test] [subscription_test] Closed consumer
2023-05-29T19:48:08,229+0800 [main] INFO  org.apache.pulsar.client.impl.PulsarClientImpl - Client closing. URL: pulsar://localhost:6650/
2023-05-29T19:48:08,270+0800 [pulsar-client-io-1-1] INFO  org.apache.pulsar.client.impl.ClientCnx - [id: 0x28aaa17b, L:/127.0.0.1:52500 ! R:localhost/127.0.0.1:6650] Disconnected
2023-05-29T19:48:10,309+0800 [main] INFO  org.apache.pulsar.client.cli.PulsarClientTool - 1 messages successfully consumed
```

以上，我们已经初步体验Pulsar的生产和消费，`需特别注意Topic的删除策略参数`。   


refer   
1.https://pulsar.apache.org/docs/2.11.x/getting-started-standalone/ 
2.《Apache Pulsar原理解析与应用实践》