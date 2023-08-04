## Flink on K8s Job调度到指定节点运行 

Flink和业务系统的Pod分散在K8s集群各个节点
 
Flink和业务系统的Pod调度到K8s集群的指定节点上


 
一、设置节点标签
1、 查看节点标签
kubectl get nodes --show-labels

2、给节点打标签
kubectl label nodes k8s03 flink.node=true
kubectl label nodes k8s04 flink.node=true

二、测试演示，采用Application模式
1、编写application-deployment-schedule.yaml（ taskManger.replicas=1, taskmanager.numberOfTaskSlots=5， job.parallelism=1，args.parallelism=10）
cd /root/flink-operator/flink-kubernetes-operator-1.3.1/examples
vi application-deployment-schedule.yaml

apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: application-deployment-schedule  # flink 集群名称
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
    taskmanager.numberOfTaskSlots: "5"
    state.checkpoints.dir: file:///opt/flink/checkpoints
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
  podTemplate:
    spec:
      hostAliases:
        - ip: "192.168.56.80"
          hostnames:
            - "k8s01"
        - ip: "192.168.56.81"
          hostnames:
            - "k8s02"
        - ip: "192.168.56.82"
          hostnames:
            - "k8s03"
        - ip: "192.168.56.83"
          hostnames:
            - "k8s04"
        - ip: "192.168.56.84"
          hostnames:
            - "k8s05"
        - ip: "192.168.56.85"
          hostnames:
            - "k8s06"
      nodeSelector:
        flink.node: "true"
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
    entryClass: com.yale.ParallelismTest
    args:   # 传递到作业main方法的参数
      - "k8s01:9092,k8s02:9092,k8s03:9092"
      - "topic-1,topic-2"
      - "k8s02"
      - "3306"
      - "flink_test"
      - "wc1,wc2"
      - "file:///opt/flink/checkpoints"
      - "10000"
      - "10"
    parallelism: 1
    upgradeMode: stateless


（2）提交作业
kubectl apply -f application-deployment-schedule.yaml

（3）查看
kubectl get po -n flink -owide
http://flink.k8s.io:30502/flink/application-deployment-schedule/#/overview