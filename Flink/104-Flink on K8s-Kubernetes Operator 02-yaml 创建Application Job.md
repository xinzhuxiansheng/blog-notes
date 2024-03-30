# Flink on K8s - Kubernetes Operator - yaml 创建 Session 集群     

>Operator version: 1.8.0    

## 介绍 
请访问  https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/custom-resource/overview/#application-deployments ,了解 Seesion yaml的编写示例； 

>建议在没有实操之前，先了解上面官网链接的内容, 别因为字少，忽略了 `Further information` 章节      
```bash     
   Further information
    Job Management and Stateful upgrades(https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/custom-resource/job-management/)

    Deployment customization and pod templates(https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/custom-resource/pod-template/)

    Full Reference(https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/custom-resource/reference/)     

    Examples(https://github.com/apache/flink-kubernetes-operator/tree/main/examples)    
```   

Flink Kubernetes Operator 定义了 `FlinkDeployment` 的资源。       

## 编写 Seesion 集群 yaml   
请参考官网提供的示例`Examples` (https://github.com/apache/flink-kubernetes-operator/tree/main/examples) 创建 Seesion 集群, `注意在 yaml中如何识别是 Seesion集群 还是 Application Job，可根据 yaml中是否包含 spec.job项配置`。          



### 仅创建Seesion集群 
```bash 
The Flink Deployment configuration contains the following:

    The name of the Flink Deployment
    The resources available for the Flink Deployment

The Flink Deployment configuration does NOT contain the following:

    The job to run
    Any job specific configurations
```

yaml 内容如下：   
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-session-deployment-only-example
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
```

```bash
kubeclt -n flink apply -f basic-session-deployment-only.yaml    
```

#### 向 Seesion集群添加作业  
```bash
For an existing Flink Deployment another configuration could be used to create new jobs. This configuration should contain the following:

    The Flink Deployment to use
    The job to run
    Any job specific configurations
```   

yaml 内容如下：     
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkSessionJob
metadata:
  name: basic-session-job-only-example
spec:
  deploymentName: basic-session-deployment-only-example
  job:
    jarURI: https://repo1.maven.org/maven2/org/apache/flink/flink-examples-streaming_2.12/1.16.1/flink-examples-streaming_2.12-1.16.1-TopSpeedWindowing.jar
    parallelism: 4
    upgradeMode: stateless  
```

```bash
kubeclt -n flink apply -f basic-session-job-only.yaml  
```

### Seesion 集群 & Job 一起创建  
将Seesion 集群 和 Job配置 一起提交到 Kubernetes。      
```bash
kubectl apply -f basic-session-deployment-and-job.yaml    
```


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



refer    
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/custom-resource/overview/#application-deployments      