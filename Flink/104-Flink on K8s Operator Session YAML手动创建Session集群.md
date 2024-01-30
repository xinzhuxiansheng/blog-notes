## Flink K8s Operator YAML 手动创建 Session 集群

>Operator version: 1.5.0   

>注意：若需删除 Seesion 集群，请先将 kind 为 FlinkSessionJob 下的 job删除 ！！！ 

### 创建 Session集群  
1.编写 session-deployment-only.yaml 
```
# Flink Session集群
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: session-deployment-only
spec:
  image: flink:1.15.4
  flinkVersion: v1_15
  imagePullPolicy: IfNotPresent   # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
  serviceAccount: flink
  jobManager:
    replicas: 1
    resource:
      memory: "1024m"
      cpu: 1
  taskManager:
    replicas: 1
    resource:
      memory: "1024m"
      cpu: 1
```

2.提交
kubectl -f session-deployment-only.yaml

3.查看集群创建情况
kubectl get all -n flink

4.配置hosts域名映射
192.168.0.44 flink.k8s.io

5.访问Flink UI
http://flink.k8s.io:32469/flink/session-deployment-only/#/overview

6.上传Jar包，提交作业
Entry Class: com.yale.StreamWordCount
Parallelism: 1  

