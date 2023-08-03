## Flink on K8s HA PVC Session 模式实战 

1.Flink HA 状态数据保存有2种方式，一种是保存到PV，另一种是保存到HDFS，也可以保存到S3对象存储



2.创建Flink HA PVC
（1）编写flink-ha-pvc.yaml
vim flink-ha-pvc.yaml

```yaml
#Flink HA 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-ha-pvc  # ha pvc名称
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
kubectl apply -f flink-ha-pvc.yaml  

（3）查看pvc
kubectl get pvc -n flink

3.创建Session集群
（1）创建集群
编写作业session-deployment-only-checkpoint-ha.yaml
vim session-deployment-only-checkpoint-ha.yaml

```yaml
# Flink Session集群
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: session-deployment-only-checkpoint-ha
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
    state.checkpoints.dir: file:///opt/flink/checkpoints  # checkpoint的默认路径
    high-availability.type: kubernetes
    high-availability: org.apache.flink.kubernetes.highavailability.KubernetesHaServicesFactory # JobManager HA
    high-availability.storageDir: file:///opt/flink/flink_recovery  # JobManager HA数据保存路径
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
            - name: flink-checkpoints  # checkpoint pvc配置
              mountPath: /opt/flink/checkpoints
            - name: flink-logs  # 日志 pvc配置
              mountPath: /opt/flink/log
            - name: flink-ha    # HA pvc配置
              mountPath: /opt/flink/flink_recovery
      volumes:  # 挂载卷配置
        - name: flink-checkpoints
          persistentVolumeClaim:
            claimName: flink-checkpoint-session-pvc
        - name: flink-logs
          persistentVolumeClaim:
            claimName: flink-log-pvc
        - name: flink-ha
          persistentVolumeClaim:
            claimName: flink-ha-pvc
```


（2）提交
kubectl apply -f session-deployment-only-checkpoint-ha.yaml

（3）查看集群创建情况
kubectl get all -n flink

（4）访问Flink UI
http://flink.k8s.io:32469/flink/session-deployment-only-checkpoint-ha/#/overview

4.提交作业，HTTP下载Jar包方式
（1）编写session-job-only-checkpoint-ha.yaml
vim session-job-only-checkpoint-ha.yaml

```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkSessionJob
metadata:
  namespace: flink
  name: session-job-only-checkpoint-ha
spec:
  deploymentName: session-deployment-only-checkpoint-ha  # 需要与创建的集群名称一致
  job:
    jarURI: http://192.168.0.140:8080/jars/flink/flink-learn-1.0-SNAPSHOT-jar-with-dependencies.jar # 使用http方式下载jar包
    entryClass: com.yzhou.job.StreamWordCountWithCP
    args:   # 传递到作业main方法的参数
      - "k8s01:9092"
      - "first"
      - "k8s02"
      - "3306"
      - "flink_test"
      - "wc5"
      - "file:///opt/flink/checkpoints"
      - "10000"
      - "1"
    parallelism: 1  # 并行度
    upgradeMode: stateless
```
    


（2）将Jar包上传到tomcat目录下，并启动tomcat，使用k8s06上的tomcat 
http://k8s01:8080/jars/flink/

（3）提交
kubectl apply -f session-job-only-checkpoint-ha.yaml

（4）查看集群创建情况
kubectl get all -n flink

（5）访问Flink UI
http://flink.k8s.io:30502/flink/session-deployment-only-checkpoint-ha/#/overview

（6）连接生产者，发送字符消息
kafka-console-producer.sh --broker-list k8s01:9092,k8s02:9092,k8s03:9092 --topic first

5.杀掉其中一个jobManager
kubectl delete po session-deployment-only-checkpoint-ha-8569cc6b5d-2zhbj -n flink