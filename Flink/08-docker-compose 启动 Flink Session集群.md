## docker-compose 启动 Flink Seesion集群

### docker-compose.yml 文件 
```yml
version: '3.7'
services:
  jobmanager:
    image: flink:1.14.6-scala_2.12-java8
    ports:
      - "8081:8081"
    command: jobmanager
    networks:
      - flinknet
    environment:
      - JOB_MANAGER_RPC_ADDRESS=jobmanager
    volumes:
      - ./flink-conf.yaml:/opt/flink/conf/flink-conf.yaml

  taskmanager:
    image: flink:1.14.6-scala_2.12-java8
    depends_on:
      - jobmanager
    command: taskmanager
    networks:
      - flinknet
    scale: 1
    environment:
      - JOB_MANAGER_RPC_ADDRESS=jobmanager
    volumes:
      - ./flink-conf.yaml:/opt/flink/conf/flink-conf.yaml

networks:
  flinknet:
    driver: bridge
```

```shell
docker-compose -f docker-compose.yml up -d  
```

>注意，此 session集群与k8s session集群不同，在k8s session集群下 TaskManager 不需要提前部署。    

### 错误内容    
```java
Starting Job Manager
[ERROR] The execution result is empty.
[ERROR] Could not get JVM parameters and dynamic configurations properly.
[ERROR] Raw output from BashJavaUtils:
#
# There is insufficient memory for the Java Runtime Environment to continue.
# Cannot create GC thread. Out of system resources.
# An error report file with more information is saved as:
# /opt/flink/hs_err_pid137.log  
```

### 解决方法    
因为 Docker版本过低，升级Docker 版本即可。