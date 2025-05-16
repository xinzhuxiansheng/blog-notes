# Flink on K8s - History Server - 在 Kubernetes 中部署 History Server  

>Flink version: 1.15.4, Kubernetes version: 1.30.8，minio version: RELEASE.2025-04-08T15-41-24Z    

## 背景    
Flink History Server 对于判断 `有界数据源` 作业的状态特别重要。你可以通过访问介绍 History Server 官网地址(https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/historyserver/) 了解它，但是 doc 并没有介绍 History Server on Kubernetes 细节。    

该篇 Blog 的核心是介绍如何在 Kubernetes 部署 Flink History Server。    

## 前置知识    
1.Flink History Server 的安装包与 Flink 的安装包是同一个，当执行 `bin/historyserver.sh (start|start-foreground|stop)` 命令，可以完成对 History Server 的启停操作， 所以将它部署在 Kubernetes 时，使用镜像也是 Flink 镜像。       

>有了安装包部署的经验，它与 Flink 是公用同一个配置文件，flink-conf.yaml 或者 config.yaml (博主已经在工作中使用到 1.19.2 version， `Flink conf/下只存在 config.yaml`) ，如果你看过我前面两篇公众号文章，你可能已经会意识到，我们的手动创建 History Server 的 ConfigMap，毕竟它不能像 Flink Cli 创建 Flink Job 那样，可以自动创建。 `下面是公众号文章链接`                
* https://mp.weixin.qq.com/s/_xUN4DD4le3YhAi_IlbK6g   
* https://mp.weixin.qq.com/s/DH81Xto6l6pm8NP7sbkxYA   

2.因为 History Server 和 Flink Job 使用同一个 Flink 镜像，又如何在 YAML 中启动 History Server？以 Flink Docker image 1.19.2 为例：    
![historyserveronk8s01](http://img.xinzhuxiansheng.com/blogimgs/flink/historyserveronk8s01.jpg)    

`ENTRYPOINT`是设置启动的脚本，`CMD` 设置默认参数, 在 `docker-entrypoint.sh` 定义了多个参数，包括 history server。    
![historyserveronk8s02](http://img.xinzhuxiansheng.com/blogimgs/flink/historyserveronk8s02.jpg)  

我们可以在 YAML 中设置 args 参数为 `history-server`,就代表启动的是 history server 服务。   
```yaml
    spec:
      imagePullSecrets:
        - name: default-secret
      containers:
        - name: flink-history
          image: 192.168.0.134:5000/flink:1.15.4-java11
          imagePullPolicy: Always
          args: ["history-server"]
```

3.Flink Job 和 History 需要配置 `归档地址`   
* Flink Job 配置的参数如下： 
```bash
jobmanager.archive.fs.dir: s3://flink-15-state/completed-jobs/
```

* History Server 配置参数如下：   
```bash
historyserver.web.port: 8082
historyserver.archive.fs.dir: s3://flink-15-state/completed-jobs/
historyserver.archive.fs.refresh-interval: 2000
historyserver.web.tmpdir: /data/flink/history
historyserver.archive.retained-jobs: 200
```

4.History Server 会将 S3归档目录中的 JSON 数据，转成多个 Http API 接口数据 json，如果 `historyserver.archive.retained-jobs` 设置过大，随着任务增加，若不设置 PVC 存储 解析后的目录数据，那 History Server 服务启动时将会重新读取 S3的归档 JSON数据，导致启动慢显现。   

>作业多的情况下，建议设置 PVC 挂载到 `historyserver.web.tmpdir` 参数设置的目录。     

## 部署步骤    

### 创建 configmap       
创建 `flink-15-configmap.yaml` 文件, 文件内容如下：   
```yaml
---
apiVersion: v1
kind: ConfigMap
metadata:
  name: flink-15-history-config
data:
  flink-conf.yaml: |+
    jobmanager.rpc.address: localhost
    jobmanager.rpc.port: 6123
    jobmanager.bind-host: localhost
    jobmanager.memory.process.size: 1600m
    taskmanager.bind-host: localhost
    taskmanager.host: localhost
    taskmanager.memory.process.size: 1728m
    taskmanager.numberOfTaskSlots: 1
    parallelism.default: 1
    jobmanager.execution.failover-strategy: region
    rest.address: localhost
    rest.bind-address: localhost
    
    # 下面是 yzhou 添加的
    env.java.opts.jobmanager: -Duser.timezone=GMT+08
    env.java.opts.taskmanager: -Duser.timezone=GMT+08
    
    #==============================================================================
    # ......s3......
    #==============================================================================
    s3.access-key: minio
    s3.secret-key: minio123
    s3.path.style.access: true
    s3.endpoint: http://192.168.0.135:9000
    fs.allowed-fallback-filesystems: s3
    
    #==============================================================================
    # ......state............
    #==============================================================================
    state.backend: rocksdb
    state.checkpoints.dir: s3://flink-15-state/checkpoints
    state.checkpoints.num-retained: 3
    state.savepoints.dir: s3://flink-15-state/savepoints
    state.backend.incremental: true
    state.backend.rocksdb.files.open: -1
    state.backend.rocksdb.thread.num: 4
    state.backend.rocksdb.writebuffer.count: 8
    state.backend.rocksdb.writebuffer.number-to-merge: 4
    state.backend.rocksdb.writebuffer.size: 64
    
    #==============================================================================
    # ......HistoryServer......
    #==============================================================================
    jobmanager.archive.fs.dir: s3://flink-15-state/completed-jobs/
    historyserver.web.port: 8082
    historyserver.archive.fs.dir: s3://flink-15-state/completed-jobs/
    historyserver.archive.fs.refresh-interval: 2000
    historyserver.web.tmpdir: /data/flink/history
    historyserver.archive.retained-jobs: 200

  log4j-console.properties: |+
    rootLogger.level = DEBUG
    rootLogger.appenderRef.console.ref = ConsoleAppender
    rootLogger.appenderRef.rolling.ref = RollingFileAppender
    log4j.logger.unipusFlink = INFO,myConsole
    log4j.appender.myConsole=org.apache.log4j.ConsoleAppender
    log4j.appender.myConsole.target=System.out
    log4j.appender.myConsole.layout=org.apache.log4j.PatternLayout
    log4j.appender.myConsole.layout.ConversionPattern=%marker %m%n
    log4j.appender.JsonConsoleAppender=org.apache.log4j.ConsoleAppender
    log4j.appender.JsonConsoleAppender.Target=System.out
    log4j.appender.JsonConsoleAppender.layout=org.apache.log4j.PatternLayout
    log4j.appender.JsonConsoleAppender.layout.ConversionPattern=%marker %m
    logger.akka.name = akka
    logger.akka.level = INFO
    logger.kafka.name= org.apache.kafka
    logger.kafka.level = INFO
    logger.hadoop.name = org.apache.hadoop
    logger.hadoop.level = INFO
    logger.zookeeper.name = org.apache.zookeeper
    logger.zookeeper.level = INFO
    appender.console.name = ConsoleAppender
    appender.console.type = CONSOLE
    appender.console.layout.type = PatternLayout
    appender.console.layout.pattern =%marker %d{yyyy-MM-dd HH:mm:ss,SSS} %-5p %-60c %x - %m%n
    appender.rolling.name = RollingFileAppender
    appender.rolling.type = RollingFile
    appender.rolling.append = false
    appender.rolling.fileName = ${sys:log.file}
    appender.rolling.filePattern = ${sys:log.file}.%i
    appender.rolling.layout.type = PatternLayout
    appender.rolling.layout.pattern = %d{yyyy-MM-dd HH:mm:ss,SSS} %-5p %-60c %x - %m%n
    appender.rolling.policies.type = Policies
    appender.rolling.policies.size.type = SizeBasedTriggeringPolicy
    appender.rolling.policies.size.size=100MB
    appender.rolling.strategy.type = DefaultRolloverStrategy
    appender.rolling.strategy.max = 10
    logger.netty.name = org.jboss.netty.channel.DefaultChannelPipeline
    logger.netty.level = OFF
```

## 创建 pvc   
vim flink-15-history-pvc.yaml  
```yaml
#Flink 日志 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-15-history-pvc  # 日志 pvc名称
  namespace: flink
spec:
  storageClassName: nfs-storage   #sc名称，更改为实际的sc名称
  accessModes:
    - ReadWriteMany   #采用ReadWriteMany的访问模式
  resources:
    requests:
      storage: 20Gi    #存储容量，根据实际需要更改
```

## 部署 Flink History Server   
vim flink-15-history-statefulset.yaml
```yaml 
---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: flink-15-history
  namespace: flink
  labels:
    app: flink-15-history
spec:
  serviceName: flink-15-history
  replicas: 1
  selector:
    matchLabels:
      app: flink-15-history
  template:
    metadata:
      labels:
        app: flink-15-history
    spec:
      imagePullSecrets:
        - name: default-secret
      containers:
        - name: flink-history
          image: 192.168.0.134:5000/flink:1.15.4-java11
          imagePullPolicy: Always
          args: ["history-server"]
          resources:
            requests:
              memory: 4096M
            limits:
              memory: 4096M
          volumeMounts:
            - mountPath: /opt/flink/lib
              name: flink-15-lib
            - mountPath: /opt/flink/conf

              name: flink-15-history-config-volume
            - mountPath: /data/flink/history
              name: persistent-storage
          ports:
            - containerPort: 8082
              name: http
      volumes:
        - name: persistent-storage
          persistentVolumeClaim:
            claimName: flink-15-history-pvc
        - name: flink-15-lib
          persistentVolumeClaim:
            claimName: flink-15-lib-pvc    
        - name: flink-15-history-config-volume
          configMap:
            name: flink-15-history-config
            items:
              - key: flink-conf.yaml
                path: flink-conf.yaml
              - key: log4j-console.properties
                path: log4j-console.properties
---
kind: Service
apiVersion: v1
metadata:
  name: flink-15-history-service
spec:
  selector:
    app: flink-15-history
  ports:
    - name: http
      protocol: TCP
      port: 8082
      targetPort: 8082
      nodePort: 30001
  type: NodePort
```  

## 执行部署    
```shell
kubectl apply -f flink-15-history-pvc.yaml -n 
kubectl apply -f flink-15-history-statefulset.yaml -n 
```

访问 NodeIp + NodePort 端口即可。   
http://192.168.0.143:30001/#/overview         

refer 
1.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/historyserver/        
2.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/logging/   