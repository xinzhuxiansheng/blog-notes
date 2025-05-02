# Flink on Kubernetes - Native Kubernetes - 创建 PVC 挂载 lib 和 plugin 目录

>Flink version: 1.17, Kubernetes version: 1.30.8       

## 创建 pvc   
vim flink-15-lib-pvc.yaml  
```yaml
#Flink 日志 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-15-lib-pvc  # 日志 pvc名称
  namespace: flink
spec:
  storageClassName: nfs-storage   #sc名称，更改为实际的sc名称
  accessModes:
    - ReadWriteMany   #采用ReadWriteMany的访问模式
  resources:
    requests:
      storage: 5Gi    #存储容量，根据实际需要更改
```

vim flink-15-plugins-pvc.yaml  
```yaml
#Flink 日志 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-15-plugins-pvc  # 日志 pvc名称
  namespace: flink
spec:
  storageClassName: nfs-storage   #sc名称，更改为实际的sc名称
  accessModes:
    - ReadWriteMany   #采用ReadWriteMany的访问模式
  resources:
    requests:
      storage: 5Gi    #存储容量，根据实际需要更改
```

**执行 create**
```shell
[root@master01 pvc]# kubectl create -f flink-15-lib-pvc.yaml -n flink
persistentvolumeclaim/flink-15-lib-pvc created
[root@master01 pvc]# kubectl create -f flink-15-plugins-pvc.yaml -n flink
persistentvolumeclaim/flink-15-plugins-pvc created
```

## 使用 busybox 挂载 pvc & 上传 file  

**busybox-pod.yaml**  
```shell
apiVersion: v1
kind: Pod
metadata:
  name: busybox-pod
  namespace: flink
  labels:
    app: busybox
spec:
  containers:
  - name: busybox
    image: busybox:latest
    command:
      - sleep
      - "3600"
    volumeMounts:  # 添加卷挂载配置
    - name: plugins-pvc  # 引用第一个PVC卷名称
      mountPath: /opt/plugins  # 挂载到容器内的路径（可根据需要修改）
    - name: lib-pvc      # 引用第二个PVC卷名称
      mountPath: /opt/lib    # 挂载到容器内的路径（可根据需要修改）
  volumes:  # 定义Pod级别的卷
  - name: plugins-pvc  # 卷名称（与volumeMounts对应）
    persistentVolumeClaim:
      claimName: flink-15-plugins-pvc  # 关联已创建的PVC
  - name: lib-pvc      # 卷名称（与volumeMounts对应）
    persistentVolumeClaim:
      claimName: flink-15-lib-pvc      # 关联已创建的PVC
```

```shell
kubectl apply -f busybox-pod.yaml -n flink
```

## 上传依赖文件  
使用 `kubectl cp` 命令，将 lib/,plugins 目录下文件拷贝到 PVC 挂载的目录下去。   