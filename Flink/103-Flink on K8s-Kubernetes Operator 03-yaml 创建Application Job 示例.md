# Flink on Kubernetes - Kubernetes Operator -yaml 创建 Application Job 示例    

>Operator version: 1.8  

## 介绍 
在上一篇《Flink on K8s - Kubernetes Operator - yaml 创建 Session 集群》 Blog中介绍 Seesion 集群部署及Job 创建，我想你对Kubernetes 和 Operator 有了一定的认识。 接下来，我们来演练 Application Job 的部署 。   


## 编写作业 yaml    
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-application-deployment-only
spec:
  image: flink:1.17
  flinkVersion: v1_17
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
  serviceAccount: flink
  jobManager:
    resource:
      memory: "2048m"
      cpu: 1
  taskManager:
    resource:
      memory: "2048m"
      cpu: 1
  job:
    jarURI: local:///opt/flink/examples/streaming/StateMachineExample.jar
    parallelism: 2
    upgradeMode: stateless
```


