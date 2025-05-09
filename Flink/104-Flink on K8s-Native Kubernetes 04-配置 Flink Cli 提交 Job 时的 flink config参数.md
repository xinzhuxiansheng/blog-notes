# Flink on Kubernetes - Native Kubernetes - 配置 Flink Cli 提交 Job 时的 flink config参数

>Flink version: 1.17, Kubernetes version: 1.30.8       

>后面参数务必参考该篇 Blog 

## 在 flink-conf.yaml
```bash
# 下面是 yzhou 添加的
env.java.opts.jobmanager: -Duser.timezone=GMT+08
env.java.opts.taskmanager: -Duser.timezone=GMT+08

akka.ask.timeout: 100s
taskmanager.slot.timeout: 100s

#==============================================================================
# ......flink docker url............
#==============================================================================
kubernetes.container.image: 192.168.0.134:5000/flink:1.15.4-java11

#==============================================================================
# ......State............
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
# ......S3......
#==============================================================================
s3.access-key: minio
s3.secret-key: minio123
s3.path.style.access: true
s3.endpoint: http://192.168.0.135:9000
fs.allowed-fallback-filesystems: s3


#==============================================================================
# ......HistoryServer......
#==============================================================================
jobmanager.archive.fs.dir: s3://flink-15-state/completed-jobs/
```

## shell 参数  
/root/yzhou/flink/flink-1.15.4/bin/flink  run-application \
    --target kubernetes-application \
    -Dkubernetes.namespace=flink \
    -Dkubernetes.service-account=flink-service-account \
    -Dkubernetes.cluster-id=flink-application-test \
    -Dkubernetes.pod-template-file.jobmanager=./jobmanager-pod-template.yaml \
    -Dkubernetes.pod-template-file.taskmanager=./taskmanager-pod-template.yaml \
    -Dclassloader.resolve-order=parent-first \
    -Dkubernetes.rest-service.exposed.type=NodePort \
    -c com.yzhou.flink.example.Kafka2Console \
    local:///opt/flink/artifacts/app.jar
 