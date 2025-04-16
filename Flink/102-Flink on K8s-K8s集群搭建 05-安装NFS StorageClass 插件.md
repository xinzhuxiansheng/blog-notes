# Flink on Kubernetes - Kubernetes集群搭建 - 安装 NFS StorageClass 插件   

## 介绍 
在K8s上部署应用，大多数场景下需要使用持久化存储，通常采用PV+PVC的方式提供持久化存储。                   

PersistentVolume (PV) 是外部存储系统中的一块存储空间，`通常由管理员创建和维护`。与 Volume 一样，PV 具有持久性，生命周期独立于 Pod，也就是说，即使Pod被删除，PV的存储仍在。                        

PersistentVolumeClaim (PVC) 是对 PV 的申请 (Claim)。`PVC 通常由普通用户创建和维护`。需要为 Pod 分配存储资源时，用户可以创建一个 PVC，指明存储资源的容量大小和访问模式（比如只读）等信息，Kubernetes 会查找并提供满足条件的 PV。         

在一个大规模的 Kubernetes 集群里，可能有成千上万个PVC，这就意味着运维人员必须事先创建这些PV。为此，Kubernetes提供了一套可以自动创建PV的机制，即 Dynamic Provisioning，而这个机制的核心在于StorageClass这个API对象。Kubernetes能够根据用户提交的PVC，找到一个对应的 StorageClass，之后Kubernetes就会调用该StorageClass声明的存储插件，进而创建出需要的PV。                

在我们的集群中，StorageClass的存储插件使用NFS。                 

## 安装NFS（所有节点操作）
```shell
yum install -y nfs-utils 
```

###  在Master  k8s01 执行以下命令 
```shell
echo "/nfs/data/ *(insecure,rw,sync,no_root_squash)" > /etc/exports

# 执行以下命令，启动 nfs 服务;创建共享目录
$ mkdir -p /nfs/data

# 在Master k8s01 执行
systemctl enable rpcbind
systemctl enable nfs-server
systemctl start rpcbind
systemctl start nfs-server

# 使配置生效
$ exportfs -r

#检查配置是否生效
$ exportfs
```

### 配置NFS  Client，在k8s02~k8s06执行
```shell
showmount -e k8s01
mkdir -p /nfs/data
mount -t nfs k8s01:/nfs/data /nfs/data
```

### 安装 StorageClass (nfs-subdir-external-provisioner)
访问 https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner，了解如何为 nfs 创建 StorageClass。             

>安装StorageClass（注意，如果nfs服务不是安装在k8s01上，需要修改`nfs-sc.yaml`文件里的NFS_SERVER和server配置项，改为nfs服务所在的主机名）                              

可参考 https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner/tree/master/deploy 目录下，构建 yaml。                      
```shell
$ kubectl apply -f nfs-sc.yaml
```

output log:                     
```
[root@k8s01 ~]# kubectl apply -f nfs-sc.yaml 
storageclass.storage.k8s.io/nfs-storage created
deployment.apps/nfs-client-provisioner created
serviceaccount/nfs-client-provisioner created
clusterrole.rbac.authorization.k8s.io/nfs-client-provisioner-runner created
clusterrolebinding.rbac.authorization.k8s.io/run-nfs-client-provisioner created
role.rbac.authorization.k8s.io/leader-locking-nfs-client-provisioner created
rolebinding.rbac.authorization.k8s.io/leader-locking-nfs-client-provisioner created
[root@k8s01 ~]# 
```

```shell 
# 确认配置是否生效
$ kubectl get sc

# 卸载
$ kubectl delete -f nfs-sc.yaml  
```     

### nfs-sc.yaml 内容如下            
```yaml
## 创建了一个存储类
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: nfs-storage
  annotations:
    storageclass.kubernetes.io/is-default-class: "true"
provisioner: k8s-sigs.io/nfs-subdir-external-provisioner
parameters:
  archiveOnDelete: "true"  ## 删除pv的时候，pv的内容是否要备份

---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nfs-client-provisioner
  labels:
    app: nfs-client-provisioner
  # replace with namespace where provisioner is deployed
  namespace: default
spec:
  replicas: 1
  strategy:
    type: Recreate
  selector:
    matchLabels:
      app: nfs-client-provisioner
  template:
    metadata:
      labels:
        app: nfs-client-provisioner
    spec:
#      hostNetwork: true
      serviceAccountName: nfs-client-provisioner
      containers:
        - name: nfs-client-provisioner
          image: registry.cn-hangzhou.aliyuncs.com/cm_ns01/nfs-subdir-external-provisioner:v4.0.2
          # resources:
          #    limits:
          #      cpu: 10m
          #    requests:
          #      cpu: 10m
          volumeMounts:
            - name: nfs-client-root
              mountPath: /persistentvolumes
          env:
            - name: PROVISIONER_NAME
              value: k8s-sigs.io/nfs-subdir-external-provisioner
            - name: NFS_SERVER
              value: k8s01 ## 指定自己nfs服务器地址
            - name: NFS_PATH  
              value: /nfs/data  ## nfs服务器共享的目录
      volumes:
        - name: nfs-client-root
          nfs:
            server: k8s01
            path: /nfs/data
---
apiVersion: v1
kind: ServiceAccount
metadata:
  name: nfs-client-provisioner
  # replace with namespace where provisioner is deployed
  namespace: default
---
kind: ClusterRole
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: nfs-client-provisioner-runner
rules:
  - apiGroups: [""]
    resources: ["nodes"]
    verbs: ["get", "list", "watch"]
  - apiGroups: [""]
    resources: ["persistentvolumes"]
    verbs: ["get", "list", "watch", "create", "delete"]
  - apiGroups: [""]
    resources: ["persistentvolumeclaims"]
    verbs: ["get", "list", "watch", "update"]
  - apiGroups: ["storage.k8s.io"]
    resources: ["storageclasses"]
    verbs: ["get", "list", "watch"]
  - apiGroups: [""]
    resources: ["events"]
    verbs: ["create", "update", "patch"]
---
kind: ClusterRoleBinding
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: run-nfs-client-provisioner
subjects:
  - kind: ServiceAccount
    name: nfs-client-provisioner
    # replace with namespace where provisioner is deployed
    namespace: default
roleRef:
  kind: ClusterRole
  name: nfs-client-provisioner-runner
  apiGroup: rbac.authorization.k8s.io
---
kind: Role
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: leader-locking-nfs-client-provisioner
  # replace with namespace where provisioner is deployed
  namespace: default
rules:
  - apiGroups: [""]
    resources: ["endpoints"]
    verbs: ["get", "list", "watch", "create", "update", "patch"]
---
kind: RoleBinding
apiVersion: rbac.authorization.k8s.io/v1
metadata:
  name: leader-locking-nfs-client-provisioner
  # replace with namespace where provisioner is deployed
  namespace: default
subjects:
  - kind: ServiceAccount
    name: nfs-client-provisioner
    # replace with namespace where provisioner is deployed
    namespace: default
roleRef:
  kind: Role
  name: leader-locking-nfs-client-provisioner
  apiGroup: rbac.authorization.k8s.io
```  
