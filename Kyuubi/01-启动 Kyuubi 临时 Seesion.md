# Kyuubi - 启动 Kyuubi 临时 Seesion 

## 创建 Kyuubi Engine 
执行 beeline 命令   
注意：jdbc:hive2://bigdata04:10009/ 指向的是 Kyuubi 服务的端口  
```bash
bin/beeline -u 'jdbc:hive2://bigdata04:10009/;#kyuubi.engine.type=SPARK_SQL;spark.master=yarn;spark.submit.deployMode=cluster' -n root
```

启动完后，可以看到 `0: jdbc:hive2://bigdata04:10009/>` 待输入的提示符。 示例输出如下：    
```bash
2025-09-12 02:55:13.313 INFO KyuubiSessionManager-exec-pool: Thread-67 org.apache.kyuubi.shaded.zookeeper.ZooKeeper: Session: 0x100002266820001 closed
2025-09-12 02:55:13.316 INFO KyuubiSessionManager-exec-pool: Thread-67 org.apache.kyuubi.operation.LaunchEngine: Processing root's query[e4421b0e-4123-40ec-b614-7564266e42ed]: RUNNING_STATE -> FINISHED_STATE, time taken: 47.994 seconds
Connected to: Spark SQL (version 3.5.5)
Driver: Kyuubi Project Hive JDBC Client (version 1.10.2)
Beeline version 1.10.2 by Apache Kyuubi
0: jdbc:hive2://bigdata04:10009/>
```

注意：当 Cli 终端结束后，Seesion 会立马结束，但是 engine 还会再运行一段时间（默认会存活 30分钟）。    

kyuubi.session.idle.timeout.ms=1800000 若需要修改 engine 存活时间，可修改 Kyuubi 配置文件 `kyuubi-defaults.conf`  


>kyuubi 默认是内存元数据模式。