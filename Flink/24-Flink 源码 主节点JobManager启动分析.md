## Flink 源码 主节点 JobManager 启动分析    

### 介绍    
JobManager 是 Flink 集群的主节点，它包含四大重要的组件：        
1.ResourceManager  
Flink 的集群资源管理器，只有一个，关于 Slot 的管理和申请等工作，都由他负责  

2.Dispatcher   
负责接受用户提交的 JobGraph，然后启动一个 JobManager            

3.JobManager   
负责一个具体的 Job 的执行，在一个集群中，可能包含多个 JobManager 同时执行，类似于 YARN 集群中 AppMaster 角色，类似于 Spark Job 中的 Driver 角色 

4.WebMonitorEndpoint    
里面维护了很多 Handler，如果客户端通过 flink run 的方式来提交一个 job 到 flink 集群，最终是由 WebMonitorEndpoint 来接受，并且决定哪一个 Handler 来执行处理  
```
submitJob ==> SubmitJobHandler  
```

总结一下： 
Flink 集群的主节点内部运行着： ResourceManager 和 Dispatcher， 当 client 提交一个 Job 到集群运行的时候（客户端会把该 Job 构建一个 JobGragh 对象）， Dispatcher 负责拉起 JobManager 来负责这个 Job 内部的 Task 执行，执行 Task 所需要的资源，JobManager 向 ResourceManager 申请。    

### JobManager 启动主类 
根据以上的启动脚本分析，了解到 JobManager的启动主类是 StandaloneSessionClusterEntrypoint        


