# 存储 - minio + juicefs 给 Kubernetes 提供存储   

>主要是 juicefs csi driver & storageclass     

## 部署 juicefs  

1.设置 juicefs helm仓库地址  
```bash
helm repo add juicefs https://juicedata.github.io/charts/
helm repo update   
```

2.配置 juicefs 和 minio 
cat values-mycluster.yaml  
```yaml
[root@master01 juicefs]# cat values-mycluster.yaml
global:
  # JuiceFS 挂载的元数据地址
  redis:
    address: "192.168.0.201:6379"  # Redis 服务的地址
    #password: "your-redis-password"  # 根据需要修改，如果没有密码可以删除此行

csi:
  storageClass:
    name: "juicefs-sc"  # 存储类名称
    reclaimPolicy: "Retain"  # 保留策略，根据需求选择 "Delete" 或 "Retain"
    volumeBindingMode: "Immediate"  # 绑定模式，可以选择 "WaitForFirstConsumer"

  enableNodePublish: true

  volume:
    accessMode: "ReadWriteMany"  # 访问模式，根据实际需求选择

  # MinIO 配置
  juicefs:
    minio:
      endpoint: "http://192.168.0.201:9000"  # MinIO 的 endpoint
      accessKey: "admin"  # MinIO 访问密钥
      secretKey: "admin@minio"  # MinIO 秘密密钥
      bucket: "juicefs-k8s"  # JuiceFS 使用的存储桶
      # 其他可能需要的参数
      #insecure: false  # 如果使用 HTTPS，设置为 false；否则设置为 true
      #region: "us-east-1"  # 根据你的配置选择区域
      # 其他配置可以根据需要添加
[root@master01 juicefs]#
```

```shell
helm upgrade --install juicefs-csi-driver juicefs/juicefs-csi-driver -n kube-system -f ./values-mycluster.yaml
```

3.设置 juicefs secret  
cat juicefs-secret.yaml
```bash
[root@master01 juicefs]# cat juicefs-secret.yaml
apiVersion: v1
kind: Secret
metadata:
  name: juicefs-secret
  namespace: default
  labels:
    # 增加该标签以启用认证信息校验
    juicefs.com/validate-secret: "true"
type: Opaque
stringData:
  name: k8sjuicefs
  metaurl: 192.168.0.201:6379
  storage: minio  # 指定存储类型为 MinIO
  bucket: http://192.168.0.201:9000/juicefs-k8s
  access-key: admin
  secret-key: admin@minio
[root@master01 juicefs]#
``` 

4.设置 juicefs storageclass
cat storage-class.yaml
```bash
[root@master01 juicefs]# cat storage-class.yaml
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: juicefs-sc
provisioner: csi.juicefs.com
parameters:
  csi.storage.k8s.io/provisioner-secret-name: juicefs-secret
  csi.storage.k8s.io/provisioner-secret-namespace: default
  csi.storage.k8s.io/node-publish-secret-name: juicefs-secret
  csi.storage.k8s.io/node-publish-secret-namespace: default
reclaimPolicy: Retain
```

## 测试  

1.创建 pvc 
```yaml
[root@master01 busybox]# cat juicefs-pvc.yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: busybox-juicefs-pvc
spec:
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 1Gi  # 申请 1Gi 的存储
  storageClassName: juicefs-sc  # 你之前创建的 JuiceFS StorageClass 名称
```

2.创建 busybox pod  
```yaml
[root@master01 busybox]# cat test-pod.yaml
apiVersion: v1
kind: Pod
metadata:
  name: juicefs-test-pod
spec:
  containers:
  - name: test-container
    image: busybox
    command: [ "sleep", "3600" ]
    volumeMounts:
    - name: juicefs-vol
      mountPath: /mnt/juicefs
  volumes:
  - name: juicefs-vol
    persistentVolumeClaim:
      claimName: busybox-juicefs-pvc
```


refer           
1.https://juicefs.com/docs/zh/csi/getting_started/    
