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