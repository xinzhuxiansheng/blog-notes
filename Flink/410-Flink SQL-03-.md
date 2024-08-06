# Flink SQL - SQL Client - 探索 CLI 的实现逻辑     

>Flink version: 1.17.2       

## 引言   
该篇是基于之前 Blog `Flink SQL - SQL Client - 搭建 SQL CLI 调试环境`搭建的环境，所以下面`探索 CLI 的实现逻辑`与它的启动参数紧密相关，命令如下：   
```bash
/data/jdk1.8.0_391/bin/java  ......  /flink-sql-client-1.17.2.jar org.apache.flink.table.client.SqlClient --jar /root/yzhou/flink/flink1172/flink-1.17.2/opt/flink-sql-client-1.17.2.jar    
```  
>这样做的好处是避免看着看着就脱离了主干道 :)。            

## 查看 Log   
在 Linux 环境中使用`./sql-client.sh`时，我看到的是打印 Flink Logo 图案，并没有一些加载时打印的 log信息，注意我表达的是执行`./sql-client.sh`时，并不代表`log/flink-root-sql-client-xxx.log`，但这里往往容易被人忽视掉，在写`Flink SQL - SQL Client - 搭建 SQL CLI 调试环境` Blog时，因为配置 log4j策略，控制台会打印出一些执行过程中的 log。如下所示,我做了一些省略标记：       
```bash
D:\Software\JDK\jdk-1.8\bin\java.exe       

[jvm 参数及classpath 区域] 

org.apache.flink.table.client.SqlClient --jar D:\Code\Java\flink-all\flink_release-1.17\flink-table\flink-sql-client\target\flink-sql-client-1.17-SNAPSHOT.jar
Connected to the target VM, address: '127.0.0.1:54952', transport: 'socket'

[Flink 全局参数加载 区域]

2024-08-05 20:56:02,176 INFO  org.apache.flink.client.cli.CliFrontend                      [] - Loading FallbackYarnSessionCli
2024-08-05 20:56:02,182 WARN  org.apache.flink.core.plugin.PluginConfig                    [] - The plugins directory [plugins] does not exist.
2024-08-05 20:56:02,248 INFO  org.apache.flink.core.fs.FileSystem                          [] - Hadoop is not in the classpath/dependencies. The extended set of supported File Systems via Hadoop is not available.
2024-08-05 20:56:02,438 INFO  org.apache.flink.table.gateway.service.context.DefaultContext [] - Execution config: {execution.savepoint.ignore-unclaimed-state=false, execution.savepoint-restore-mode=NO_CLAIM, execution.attached=true, pipeline.jars=[file:/D:/Code/Java/flink-all/flink_release-1.17/flink-table/flink-sql-client/target/flink-sql-client-1.17-SNAPSHOT.jar, file:/D:/Code/Java/flink-all/flink_release-1.17/devlib/flink-python-1.17-SNAPSHOT.jar], execution.shutdown-on-attached-exit=false, pipeline.classpaths=[], execution.target=remote}
2024-08-05 20:56:04,177 INFO  org.apache.flink.configuration.Configuration                 [] - Config uses fallback configuration key 'rest.port' instead of key 'rest.bind-port'
2024-08-05 20:56:04,340 INFO  org.apache.flink.table.gateway.rest.SqlGatewayRestEndpoint   [] - Starting rest endpoint.
2024-08-05 20:56:07,562 INFO  org.apache.flink.table.gateway.rest.SqlGatewayRestEndpoint   [] - Rest endpoint listening at localhost:54959
2024-08-05 20:56:07,563 INFO  org.apache.flink.table.client.SqlClient                      [] - Start embedded gateway on port 54959
2024-08-05 16:57:48,596 INFO  org.apache.flink.table.client.gateway.ExecutorImpl           [] - Open session to localhost:54959 with connection version: V2.
八月 05, 2024 4:57:53 下午 org.jline.utils.Log logr
警告: Unable to create a system terminal, creating a dumb terminal (enable debug logging for more information)

2024-08-05 16:57:53,398 INFO  org.apache.flink.table.client.cli.CliClient                  [] - Command history file path: C:\Users\yzhou\flink-sql-history

[Flink icon 图标 区域]

        Welcome! Enter 'HELP;' to list all available commands. 'QUIT;' to exit.

Command history file path: C:\Users\yzhou\flink-sql-history

Flink SQL> 
```  







## 入口 main   
`org.apache.flink.table.client.SqlClient#main()`是 Sql Cli 的入口 main()方法，





启动 ./sql-client.sh 之后，编写 SQL，发现规律是回车换行并不代表输入结束，而是`;`



`;` + 回车换行  =  输入结束     











## 启动 Embedded Gateway    
在之前 Blog`Flink 源码 - Standalone - 探索 Flink Stream Job Show Plan 实现过程 - 构建 StreamGraph`简单介绍过`Flink Web UI`的`WebMonitorEndpoint`的启动过程，Flink Web UI的 HTTP Server 是由`WebMonitorEndpoint`提供的（它是由 Netty实现的 Server），我在调试`Show Plan`功能时会频繁涉及到 Netty Handler的处理流程，如下图：   
![sqlclientstartup01](images/sqlclientstartup01.png)      

>大家要是对这部分不太清楚，可查看我之前的 Blog。     


![sqlclientstartup02](images/sqlclientstartup02.png)












refer   
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/table/sqlclient/  