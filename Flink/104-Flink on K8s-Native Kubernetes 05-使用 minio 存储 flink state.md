# Flink on Kubernetes - Native Kubernetes - 使用 Minio 存储 Flink State 

>Flink version: 1.15.4, Kubernetes version: 1.30.8，minio version: RELEASE.2025-04-08T15-41-24Z

## 添加 S3 配置 
在 `flink-conf.yaml` 添加 s3 存储, 示例如下：    
```bash
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
# ......s3......
#==============================================================================
s3.access-key: minio
s3.secret-key: minio123
s3.path.style.access: true
s3.endpoint: http://192.168.0.135:9000
fs.allowed-fallback-filesystems: s3

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
```

### 注意  
1.s3.access-key: minio,s3.secret-key: minio123 这两个参数对应的是 Minio 部署时配置的环境变量，内容如下：      
```shell
vim /etc/profile

export MINIO_ACCESS_KEY=minio
export MINIO_SECRET_KEY=minio123  

source /etc/profile
```

2.s3.endpoint 参数配置的端口，指向的是 minio 启动时指定的 `--address` 参数。  
```shell
# minio 启动命令
/usr/local/bin/minio server --address :9000 --console-address :9001 /root/minio/data 
``` 

## 添加 flink-s3-fs-hadoop-1.15.4.jar 
使用 PVC 方式在 flink docker 镜像 `/opt/flink/lib` 目录下，添加 `flink-s3-fs-hadoop-1.15.4.jar` jar