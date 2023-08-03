## Flink on K8s HA PVC Application 模式实战 

1.创建checkpoint pvc
创建checkpoint pvc yaml文件flink-checkpoint-application-pvc.yaml
vim flink-checkpoint-application-pvc.yaml

```yaml
# Flink checkpoint 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-checkpoint-application-pvc  # checkpoint pvc名称
  namespace: flink   # 指定归属的名命空间
spec:
  storageClassName: nfs-storage   #sc名称，更改为实际的sc名称
  accessModes:
    - ReadWriteMany   #采用ReadWriteMany的访问模式
  resources:
    requests:
      storage: 1Gi    #存储容量，根据实际需要更改
```

2.Application模式测试
提交作业
（1）编写application-deployment-checkpoint.yaml
vim application-deployment-checkpoint.yaml

```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: application-deployment-checkpoint  # flink 集群名称
spec:
  image: flink:1.13.6  # flink基础镜像
  flinkVersion: v1_13  # flink版本，选择1.13
  imagePullPolicy: IfNotPresent  # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
    state.checkpoints.dir: file:///opt/flink/checkpoints
  serviceAccount: flink
  jobManager:
    resource:
      memory: "1024m"
      cpu: 1
  taskManager:
    resource:
      memory: "1024m"
      cpu: 1
  podTemplate:
    spec:
      hostAliases:
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
            - name: flink-jar  # 挂载nfs上的jar
              mountPath: /opt/flink/jar
            - name: flink-checkpoints  # 挂载checkpoint pvc
              mountPath: /opt/flink/checkpoints
            - name: flink-log  # 挂载日志 pvc
              mountPath: /opt/flink/log
      volumes:
        - name: flink-jar
          persistentVolumeClaim:
            claimName: flink-jar-pvc
        - name: flink-checkpoints
          persistentVolumeClaim:
            claimName: flink-checkpoint-application-pvc
        - name: flink-log
          persistentVolumeClaim:
            claimName: flink-log-pvc
  job:

    jarURI: local:///opt/flink/jar/flink-learn-1.0-SNAPSHOT-jar-with-dependencies.jar  # 使用pv方式挂载jar包
    entryClass: com.yzhou.job.StreamWordCountWithCP
    args:   # 传递到作业main方法的参数
      - "k8s01:9092"
      - "first"
      - "k8s02"
      - "3306"
      - "flink_test"
      - "wc3"
      - "file:///opt/flink/checkpoints"
      - "10000"
      - "1"

    parallelism: 1
    upgradeMode: stateless
```



（2）提交
kubectl apply -f application-deployment-checkpoint.yaml

（3）查看集群创建情况
watch -n 1 "kubectl get all -n flink"

（4）访问Flink UI
http://flink.k8s.io:30502/flink/application-deployment-checkpoint/#/overview

（5）连接生产者，发送字符消息
kafka-console-producer.sh --bootstrap-server k8s01:9092,k8s02:9092,k8s03:9092 --topic first


2、终止和重启作业
（1）终止作业
kubectl delete -f application-deployment-checkpoint.yaml

（2）查看集群创建情况
kubectl get all -n flink

（3）编写重启作业yaml
vi application-deployment-checkpoint-resume.yaml

```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: application-deployment-checkpoint  # flink 集群名称
spec:
  image: flink:1.13.6  # flink基础镜像
  flinkVersion: v1_13  # flink版本，选择1.13
  imagePullPolicy: IfNotPresent  # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
    state.checkpoints.dir: file:///opt/flink/checkpoints
  serviceAccount: flink
  jobManager:
    resource:
      memory: "1024m"
      cpu: 1
  taskManager:
    resource:
      memory: "1024m"
      cpu: 1
  podTemplate:
    spec:
      hostAliases:
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
            - name: flink-jar  # 挂载nfs上的jar
              mountPath: /opt/flink/jar
            - name: flink-checkpoints  # 挂载checkpoint pvc
              mountPath: /opt/flink/checkpoints
            - name: flink-log  # 挂载日志 pvc
              mountPath: /opt/flink/log
      volumes:
        - name: flink-jar
          persistentVolumeClaim:
            claimName: flink-jar-pvc
        - name: flink-checkpoints
          persistentVolumeClaim:
            claimName: flink-checkpoint-application-pvc
        - name: flink-log
          persistentVolumeClaim:
            claimName: flink-log-pvc
  job:
    jarURI: local:///opt/flink/jar/flink-learn-1.0-SNAPSHOT-jar-with-dependencies.jar # 使用pv方式挂载jar包
    entryClass: com.yzhou.job.StreamWordCountWithCP
    args:   # 传递到作业main方法的参数
      - "k8s01:9092"
      - "first"
      - "k8s02"
      - "3306"
      - "flink_test"
      - "wc3"
      - "file:///opt/flink/checkpoints"
      - "10000"
      - "1"
    initialSavepointPath: /opt/flink/checkpoints/ffffffffdfda9fcf0000000000000001/chk-33/ # checkpoint文件绝对路径
    parallelism: 1
    upgradeMode: stateless
```



（2）提交
kubectl apply -f application-deployment-checkpoint.yaml

（3）查看集群创建情况
watch -n 1 "kubectl get all -n flink" 

（4）访问Flink UI
http://flink.k8s.io:32469/flink/application-deployment-checkpoint/#/overview

（5）连接生产者，发送字符消息
kafka-console-producer.sh --broker-list k8s01:9092 --topic first