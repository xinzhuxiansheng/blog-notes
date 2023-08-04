一 Flink内存介绍

1、Flink内存官方说明文件
https://nightlies.apache.org/flink/flink-docs-release-1.16/docs/deployment/memory/mem_setup/

2、Flink内存模型简介
Flink 的内存模型划分为进程总内存和Flink总内存，其中进程总内存（Total Process Memory）包含了由 Flink 应用使用的内存（Flink 总内存）以及由运行 Flink 的 JVM 使用的内存。 Flink 总内存（Total Flink Memory）包括 JVM 堆内存（Heap Memory）和堆外内存（Off-Heap Memory）。 其中堆外内存包括直接内存（Direct Memory）和本地内存（Native Memory）。

Flink's process memory model


TaskManager内存模型， 也是包括堆内存和堆外内存
Simple memory model

下表中列出了 Flink TaskManager 内存模型的所有组成部分，以及影响其大小的相关配置参数。
 组成部分
	
 配置参数
	
 描述
 框架堆内存（Framework Heap Memory）
	
 taskmanager.memory.framework.heap.size
	
 用于flink框架的JVM堆内存
 任务堆内存（Task Heap Memory）
	
 taskmanager.memory.task.heap.size
	
 用于flink应用的算子和用户代码的JVM堆内存
 托管内存（Managed Memory）
	
 taskmanager.memory.managed.size
 taskmanager.memory.managed.fraction
	
 由flink管理的用于排序、哈希表、缓存中间结果及Rocks DB State Backend的本地内存
 框架堆外内存（Framework Off-Heap Memory）
	
 taskmanager.memory.framework.off-heap.size
	
 用于flink框架的堆外内存
 任务堆外内存（Task Off-Heap Memory）
	
 taskmanager.memory.task.off-heap.size
	
 用于flink应用的算子及用户代码的堆外内存
 网络内存（Network Memory）
	
 taskmanager.memory.network.min
 taskmanager.memory.network.max
 taskmanager.memory.network.fraction
	
 用于任务之间数据传输的直接内存（例如网络传输缓冲），该内存为基于flink总内存的受限等比内存
 JVM Metaspace
	
 taskmanager.memory.jvm-metaspace.size
	
 Flink JVM进程的Metaspace
 JVM开销
	
 taskmanager.memory.jvm-overhead.min
 taskmanager.memory.jvm-overhead.max
 taskmanager.memory.jvm-overhead.fraction
	
 用于其他JVM开销的本地内存，例如栈空间、垃圾回收空间

二、Flink On K8s默认内存情况

jvm_params: -Xmx469762048 -Xms469762048 -XX:MaxMetaspaceSize=268435456
dynamic_configs: -D jobmanager.memory.off-heap.size=134217728b -D jobmanager.memory.jvm-overhead.min=201326592b -D jobmanager.memory.jvm-metaspace.size=268435456b -D jobmanager.memory.heap.size=469762048b -D jobmanager.memory.jvm-overhead.max=201326592b




  
三、修改Flink On K8s 内存
（1）编写session-deployment-only-ha-memory.yaml
vi session-deployment-only-ha-memory.yaml

# Flink Session集群
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: session-deployment-only-ha-memory
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
    jobmanager.memory.heap.size: 1024mb
    jobmanager.memory.jvm-metaspace.size: 384mb
    jobmanager.memory.off-heap.size: 256mb
    jobmanager.memory.jvm-overhead.min: 128mb
    jobmanager.memory.jvm-overhead.max: 896mb
    jobmanager.memory.jvm-overhead.fraction: "0.1"
    taskmanager.memory.task.heap.size: 1024mb  # 用于flink应用的算子和用户代码的JVM堆内存
    taskmanager.memory.managed.size: 256mb     # 由flink管理的用于排序、哈希表、缓存中间结果及Rocks DB State Backend的本地内存
    taskmanager.memory.task.off-heap.size: 256mb  # 用于flink应用的算子及用户代码的堆外内存
    taskmanager.memory.jvm-metaspace.size: 384mb  # Flink JVM进程的Metaspace
    taskmanager.memory.jvm-overhead.min: 256mb    # 用于其他JVM开销的本地内存，例如栈空间、垃圾回收空间
    taskmanager.memory.jvm-overhead.max: 896mb
    taskmanager.memory.jvm-overhead.fraction: "0.1"
  serviceAccount: flink
  jobManager:
    replicas: 2  # HA下， jobManger的副本数要大于1
    resource:
      memory: "2048m"
      cpu: 1
  taskManager:
    replicas: 1
    resource:
      memory: "3096m"
      cpu: 1
  podTemplate:  # podTemplate使用的是kubernetes自带的语法，可以配置hosts和挂载卷等资源
    spec:
      hostAliases:  # hosts配置
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



2）提交
# 先在k8s01上运行nc
nc -lk 7777
# 提交作业
kubectl apply -f session-deployment-only-ha-menory.yaml

（3）查看集群创建情况
kubectl get all -n flink -owide

（4）访问Flink UI
http://flink.k8s.io:30502/flink/session-deployment-only-ha-memory/#/overview

（5）提交作业
Entry Class: com.yale.StreamWordCount
Parallelism: 1