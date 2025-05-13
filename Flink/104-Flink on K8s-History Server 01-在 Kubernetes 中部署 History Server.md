# Flink - History Server - 在 Kubernetes 中部署 History Server  

>Flink version: 1.15.4, Kubernetes version: 1.30.8，minio version: RELEASE.2025-04-08T15-41-24Z    

## 引文   
Flink History Server 二进制部署方法可参考官网 `https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/historyserver/` 地址， 

## 创建 configmap   
创建 `flink-15-configmap.yaml`, 文件内容如下：   

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
    # 指定由JobManager归档的作业信息所存放的目录，这里使用的是HDFS
    jobmanager.archive.fs.dir: s3://flink-15-state/completed-jobs/
    # History Server所绑定的ip
    # historyserver.web.address: xxxx
    # 指定History Server所监听的端口号（默认8082）
    historyserver.web.port: 8082
    # 指定History Server扫描哪些归档目录，多个目录使用逗号分隔
    historyserver.archive.fs.dir: s3://flink-15-state/completed-jobs/
    # 指定History Server间隔多少毫秒扫描一次归档目录
    historyserver.archive.fs.refresh-interval: 2000
    #查找到的归档文件会下载并缓存到本地存储路径，默认/tmp目录下面，默认路径为System.getProperty("java.io.tmpdir") + File.separator + "flink-web-history-" + UUID.randomUUID()
    historyserver.web.tmpdir: /data/flink/history
    # 历史记录保留个数
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

http://192.168.0.143:30001/#/overview     

refer 
1.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/historyserver/        
2.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/logging/   