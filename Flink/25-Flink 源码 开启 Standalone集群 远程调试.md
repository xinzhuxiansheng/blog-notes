## Flink 开启 Standalone集群 远程调试   

>Flink version: 1.15.4     

### 引言    
通过 `flink/bin/start-cluster.sh` 启动的 单节点 `JobManager`，单节点 `TaskManager`。 现在我们需要对 Standalone 集群进行远程调试， `那如何在 Flink 中添加远程调试的启动参数 ？`  

### 基于 Flink Standalone 集群，配置远程调试参数            
从 `flink/bin/start-cluster.sh` 脚本中可了解到 JM,TM 启动参数在 `flink/bin/config.sh`中有定义，内容如下：   
```shell
KEY_ENV_JAVA_OPTS="env.java.opts"
KEY_ENV_JAVA_OPTS_JM="env.java.opts.jobmanager"
KEY_ENV_JAVA_OPTS_TM="env.java.opts.taskmanager"
```

由于之前在 `flink-conf.yaml` 中配置 `env.java.opts: "-Djava.security.krb5.conf=krb5.conf"` ,j解决 kerboers认证的经验，那 `env.java.opts.jobmanager` 和 `env.java.opts.taskmanager` 参数也是可以在 `flink-conf.yaml`配置的。 

在 flink-conf.yaml 中添加以下参数： 
```shell
env.java.opts.jobmanager: -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=*:5007
env.java.opts.taskmanager: -agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=*:5008
```

>注意：suspend 参数，=y （JVM 启动时暂停），还是 =n (JVM 将在启动时不会暂停)，还需看你调试步骤。 

再重新启动 Standalone 集群。 

>注意：JM，TM的远程端口需分别设置不同端口。在机器上执行 `ps -ef|grep flink` 验证参数是否生效。    


### IDEA 配置 remote debug 
* Jobmanager 的入口类是 `org.apache.flink.runtime.entrypoint.StandaloneSessionClusterEntrypoint`    

* Taskmanager 的入口类是 `org.apache.flink.runtime.taskexecutor.TaskManagerRunner` 

>注意：在IDEA 配置`远程调试` 表单的 `use module classpath` 参数，其模块指的是`调试入口类所在的代码模块`, 根据 Flink 源码可知，StandaloneSessionClusterEntrypoint，TaskManagerRunner 都在 flink-runtime 模块中。  

找到 main() 方法打上断点就可以开始调试了。      