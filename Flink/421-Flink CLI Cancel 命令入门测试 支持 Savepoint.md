## Flink CLI Cancel 命令入门测试 支持 Savepoint       

>Flink version: 1.15.4      

### 取消任务 & 指定 savepoint 参数    
* -s,--fromSavepoint <savepointPath>         Path to a savepoint to restore
                                        the job from (for example
                                        hdfs:///flink/savepoint-1537).      
指定 savepoint 存储路径   

```
./flink cancel -s [hdfs/file]://路径 [job id]   
```

示例： 
./flink cancel -s file:///root/flink/savepoint c7354f8e4835c676d9c1af73098ee54b  

