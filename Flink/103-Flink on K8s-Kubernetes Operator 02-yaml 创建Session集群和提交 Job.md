# Flink on K8s - Kubernetes Operator - yaml 创建 Session 集群     

>Operator version: 1.8  

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

>注意，此时Seesion 集群会自动创建 2个 `Service`资源:    
`basic-session-deployment-only-example`：这个 Service 通常用于 Flink JobManager 和 TaskManager 之间的内部通信。它暴露了 6123 和 6124 端口，这些端口分别用于不同的通信目的，如数据传输和管理操作。ClusterIP 类型的服务意味着它只能在集群内部访问，None 作为 ClusterIP 指出这是一个 headless service，用于直接暴露 pod 的 IP 地址而不是通过单一的服务 IP 进行负载均衡。       

`basic-session-deployment-only-example-rest`：这个 Service 是为了访问 Flink 的 REST API 而设置的，主要是用于提交作业、查询作业状态等操作。它暴露了 8081 端口，这是 Flink Dashboard 和 REST API 的默认端口。同样是 ClusterIP 类型，意味着这个服务也仅在集群内部可访问。不同于上一个服务，这个服务有一个分配的 ClusterIP（例如 10.96.185.175），它提供了一个稳定的内部 IP 地址来访问服务。        

#### NodePort Service yaml 
若方便在宿主机访问 Seesion:8081 页面，我需创建一个 使用 NodePort暴露端口的 Service。      

`basic-session-deployment-only-service.yaml` 内容如下：   
```bash
apiVersion: v1
kind: Service
metadata:
  name: basic-session-deployment-only-example-rest-nodeport
spec:
  type: NodePort
  ports:
    - port: 8081
      targetPort: 8081
      nodePort: 30081
      name: result
  selector:
    app: basic-session-deployment-only-example
    component: jobmanager
    type: flink-native-kubernetes
```

output log: 
```bash
[root@k8s01 job_yaml]# kubectl get svc -n flink 
NAME                                                  TYPE        CLUSTER-IP      EXTERNAL-IP   PORT(S)             AGE
basic-session-deployment-only-example                 ClusterIP   None            <none>        6123/TCP,6124/TCP   6h3m
basic-session-deployment-only-example-rest            ClusterIP   10.96.185.175   <none>        8081/TCP            6h3m
basic-session-deployment-only-example-rest-nodeport   NodePort    10.96.107.159   <none>        8081:30081/TCP      5h19m
flink-operator-webhook-service                        ClusterIP   10.96.189.126   <none>        443/TCP             11h
```

此时，可通过 虚机IP + 30081， 访问页面。    


#### 此 Seesion 与 Standalone Seesion 不同  
在 Standalone Seesion集群下，TaskManager是一起部署好的， 而 Kubernetes Operator 的 Seesion Mode，仅仅只启动 JobManager，而 TaskManager 会随着申请资源大小而创建，并不受 JobManager限制。      

这里特别注意： Operator 的 Seesion下 创建的 Job，它对应的资源是 sessionjob。    

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

output log:   
```bash     
basic-session-deployment-only-example-taskmanager-1-1    1/1     Running   0               2m47s
basic-session-deployment-only-example-taskmanager-1-2    1/1     Running   0               2m47s
```

下面是对 `upgradeMode: stateless`参数介绍    
```
在 Flink Kubernetes Operator 中，`upgradeMode` 配置项定义了如何处理 Flink 应用的升级过程。当设置为 `stateless` 时，这意味着在进行升级时，当前正在运行的 Flink 应用的状态不会被保留。换句话说，当应用升级发生时，任何现有的作业状态都不会被迁移到新的版本。这种升级模式通常用于那些不需要保存状态，或者可以接受从头开始处理的应用场景。

### `stateless` 升级模式的特点包括：

- **快速部署**：由于不需要迁移状态，`stateless` 模式允许快速部署新版本的 Flink 应用。
- **简化操作**：这种模式简化了升级过程，因为操作员不需要担心状态的保存和恢复。
- **潜在的数据丢失**：如果应用的状态对业务逻辑很重要，使用 `stateless` 升级可能导致状态信息丢失。因此，这种模式不适合那些需要精确状态管理的应用。

### 应用场景

- **无状态计算**：对于那些处理实时数据流但不需要保持长期状态的 Flink 应用，`stateless` 升级是合适的。例如，一个简单的实时监控系统可能适用于这种模式。
- **开发和测试**：在开发过程中，频繁地迭代和部署新版本可能更加看重部署速度而不是状态的保持。`stateless` 模式可以在这种情况下提供便利。
- **可接受数据处理重启的应用**：对于某些场景，即使在升级后从最近的检查点或保存点重新开始处理，也是可以接受的。这些场景可能会选择 `stateless` 升级模式，尽管这意味着不保留完整的状态。

在选择 `stateless` 升级模式时，重要的是要充分理解应用的需求和升级过程中可能面临的限制。对于需要保持状态连续性的重要生产应用，考虑使用其他升级模式，如 `stateful`，这种模式会尝试保留并迁移应用状态。
```

### Seesion 集群 & Job 一起创建    
将Seesion 集群 和 Job配置 一起提交到 Kubernetes。      
```bash
kubectl apply -f basic-session-deployment-and-job.yaml    
```

yaml 内容如下：         
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-session-deployment-example
spec:
  image: flink:1.17
  flinkVersion: v1_17
  jobManager:
    resource:
      memory: "2048m"
      cpu: 1
  taskManager:
    resource:
      memory: "2048m"
      cpu: 1
  serviceAccount: flink
---
apiVersion: flink.apache.org/v1beta1
kind: FlinkSessionJob
metadata:
  name: basic-session-job-example
spec:
  deploymentName: basic-session-deployment-example
  job:
    jarURI: https://repo1.maven.org/maven2/org/apache/flink/flink-examples-streaming_2.12/1.16.1/flink-examples-streaming_2.12-1.16.1-TopSpeedWindowing.jar
    parallelism: 4
    upgradeMode: stateless

---
apiVersion: flink.apache.org/v1beta1
kind: FlinkSessionJob
metadata:
  name: basic-session-job-example2
spec:
  deploymentName: basic-session-deployment-example
  job:
    jarURI: https://repo1.maven.org/maven2/org/apache/flink/flink-examples-streaming_2.12/1.16.1/flink-examples-streaming_2.12-1.16.1.jar
    parallelism: 2
    upgradeMode: stateless
    entryClass: org.apache.flink.streaming.examples.statemachine.StateMachineExample
```


此时，我们已经完成以下内容：            
1.Seesion 集群创建          
2.Job 提交          
3.Seesion 集群 和 Job 一起提交，按照先后顺序创建        

>注意：若需删除 Seesion 集群，请先将 kind 为 FlinkSessionJob 下的 job删除 ！！！          

refer    
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/custom-resource/overview/#application-deployments      

   