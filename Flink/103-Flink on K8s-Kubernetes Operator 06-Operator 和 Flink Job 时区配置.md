# Flink on Kubernetes - Kubernetes Operator - Operator 和 Flink Job 时区配置  

>Operator version: 1.8  

## 引言    
Kubernetes Pod 时区问题(少于8小时)是一个比较容易忽略的问题，在实际平台开发中，我们会用`日志采集组件`收集 Kubernetes Pod的 log，这样会有以下几点好处： 
* 便于 log 检索, Flink Job 会存在 多个Pod，那一个个查看，有些费时费力。        
* 便于 log 持久化, Flink Job 会存在 异常重启，若log 没有采集且没有外部挂载，则异常时的 log会丢失。        

在 log 检索场景中，支持时间条件检索是必不可少的（时间语义要准确），若还存在与其他平台关联检索，那时间就一定要校准。            


## 查看 POD logs 
下面是Flink 相关 POD 的示例列表, 使用 `kubectl logs -n flink --tail=100 --follow POD名称` 查看 POD logs。       
示例： 
```bash   
NAMESPACE↑  NAME                                                        PF READY STATUS    
flink       basic-application-deployment-only-ingress-58579ff58c-n4jqb  ●  1/1   Running   
flink       basic-application-deployment-only-ingress-taskmanager-1-1   ●  1/1   Running   
flink       busybox                                                     ●  1/1   Running   
flink       flink-kubernetes-operator-59878dff7-l4zp8                   ●  2/2   Running   
```

>Flink Job POD 和 Flink kubernetes operator POD 的默认时区是UTC，与我们的北京时区相差8小时     

## Flink Job POD 时区配置      
使用 `kubectl logs -n flink --tail=100 --follow basic-application-deployment-only-ingress-58579ff58c-n4jqb` 查看 Job POD logs     
```bash
2024-05-01 08:34:19,494 INFO  org.apache.flink.runtime.checkpoint.CheckpointCoordinator    [] - Completed checkpoint 11082 for job 8ed117bddd09c5fa736f7792cc04498f (15387 bytes, checkpointDuration=8 ms, finalizationTime=0 ms).
2024-05-01 08:34:21,487 INFO  org.apache.flink.runtime.checkpoint.CheckpointCoordinator    [] - Triggering checkpoint 11083 (type=CheckpointType{name='Checkpoint', sharingFilesStrategy=FORWARD_BACKWARD}) @ 1715070861487 for job 8ed117bddd09c5fa736f7792cc04498f.
2024-05-01 08:34:21,494 INFO  org.apache.flink.runtime.checkpoint.CheckpointCoordinator    [] - Completed checkpoint 11083 for job  8ed117bddd09c5fa736f7792cc04498f (15288 bytes, checkpointDuration=7 ms, finalizationTime=0 ms).
```   

1.yaml 配置 时区        
在 Flink Job YAML 的 `podTemplate` 配置项 中添加以下内容即可：         
```yaml
  podTemplate:
    spec:
      containers:
        - name: flink-main-container
          env:
            - name: TZ  # 设置容器运行的时区
              value: Asia/Shanghai
```

**给出一个完整示例：**      
basic-application-deployment-only-ingress-tz.yaml       
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-application-deployment-only-ingress-tz
spec:
  image: flink:1.17
  flinkVersion: v1_17
  ingress: 
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
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
  podTemplate:
    spec:
      containers:
        - name: flink-main-container
          env:
            - name: TZ  # 设置容器运行的时区
              value: Asia/Shanghai
  job:
    jarURI: local:///opt/flink/examples/streaming/StateMachineExample.jar
    parallelism: 2
    upgradeMode: stateless
```


2.提交作业      
```shell
kubectl apply -f basic-application-deployment-only-ingress-tz.yaml        
```

3.查看作业 Pod       
```shell
kubectl get all -n flink 
```

4.打开网页，查看日志      
```shell
kubectl logs -n flink --tail=100 --follow basic-application-deployment-only-ingress-tz-bd954c447-s446f    
``` 

Output logs:      
```bash
2024-05-01 17:01:53,671 INFO  org.apache.flink.runtime.checkpoint.CheckpointCoordinator    [] - Completed checkpoint 360 for job cb8ec3e5a641da6fcd2a6db1abc10e8a (15090 bytes, checkpointDuration=15 ms, finalizationTime=0 ms).
2024-05-01 17:01:55,658 INFO  org.apache.flink.runtime.checkpoint.CheckpointCoordinator    [] - Triggering checkpoint 361 (type=CheckpointType{name='Checkpoint', sharingFilesStrategy=FORWARD_BACKWARD}) @ 1715072515656 for job cb8ec3e5a641da6fcd2a6db1abc10e8a.
2024-05-01 17:01:55,672 INFO  org.apache.flink.runtime.checkpoint.CheckpointCoordinator    [] - Completed checkpoint 361 for job cb8ec3e5a641da6fcd2a6db1abc10e8a (15225 bytes, checkpointDuration=15 ms, finalizationTime=1 ms).
```


## Flink Kubernetes Operator POD 时区配置       
在之前的 Blog “Flink on Kubernetes - Kubernetes Operator - 安装 Operator” 介绍了使用 helm方式 安装 flink operator， 若要重新安装，则需要先卸载，那`flink operator 卸载后，会影响正在运行的 Flink Job？`    

>接下来，一起探索...      

### 1.卸载 flink-kubernetes-operator  
```shell
[root@k8s01 k8s_yaml]# helm uninstall flink-kubernetes-operator -n flink 
These resources were kept due to the resource policy:
[RoleBinding] flink-role-binding
[Role] flink
[ServiceAccount] flink

release "flink-kubernetes-operator" uninstalled 
[root@k8s01 k8s_yaml]#   
```

执行完后， 
* 1.Flink Job 运行正常（kubectl -n flink get pod |grep "basic-application-deployment-only" ）         
```bash
[root@k8s01 k8s_yaml]# kubectl -n flink get pod |grep "basic-application-deployment-only"
basic-application-deployment-only-ingress-tz-bd954c447-s446f   1/1     Running   0               35m
basic-application-deployment-only-ingress-tz-taskmanager-1-1   1/1     Running   0               35m
```

* 2.FlinkDeployment 也正常展示 (kubectl get flinkdeployment -n flink)       
```bash
[root@k8s01 k8s_yaml]# kubectl get flinkdeployment -n flink
NAME                                           JOB STATUS   LIFECYCLE STATE
basic-application-deployment-only-ingress-tz   RUNNING      STABLE
``` 

* 3.CRD 仍然存在      
```bash
[root@k8s01 k8s_yaml]# kubectl get crd -n flink |grep flink
flinkdeployments.flink.apache.org                     2024-03-29T13:05:39Z
flinksessionjobs.flink.apache.org                     2024-03-29T13:05:39Z
```

* 4.但 flink-kubernetes-operator POD 已删除。   

>接下来，使用 helm 安装 flink-kubernetes-operator   

### 2.安装 flink-kubernetes-operator 且指定配置参数   
大家可访问`https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/helm/#overriding-configuration-parameters-during-helm-install`，了解官网的介绍，在 helm install 的时候，我们可以通过指定配置参数来安装。例如，现在我们需要对 `flink-kubernetes-operator`的 POD 增加 TZ 环境变量，内容如下：  
```bash
  env:
    - name: TZ
      value: Asia/Shanghai
```
**通过 operatorPod 指定 env 参数**     
```shell
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink --set operatorPod.env[0].name=TZ,operatorPod.env[0].value=Asia/Shanghai 
```

Output logs:    
```bash
[root@k8s01 k8s_yaml]# helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink --set operatorPod.env[0].name=TZ,operatorPod.env[0].value=Asia/Shanghai 
NAME: flink-kubernetes-operator
LAST DEPLOYED: Tue May  7 17:29:22 2024
NAMESPACE: flink
STATUS: deployed
REVISION: 1
TEST SUITE: None
```

此时 查看 flink-kubernetes-operator logs 是正确。     
```bash 
[root@k8s01 k8s_yaml]# kubectl get pod -n flink
NAME                                                           READY   STATUS    RESTARTS        AGE
basic-application-deployment-only-ingress-tz-bd954c447-s446f   1/1     Running   0               42m
basic-application-deployment-only-ingress-tz-taskmanager-1-1   1/1     Running   0               41m
busybox                                                        1/1     Running   649 (24m ago)   38d
flink-kubernetes-operator-7d5d8dcb64-bqnpp                     2/2     Running   0               117s
```

**查看 YAML**   
```shell
kubectl get pod flink-kubernetes-operator-7d5d8dcb64-bqnpp -n flink -o yaml   
```

查看在 `containers` 包含以下配置： 
```bash
    - name: TZ
      value: Asia/Shanghai
```

## 总结   
目前来看，Operator 和 Flink Job 时区都已设置完成。        
得到的结论，卸载 flink-kubernetes-operator，并没有影响正在运行的 Flink Job。            
>别忘记，在 Java 程序部署 Flink Job的方法中，添加 时区配置。      
