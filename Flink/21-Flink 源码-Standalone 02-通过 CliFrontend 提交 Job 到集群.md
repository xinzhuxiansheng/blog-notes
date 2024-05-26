# Flink 源码 - Standalone - 通过 CliFrontend 提交 Job 到 Standalone 集群   

>Flink version: 1.17.2   
 
## 修订		
在上一篇 Blog "Flink 源码 - Standalone - Idea 启动 Standalone 集群 (Session Model)" 内容中，介绍了 Idea 配置 "JobManager" 启动项和配置 "TaskManager" 启动项，它们都有 `Modify classpath`的配置，需手动添加`devconf`目录下的 jar，这个地方还需做一些修订。                   

我是参考 `./bin/start-cluster.sh` 脚本启动后的Java进程, 添加了 classpath，`可在 Idea 中，其实不需要全部配置`,当然添加后，启动并没有什么异常情况，但我现在要说的是一个例外，内容如下：                       

我在修改 `flink-runtime`模块的代码后，重启调试，发现修改后的代码并未生效，我尝试将 Maven 本地仓库中的`maven-repository/org/apache/flink/flink-runtime/1.17-SNAPSHOT` 目录清空，仍然不起作用，后来才意识到，我手动添加过 flink打包后的 lib，所以，我在 Idea 的启动配置项，将 `flink-dist-1.18-SNAPSHOT.jar` 从 classpath 配置中移除，再重新启动调试，则添加的代码就可以执行了。          

>注意：若修改了 源码中某个模块，若在classpath 也配置了，记得移除掉。                

修订结束，接下来我们介绍 “通过 CliFrontend 提交 Job 到 Standalone 集群”。              

## 引言     
在上一篇 Blog "Flink 源码 - Standalone - Idea 启动 Standalone 集群 (Session Model)" 介绍了"在 Idea 启动了 完整的 Standalone 集群"，本篇的主要内容是: 在 Idea 使用 `CliFrontend` 提交 Job 到 Standalone 集群，大家可阅读官网：https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/try-flink/local_installation/#submitting-a-flink-job, 了解 `$ ./bin/flink run examples/streaming/WordCount.jar` 脚本命令提交 Job 的示例。      

>注意：我在实操 `./bin/flink run 和 CliFrontend` 并非一帆风顺 ::>_<:: ，为了引入 CliFrontend 主题内容，我先介绍下 `./bin/flink 与 CliFrontend 关联性`。           

### 了解 ./bin/flink 与 CliFrontend 关联性          
vim bin/flink 脚本，将 exec 执行命令注释，添加 echo 打印。 修改内容：                   
```bash
echo "exec ${JAVA_RUN} $JVM_ARGS $FLINK_ENV_JAVA_OPTS ${log_setting[@]} -classpath \"`manglePathList \"$CC_CLASSPATH:    $INTERNAL_HADOOP_CLASSPATHS\"`\" org.apache.flink.client.cli.CliFrontend \"$@\""

 # exec "${JAVA_RUN}" $JVM_ARGS $FLINK_ENV_JAVA_OPTS "${log_setting[@]}" -classpath "`manglePathList "$CC_CLASSPATH:$I    NTERNAL_HADOOP_CLASSPATHS"`" org.apache.flink.client.cli.CliFrontend "$@"    
```

Output log:              
```bash
[root@vm01 flink-1.17.2]# ./bin/flink run examples/streaming/WordCount.jar  
exec /data/jdk1.8.0_391/bin/java    -Dlog.file=/root/yzhou/flink/flink1172/flink-1.17.2/log/flink-root-client-vm01.log -Dlog4j.configuration=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j-cli.properties -Dlog4j.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j-cli.properties -Dlogback.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/logback.xml -classpath "/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-cep-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-connector-files-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-csv-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-json-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-scala_2.12-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-api-java-uber-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-planner-loader-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-runtime-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-1.2-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-core-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-slf4j-impl-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-dist-1.17.2.jar::/opt/module/hadoop-3.1.3/etc/hadoop:" org.apache.flink.client.cli.CliFrontend "run examples/streaming/WordCount.jar"  
```
那么 ./bin/flink 脚本会调用 `CliFrontend#main()` 方法。     

## ./bin/flink run 实操    

### 修改 Standalone 的 conf/flink-conf.yaml         
>操作是基于 上一篇 Blog "Flink 源码 - Standalone - Idea 启动 Standalone 集群 (Session Model)" 的 Standalone 环境部署并且 devconf、devlib 目录配置会延续使用。          
![cliFrontenddebug01](http://img.xinzhuxiansheng.com/blogimgs/flink/cliFrontenddebug01.png)     

vim conf/flink-conf.yaml ,修改内容如下：            
```yaml
# 以下是修改
jobmanager.rpc.address: 192.168.0.201  # 修改IP
taskmanager.bind-host: 192.168.0.201   # 修改IP
taskmanager.host: 192.168.0.201        # 修改IP

# 以下是新增 
# 添加远程调试  
env.java.opts.jobmanager: -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=192.168.0.201:5007   
env.java.opts.taskmanager: -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=192.168.0.201:5008
``` 
>注意：`jobmanager.bind-host` 该参数不需要修改          

保存配置，重启 Standalone 集群。        
```shell
./bin/stop-cluster.sh   
./bin/start-cluster.sh    
```

### 提交 Job   
```shell
./bin/flink run examples/streaming/WordCount.jar          
```

Output log：    
```shell
[root@vm01 flink-1.17.2]# ./bin/flink run examples/streaming/WordCount.jar
Executing example with default input data.
Use --input to specify file input.
Printing result to stdout. Use --output to specify output path.
Job has been submitted with JobID ad22b1dfbf2ee7863516b8570af1b54d
Program execution finished
Job with JobID ad22b1dfbf2ee7863516b8570af1b54d has finished.
Job Runtime: 1366 ms
```

使用 tail log/flink-*-taskexecutor-*.out， 我的log示例：cat log/flink-root-taskexecutor-3-vm01.out               
```bash
(thought,1)
(and,10)
(enterprises,1)
(of,14)
(great,1)
(pith,1)
(and,11)
(moment,1)
(with,3)

......
```

>注意：可使用 ./bin/flink -h 了解它支持的参数含义，需特别注意 `不同运行模式的 Flink Job`，它的 cli 参数有区别, 别看错行了 ^_^;  ，例如 ./bin/flink run ... , ./bin/flink run-application ... , ./bin/flink yarn-cluster ...     

cli 成功了，接下来，开始介绍 CliFrontend 提交 Job 配置。                  

## 配置 Idea 启动 CliFrontend    
首先，我们在 项目根目录下创建一个 `devcliconf` 文件夹，将 `devconf`下的文件 copy 到 devcliconf 文件夹下。       

>为什么要这么做？   
![cliFrontenddebug02](http://img.xinzhuxiansheng.com/blogimgs/flink/cliFrontenddebug02.png)   

`StandaloneSessionClusterEntrypoint(Jobmanager)、TaskManagerRunner(Taskmanager)、./bin/flink run(CLI)` 这三个都依赖 `conf/flink-conf.yaml`,  CLI 在提交作业的时候，可以指定将 Job 提交到 某个集群，例如本地集群（CLI & Standalone 集群在一个节点）、外部集群 （CLI 与 Standalone 集群不在同一个节点 ），而控制它的参数在 conf/flink-conf.yaml 中配置：      
```yaml  
rest.address: 192.168.0.201 
rest.bind-address: 192.168.0.201  
```

所以，为了避免后面在 IDEA 调试 CLI & Standalone 源码时，来回调整 conf/flink-conf.yaml, 则单独将 CLI的配置放到 `devcliconf`目录下。          

### 1）配置 CliFrontend 启动项    
配置启动类：`org.apache.flink.client.cli.CliFrontend`           
JDK: 1.8    
Module: `flink-clients`   
VM options:   
```shell 
-Dlog.file=./log/flink-root-client-local.log 
-Dlog4j.configuration=./devcliconf/log4j-cli.properties 
-Dlog4j.configurationFile=./devcliconf/log4j-cli.properties 
-Dlogback.configurationFile=./devcliconf/logback.xml  
```

Main class: org.apache.flink.client.cli.CliFrontend     
Program arguments:  run ./flink-dist/target/flink-1.17-SNAPSHOT-bin/flink-1.17-SNAPSHOT/examples/streaming/WordCount.jar     
Environment variables: FLINK_CONF_DIR=./devcliconf     
Modify classpath: 选择 `Include` 且指定 devlib 的目录下的`jar`。   `需特别注意, 选择目录无效，需分别添加 jar`                 

>注意：新增了 `Environment variables: FLINK_CONF_DIR=./devcliconf`, 在之前 启动 StandaloneSessionClusterEntrypoint(Jobmanager)、TaskManagerRunner(Taskmanager) 是在 main()方法 添加 `--configDir devconf`。 为啥会以环境变量方式添加？         

CLI 在提交作业之后，会调用 CliFrontend#getConfigurationDirectoryFromEnv() 方法获取 conf/flink-conf.yaml 配置，首先会从 `FLINK_CONF_DIR 环境变量`获取 conf/ 路径，其次是其他路径判断。（坑了我较长时间，因为不添加它时，console log 没有展示异常信息，凡是别慌，看下源码 (￣▽￣)σ" ）                

### 2）配置控制台打印 log   
在项目根目录下 `devcliconf/log4j-cli.properties` 已经存在 `ConsoleAppender`的定义， 添加内容如下：   
```shell
rootLogger.appenderRef.stdout.ref = ConsoleAppender  # 24行 添加 
```

### 3）测试 CliFrontend 提交 Job 到远程 Standalone Cluster  
将 CliFrontend run 起来：       

Console log:        
```shell
Executing example with default input data.
Use --input to specify file input.
Printing result to stdout. Use --output to specify output path.
2024-05-27 01:30:50,037 INFO  org.apache.flink.client.program.rest.RestClusterClient       [] - Submitting job 'WordCount' (34bd1f5ecee4bfdd9c736241a56c8ef2).
2024-05-27 01:30:52,569 INFO  org.apache.flink.client.program.rest.RestClusterClient       [] - Successfully submitted job 'WordCount' (34bd1f5ecee4bfdd9c736241a56c8ef2) to 'http://192.168.0.201:8081'.
Job has been submitted with JobID 34bd1f5ecee4bfdd9c736241a56c8ef2
Program execution finished
Job with JobID 34bd1f5ecee4bfdd9c736241a56c8ef2 has finished.
Job Runtime: 1342 ms  
```     

Flink Web UI     
![cliFrontenddebug03](http://img.xinzhuxiansheng.com/blogimgs/flink/cliFrontenddebug03.png)     



refer       
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/try-flink/local_installation/#submitting-a-flink-job      
2.https://blog.csdn.net/u014686399/article/details/129523827              

