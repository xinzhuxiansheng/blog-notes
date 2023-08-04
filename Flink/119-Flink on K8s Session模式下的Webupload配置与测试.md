## Flink on K8s Session模式下的Webupload配置与测试.md 


webupload用途
配置webupload，用于在session模式的场景下，持久化保存用户上传的Flink 作业jar包， 或第三方调度系统通过REST API上传的Jar包，以便在Session集群重启时能复用，而不是每次重启后都要重新上传。



 

1、创建Flink Webupload PVC
（1）编写flink-webupload-pvc.yaml

vim flink-webupload-pvc.yaml

```yaml
#Flink Webupload 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-webpuload-pvc  # ha pvc名称
  namespace: flink
spec:
  storageClassName: nfs-storage   #sc名称，更改为实际的sc名称
  accessModes:
    - ReadWriteMany   #采用ReadWriteMany的访问模式
  resources:
    requests:
      storage: 1Gi    #存储容量，根据实际需要更改

```



（2）创建HA pvc
kubectl apply -f webupload-ha-pvc.yaml

（3）查看pvc
kubectl get pvc -n flink

2、创建Session集群
（1）编写作业session-deployment-only-ha-webupload.yaml
vi session-deployment-only-ha-webupload.yaml

```yaml
# Flink Session集群
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: session-deployment-only-ha-webpuload
spec:
  image: flink:1.13.6
  flinkVersion: v1_13
  imagePullPolicy: IfNotPresent   # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
    high-availability.type: kubernetes
    high-availability: org.apache.flink.kubernetes.highavailability.KubernetesHaServicesFactory # JobManager HA
    high-availability.storageDir: file:///opt/flink/flink_recovery  # JobManager HA数据保存路径
    web.upload.dir: /opt/flink/webupload  # 上传jar包的保存目录. 如果不指定，则会保存在web.tmpdir
    web.tmpdir: /opt/flink/webupload  # Local directory that is used by the REST API for temporary files
  serviceAccount: flink
  jobManager:
    replicas: 2  # HA下， jobManger的副本数要大于1
    resource:
      memory: "1024m"
      cpu: 1
  taskManager:
    replicas: 1
    resource:
      memory: "1024m"
      cpu: 1
  podTemplate:  # podTemplate使用的是kubernetes自带的语法，可以配置hosts和挂载卷等资源
    spec:
      hostAliases:  # hosts配置
        - ip: "192.168.0.140"
          hostnames:
            - "k8s01"
        - ip: "192.168.0.141"
          hostnames:
            - "k8s02"
        - ip: "192.168.0.142"
          hostnames:
            - "k8s03"
        - ip: "192.168.0.143"
          hostnames:
            - "k8s04"
        - ip: "192.168.0.144"
          hostnames:
            - "k8s05"
        - ip: "192.168.0.145"
          hostnames:
            - "k8s06"
      containers:
        - name: flink-main-container
          env:
            - name: TZ
              value: Asia/Shanghai
          volumeMounts:
            - name: flink-logs  # 日志 pvc配置
              mountPath: /opt/flink/log
            - name: flink-ha    # HA pvc配置
              mountPath: /opt/flink/flink_recovery
            - name: flink-webupload  # web upload pvc配置
              mountPath: /opt/flink/webupload
      volumes:  # 挂载卷配置
        - name: flink-logs
          persistentVolumeClaim:
            claimName: flink-log-pvc
        - name: flink-ha
          persistentVolumeClaim:
            claimName: flink-ha-pvc
        - name: flink-webupload
          persistentVolumeClaim:
            claimName: flink-webpuload-pvc
```



（2）提交
# 先在k8s01上运行nc
nc -lk 7777

# 提交作业
kubectl apply -f session-deployment-only-ha-webupload.yaml

（3）查看集群创建情况
kubectl get all -n flink -owide

（4）访问Flink UI
http://flink.k8s.io:30502/flink/session-deployment-only-ha-webpuload/#/overview

（5）上传Jar包，提交作业
Entry Class: com.yale.StreamWordCount
Parallelism: 1

（6）查看上传Jar包的实际保存位置
ll /nfs/data/flink-flink-webpuload-pvc-pvc-f9e1b036-0635-4930-a113-929ffd24f888/flink-web-upload/

（7）查看作业输出结果
k logs session-deployment-only-ha-webpuload-taskmanager-1-1 -n flink

（8）停止Flink作业，并重新启动，查看上传的Jar包是否仍在
# 查看运行中的作业（集群）
kubectl get FlinkDeployment -n flink

# 终止作业（集群）
kubectl delete FlinkDeployment session-deployment-only-ha-webpuload -n flink

