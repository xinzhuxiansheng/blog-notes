## Flink on K8s HA ZK Application 模式实战 


一、Flink HA Zookeeper服务实现方式测试
1、提交作业
（1）编写application-deployment-checkpoint-ha-zk.yaml
vi application-deployment-checkpoint-ha-zk.yaml

apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: application-deployment-checkpoint-ha-zk  # flink 集群名称
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
    state.checkpoints.dir: file:///opt/flink/checkpoints  # checkpoint的路径
    high-availability: ZOOKEEPER
    high-availability.zookeeper.quorum: 192.168.0.140:2181,192.168.0.141:2181
    high-availability.zookeeper.path.root: /flink
    high-availability.storageDir: file:///opt/flink/flink_recovery  # JobManager HA数据保存路径
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
            - name: flink-checkpoints  # 挂载checkpoint pvc
              mountPath: /opt/flink/checkpoints
            - name: flink-log  # 挂载日志 pvc
              mountPath: /opt/flink/log
            - name: flink-ha    # HA pvc配置
              mountPath: /opt/flink/flink_recovery
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
        - name: flink-ha
          persistentVolumeClaim:
            claimName: flink-ha-pvc
  job:
    jarURI: local:///opt/flink/jar/flink-on-k8s-demo-1.0-SNAPSHOT-jar-with-dependencies.jar # 使用pv方式挂载jar包
    entryClass: com.yale.StreamWordCountWithCP
    args:   # 传递到作业main方法的参数
      - "k8s01:9092,k8s02:9092,k8s03:9092"
      - "first"
      - "k8s02"
      - "3306"
      - "flink_test"
      - "wc8"
      - "file:///opt/flink/checkpoints"
      - "10000"
      - "1"
    parallelism: 1
    upgradeMode: stateless



2、提交
（1）先启动zookeeper和kafka
zk.sh start
kf.sh start
（2）提交作业
kubectl apply -f application-deployment-checkpoint-ha-zk.yaml

3、查看集群创建情况
watch -n 1 "kubectl get all -n flink"

4、访问Flink UI
http://flink.k8s.io:30502/flink/application-deployment-checkpoint-ha-zk/#/overview

5、连接生产者，发送字符消息
kafka-console-producer.sh --broker-list k8s01:9092,k8s02:9092,k8s03:9092 --topic first

6、杀掉其中一个jobManager

二、两种HA实现方式的选择
对于Kubernetes的服务实现方式, jobmanager的高可用状态信息保存在ConfigMap里, jobmanager的主备选举和状态监控需要借助监控k8s的configmap来实现, 这个过程会不断地与K8s的apiserver进行调用交互, 当Flink作业较多的时候, 会对K8s造成一定的压力. 反过来, 在实际应用中, K8s是一个基础的容器运行平台, 它上面除了运行Flink作业外, 还会运行其他应用的Pod, 当K8s集群压力过大时, Flink HA与K8s ApiServer的交互会受到影响, 从而影响Flink HA, 尤其在k8s资源非常紧张, 负载压力很大的情况下, 会导致jobmanager 主从切换失败, 最终导致Flink作业异常终止.