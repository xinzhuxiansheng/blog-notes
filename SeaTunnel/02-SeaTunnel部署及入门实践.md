## SeaTunnel部署及入门实践  

>version: 2.3.2 

### 引言    
SeaTunnel就不过多介绍，请参考上一篇《SeaTunnel介绍》，本篇主要讲解SeaTunnel部署及入门实践。 

### 部署    

#### Step1: 准备环境    
在开始本地运行之前，您需要确保您已经安装了 SeaTunnel 所需的以下软件：`安装Java（Java 8或11，高于Java 8的其他版本理论上也可以工作）并设置JAVA_HOME`。    

#### Step2: 下载 SeaTunnel  
进入seatunnel下载页面(https://seatunnel.apache.org/download/)，下载最新版本的分发包`seatunnel-<version>-bin.tar.gz`， 或者您可以通过终端下载：  
```shell
export version="2.3.2"
wget "https://archive.apache.org/dist/seatunnel/${version}/apache-seatunnel-${version}-bin.tar.gz"
tar -xzvf "apache-seatunnel-${version}-bin.tar.gz"
``` 

#### Step3: 安装连接器插件  
从`2.2.0-beta`开始，二进制包默认不提供连接器依赖，所以第一次使用时，我们需要执行以下命令来安装连接器：（当然，你也可以手动下载连接器从 Apache Maven 存储库下载(https://repo.maven.apache.org/maven2/org/apache/seatunnel/)，然后手动移动到连接器/seatunnel 连接器目录）。       
```shell
sh bin/install-plugin.sh 2.3.2
```
如果需要指定连接器的版本，以2.3.2为例，我们需要执行    
```shell
sh bin/install-plugin.sh 2.3.2  
``` 

通常你不需要所有的connector插件，所以你可以通过配置`config/plugin_config`来指定你需要的插件，例如你只需要connector-console插件，那么你可以将`plugin_config`修改为   
```
--seatunnel-connectors--
connector-console
--end--
``` 

`如果您想让示例应用程序正常工作，您需要添加以下插件`    
```
--seatunnel-connectors--
connector-fake
connector-console
--end--
```

您可以在`${SEATUNNEL_HOME}/connectors/plugins-mapping.properties` 下找到所有支持的连接器和相应的plugin_config 配置名称。        


>如果您想通过手动下载连接器的方式安装连接器插件，需要特别注意以下几点:  
`Connectors目录包含以下子目录，如果不存在，需要手动创建`    
```
flink
flink-sql
seatunnel
spark
``` 
如果您想手动安装V2连接器插件，只需下载您需要的V2连接器插件并将其放在seatunnel目录下。       


### 入门实践（seatunnel-engine）    
SeaTunnel目前支持 3种引擎（seatunnel-engine，flink，spark）， 该入门案例是使用seatunnel-engine引擎。其他引擎案例请参考https://seatunnel.apache.org/docs/2.3.2/start-v2/locally/quick-start-flink， https://seatunnel.apache.org/docs/2.3.2/start-v2/locally/quick-start-spark   


#### Step1: 部署 SeaTunnel 和 Connectors    
开始之前，请确保您已按照部署中的说明下载并部署 SeaTunnel    

#### Step2: 添加作业配置文件以定义作业  
编辑`config/v2.batch.config.template`，决定了seatunnel启动后数据输入、处理、输出的方式和逻辑。以下是配置文件的示例，与上面提到的示例应用程序相同。  

```
env {
  execution.parallelism = 1
  job.mode = "BATCH"
}

source {
  FakeSource {
    result_table_name = "fake"
    row.num = 16
    schema = {
      fields {
        name = "string"
        age = "int"
      }
    }
  }
}

transform {
  FieldMapper {
    source_table_name = "fake"
    result_table_name = "fake1"
    field_mapper = {
      age = age
      name = new_name
    }
  }
}

sink {
  Console {
    source_table_name = "fake1"
  }
}

```

有关配置的更多信息请查看配置概念(https://seatunnel.apache.org/docs/2.3.2/concept/config)    

#### Step3: 运行 SeaTunnel 应用程序
您可以通过以下命令启动应用程序  
```shell
cd "apache-seatunnel-incubating-${version}"
./bin/seatunnel.sh --config ./config/v2.batch.config.template -e local
```

查看输出：运行命令时，您可以在控制台中看到其输出。您可以认为这是命令运行成功与否的标志。
SeaTunnel 控制台将打印一些日志，如下所示：  
``` 
2023-06-30 08:39:37,208 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=1:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : gxgFD, 906018297
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=2:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : jjHpJ, 1150523788
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=3:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : iBzTD, 952799146
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=4:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : ErgKZ, 1684743447
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=5:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : ZllkM, 767799189
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=6:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : ujHhe, 1878962786
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=7:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : cHivd, 564890880
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=8:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : YViKl, 26547403
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=9:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : ZfUVU, 1491143191
2023-06-30 08:39:37,209 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=10:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : Izvpp, 812425976
2023-06-30 08:39:37,210 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=11:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : ohfvf, 2112805640
2023-06-30 08:39:37,210 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=12:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : PkYns, 987977990
2023-06-30 08:39:37,210 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=13:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : XUpMt, 568568052
2023-06-30 08:39:37,210 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=14:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : jnlxL, 1698187042
2023-06-30 08:39:37,210 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=15:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : eSxGA, 412341258
2023-06-30 08:39:37,210 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=0  rowIndex=16:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : sVwxf, 941238118
```

现在，您已经快速浏览了 SeaTunnel，您可以看到连接器来查找 SeaTunnel 支持的所有源和接收器。或者如果您想了解有关 SeaTunnel 引擎的更多信息(https://seatunnel.apache.org/docs/2.3.2/seatunnel-engine/about/)，请参阅 SeaTunnel 引擎。

refer   
1.https://seatunnel.apache.org/docs/2.3.2/start-v2/locally/deployment   
2.https://www.jb51.net/program/2850931un.htm    
3.https://mp.weixin.qq.com/s/EIZf1Bfjeq9lYPLtzp9ucA     
4.https://seatunnel.apache.org/docs/2.3.2/start-v2/locally/quick-start-seatunnel-engine 
