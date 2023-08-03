## Flink on K8s HA PVC Application 模式实战 

1、提交作业
（1）编写application-deployment-checkpoint-ha-hdfs.yaml

vim application-deployment-checkpoint-ha-hdfs.yaml

```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: application-deployment-checkpoint-ha-hdfs  # flink 集群名称
spec:
  image: flink-hdfs:1.13.6  # flink集成hadoop依赖包的镜像
  flinkVersion: v1_13  # flink版本，选择1.13
  imagePullPolicy: IfNotPresent  # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
    state.checkpoints.dir: hdfs:///flink-checkpoints  # checkpoint的路径
    kubernetes.hadoop.conf.config-map.name: apache-hadoop-conf
    high-availability.type: kubernetes
    high-availability: org.apache.flink.kubernetes.highavailability.KubernetesHaServicesFactory # JobManager HA
    high-availability.storageDir: hdfs:///flink-recovery  # JobManager HA数据保存路径
  serviceAccount: flink
  jobManager:
    replicas: 2  # HA下， jobManger的副本数要大于1
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
            - name: flink-log  # 挂载日志 pvc
              mountPath: /opt/flink/log
      volumes:
        - name: flink-jar
          persistentVolumeClaim:
            claimName: flink-jar-pvc
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
      - "wc7"
      - "hdfs:///flink-checkpoints"
      - "10000"
      - "1"
    parallelism: 1
    upgradeMode: stateless
```



2、提交
kubectl apply -f application-deployment-checkpoint-ha-hdfs.yaml

3、查看集群创建情况
watch -n 1 "kubectl get all -n flink"

4、访问Flink UI
http://flink.k8s.io:32469/flink/application-deployment-checkpoint-ha-hdfs/#/overview

5、连接生产者，发送字符消息
kafka-console-producer.sh --bootstrap-server k8s01:9092 --topic first

6、杀掉其中一个jobManager

