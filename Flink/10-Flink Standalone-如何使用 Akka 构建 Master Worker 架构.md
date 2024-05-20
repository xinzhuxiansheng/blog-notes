# Flink Standalone - 如何使用 Akka 构建 Master Worker 架构    

>Flink version: 1.17   

## 引言  
为了引出本篇的核心内容，我先将 Flink Standalone（Session Mode） 



**启动 Standalone 集群**
```shell
./start-cluster.sh  
```     

Output log:         
```shell
[root@vm01 bin]# ./start-cluster.sh
Starting cluster.
Starting standalonesession daemon on host vm01.
Starting taskexecutor daemon on host vm01.
[root@vm01 bin]#
```

**查看 Standalone 相关进程**
```shell
[root@vm01 bin]# jps
1154 TaskManagerRunner
858 StandaloneSessionClusterEntrypoint   
```

## 调试 sh      
```shell
# 安装 dev tools  
yum groupinstall 'Development Tools'            

# 下载 bashdb-4.4-1.0.1   
https://sourceforge.net/projects/bashdb/files/bashdb/4.4-1.0.1/     

# 安装 bashdb 
./configure     
make && make install 

# 查看是否安装成功      
bashdb --version   
```

## bashdb 使用      
```shell
n 单步 步过 遇到函数 不进函数    

print $test 打印test变量的值        

l 列出当前行以下的10行  
```

### break（断点）       
查看官网文档 `https://bashdb.sourceforge.net/bashdb.html#Set-Breaks` 







>Flink Web UI 无法访问的问题  
```shell
# 在其他节点，无法访问  
[root@vm01 conf]# curl 192.168.0.201:8081
curl: (7) Failed connect to 192.168.0.201:8081; Connection refused

# 在 Flink Standalone 节点，访问正常
[root@vm01 conf]# curl localhost:8081
<!--
  ~ Licensed to the Apache Software Foundation (ASF) under one
  ~ or more contributor license agreements.  See the NOTICE file
  ~ distributed with this work for additional information
  ~ regarding copyright ownership.  The ASF licenses this file
  ~ to you under the Apache License, Version 2.0 (the
```

**修改 conf/flink-conf.yaml**       
修改 rest.address, rest.bind-address 为 IP   
```yaml
rest.address: 192.168.0.201
rest.bind-address: 192.168.0.201
```







## 启动命令     
```bash
root      94562      1  2 10:00 pts/0    00:00:33 /data/jdk1.8.0_391/bin/java -Xmx1073741824 -Xms1073741824 -XX:MaxMetaspaceSize=268435456 -Dlog.file=/root/yzhou/flink/flink1172/flink-1.17.2/log/flink-root-standalonesession-0-vm01.log -Dlog4j.configuration=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j.properties -Dlog4j.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j.properties -Dlogback.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/logback.xml -classpath /root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-cep-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-connector-files-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-csv-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-json-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-scala_2.12-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-api-java-uber-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-planner-loader-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-runtime-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-1.2-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-core-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-slf4j-impl-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-dist-1.17.2.jar:::/opt/module/hadoop-3.1.3/etc/hadoop: org.apache.flink.runtime.entrypoint.StandaloneSessionClusterEntrypoint -D jobmanager.memory.off-heap.size=134217728b -D jobmanager.memory.jvm-overhead.min=201326592b -D jobmanager.memory.jvm-metaspace.size=268435456b -D jobmanager.memory.heap.size=1073741824b -D jobmanager.memory.jvm-overhead.max=201326592b --configDir /root/yzhou/flink/flink1172/flink-1.17.2/conf --executionMode cluster
root      94852      1  2 10:00 pts/0    00:00:32 /data/jdk1.8.0_391/bin/java -XX:+UseG1GC -Xmx536870902 -Xms536870902 -XX:MaxDirectMemorySize=268435458 -XX:MaxMetaspaceSize=268435456 -Dlog.file=/root/yzhou/flink/flink1172/flink-1.17.2/log/flink-root-taskexecutor-0-vm01.log -Dlog4j.configuration=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j.properties -Dlog4j.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/log4j.properties -Dlogback.configurationFile=file:/root/yzhou/flink/flink1172/flink-1.17.2/conf/logback.xml -classpath /root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-cep-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-connector-files-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-csv-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-json-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-scala_2.12-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-api-java-uber-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-planner-loader-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-table-runtime-1.17.2.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-1.2-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-api-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-core-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/log4j-slf4j-impl-2.17.1.jar:/root/yzhou/flink/flink1172/flink-1.17.2/lib/flink-dist-1.17.2.jar:::/opt/module/hadoop-3.1.3/etc/hadoop: org.apache.flink.runtime.taskexecutor.TaskManagerRunner --configDir /root/yzhou/flink/flink1172/flink-1.17.2/conf -D taskmanager.memory.network.min=134217730b -D taskmanager.cpu.cores=40.0 -D taskmanager.memory.task.off-heap.size=0b -D taskmanager.memory.jvm-metaspace.size=268435456b -D external-resources=none -D taskmanager.memory.jvm-overhead.min=201326592b -D taskmanager.memory.framework.off-heap.size=134217728b -D taskmanager.memory.network.max=134217730b -D taskmanager.memory.framework.heap.size=134217728b -D taskmanager.memory.managed.size=536870920b -D taskmanager.memory.task.heap.size=402653174b -D taskmanager.numberOfTaskSlots=40 -D taskmanager.memory.jvm-overhead.max=201326592b
```















refer                   
1.https://nightlies.apache.org/flink/flink-docs-release-1.19/docs/deployment/resource-providers/standalone/overview/                    
2.https://nightlies.apache.org/flink/flink-docs-master/release-notes/flink-1.18/                
3.https://blog.csdn.net/whatday/article/details/88714555            
4.https://blog.csdn.net/weixin_44512041/article/details/135517529                   
5.http://bashdb.sourceforge.net/bashdb.html             
6.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/concepts/flink-architecture/      




