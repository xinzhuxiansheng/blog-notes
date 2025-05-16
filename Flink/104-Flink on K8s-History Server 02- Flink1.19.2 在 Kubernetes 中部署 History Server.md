# Flink on K8s - History Server - 在 Kubernetes 中部署 History Server  

>Flink version: 1.15.4, Kubernetes version: 1.30.8，minio version: RELEASE.2025-04-08T15-41-24Z    

##  
Flink History Server 二进制部署方法可参考官网 `https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/historyserver/` 地址， 

## 创建 configmap   
创建 `flink-19-configmap.yaml`, 文件内容如下：   

```yaml
---
apiVersion: v1
kind: ConfigMap
metadata:
  name: flink-19-history-config
data:
  config.yaml: |+
    jobmanager:
      rpc:
        address: localhost
        port: 6123
      bind-host: localhost
      memory:
        process:
          size: 1600m
      execution:
        failover-strategy: region
      archive:
        fs:
          dir: s3://flink-19-state/completed-jobs/        
        

    taskmanager:
      bind-host: localhost
      host: localhost
      memory:
        process:
          size: 1728m
      numberOfTaskSlots: 1

    parallelism:
      default: 1

    rest:
      address: localhost
      bind-address: localhost

    env:
      java:
        opts:
          jobmanager: -Duser.timezone=GMT+08
          taskmanager: -Duser.timezone=GMT+08

    # S3 configuration
    s3:
      access-key: minio
      secret-key: minio123
      path:
        style:
          access: true
      endpoint: http://192.168.0.135:9000

    fs:
      allowed-fallback-filesystems: s3

    # State backend configuration
    state:
      checkpoints:
        dir: s3://flink-19-state/checkpoints
        num-retained: 3
      savepoints:
        dir: s3://flink-19-state/savepoints
      backend:
        type: rocksdb
        incremental: true
        rocksdb:
          files:
            open: -1
          thread:
            num: 4
          writebuffer:
            count: 8
            number-to-merge: 4
            size: 64

    historyserver:
      web:
        port: 8082
        tmpdir: /data/flink/history
      archive:
        fs:
          dir: s3://flink-19-state/completed-jobs/
          refresh-interval: 2000
        retained-jobs: 200

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
vim flink-19-history-pvc.yaml  
```yaml
#Flink 日志 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-19-history-pvc  # 日志 pvc名称
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
vim flink-19-history-statefulset.yaml
```yaml 
---
apiVersion: apps/v1
kind: StatefulSet
metadata:
  name: flink-19-history
  namespace: flink
  labels:
    app: flink-19-history
spec:
  serviceName: flink-19-history
  replicas: 1
  selector:
    matchLabels:
      app: flink-19-history
  template:
    metadata:
      labels:
        app: flink-19-history
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
              memory: 2048M
            limits:
              memory: 2048M
          volumeMounts:
            - mountPath: /opt/flink/lib
              name: flink-19-lib
            - mountPath: /opt/flink/conf
              name: flink-19-history-config-volume
            - mountPath: /data/flink/history
              name: persistent-storage
          ports:
            - containerPort: 8082
              name: http
      volumes:
        - name: persistent-storage
          persistentVolumeClaim:
            claimName: flink-19-history-pvc
        - name: flink-19-lib
          persistentVolumeClaim:
            claimName: flink-19-lib-pvc    
        - name: flink-19-history-config-volume
          configMap:
            name: flink-19-history-config
            items:
              - key: config.yaml
                path: config.yaml
              - key: log4j-console.properties
                path: log4j-console.properties
---
kind: Service
apiVersion: v1
metadata:
  name: flink-19-history-service
spec:
  selector:
    app: flink-19-history
  ports:
    - name: http
      protocol: TCP
      port: 8082
      targetPort: 8082
      nodePort: 30002
  type: NodePort
```  

## 执行部署  

http://192.168.0.143:30001/#/overview     

refer 
1.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/historyserver/        
2.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/logging/   