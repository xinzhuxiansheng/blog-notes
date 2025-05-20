# Flink 源码 - Native Kubernetes - 探索如何远程调试 Kubernetes 中 JobManager 和 TaskManager

>Flink version: 1.15.4, JDK version: 11, Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8         

## 引言  
对 Java 服务来说，`远程调试`是我们学习源码调用逻辑、获取实际变量值 ... 等信息中非常高效的手段。我们会在 JVM Server(JDK11) 添加以下参数开启远程调试功能。          
```bash
-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=*:5005
```

其中 `suspend` 参数控制 JVM 启动时的行为，非常重要。   

* suspend=y, JVM 启动时暂停，等待调试器连接后才继续执行	需要调试启动初始化过程。     
* suspend=n，JVM 立即启动，不等待调试器连接	调试已经运行中的程序。       

>这需要你清楚 `调试断点` 位置在什么阶段触发调用。             
 
而在 Flink 框架技术领域中，Job 会有两个角色的 JVM 进程服务，分别是 JobManager，TaskManager，接下来，我们探索如何远程调试 Kubernetes 中 JobManager 和 TaskManager？     

## "复杂化"了       
Kubernetes 让远程调试`复杂化`了，为了得到`复杂点`,我们先从 Flink Standalone 模式来看，它与一般的 Java 程序或者 Spring Boot 程序在开启远程调试功能上没有区别，Standalone 下的 JobManager，TaskManager，它们都是 JVM 常驻进程，它们拥有的 `IP`信息是机器的IP，并不会像 Kubernetes 那样，IP会随着 Pod 的生成、销毁而改变。 即使 Standalone 有多个 TaskManager 情况下，IP+PORT 也是可以保证唯一性的，若 IDEA 中的 Remote Debugger 调试哪个 TaskManager 就配置哪个 TaskManager 的 IP + 配置的调试端口即可。      
![remotedebugonk8s01](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s01.jpg)    

这里就是我提到的 `复杂点`，后面还有`更复杂的`,在 Kubernetes 部署的 Job，我怎么才能定位 JobManager，TaskManager 呢？接下来，我们再从 `Flink Web UI`来讨论，它是如何定位到 Flink JobManager 的？     
![remotedebugonk8s02](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s02.jpg)   

使用 Flink Cli 部署的 Native Kubernetes Application Model Job，它创建了名为 `${cluster-id} + -rest` 的 service，使用 Kubernetes 的 `Labels Selector` 来绑定 JobManager ,如下图所示：    
![remotedebugonk8s03](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s03.jpg)      

我使用 `kubectl get service flink-application-test-rest -n flink -o yaml > service.yaml` 命令，将 Service YAML 内容导出来。其核心内容在于 YAML 中的 `selector` 选择器设置，它要与 JobManager 的YAML 中的 `labels` 标签要匹配上。（这里的`匹配`是指假设 JobManager 有多个标签， service 通过绑定其中 <= 10 个标签，主要保证绑定的标签筛选不到其他资源就可以， 这就像SQL 中的 Where 条件， where app = 'flink-application-test' AND component = 'jobmanager' AND type = 'flink-native-kubernetes'）      
![remotedebugonk8s04](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s04.jpg)      

`JobManager YAML 中的 labels：`  
![remotedebugonk8s05](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s05.jpg)    

```yaml
[root@master01 flink15]# cat service.yaml
apiVersion: v1
kind: Service
metadata:
  creationTimestamp: "2025-05-19T01:32:54Z"
  labels:
    app: flink-application-test
    type: flink-native-kubernetes
  name: flink-application-test-rest
  namespace: flink
  ownerReferences:
  - apiVersion: apps/v1
    blockOwnerDeletion: true
    controller: true
    kind: Deployment
    name: flink-application-test
    uid: 71418b40-0e05-459c-a734-90425b8075e0
  resourceVersion: "7515861"
  uid: 8c9e6fbc-3687-4e7f-8fb8-24be7249bc19
spec:
  clusterIP: 10.96.204.90
  clusterIPs:
  - 10.96.204.90
  externalTrafficPolicy: Cluster
  internalTrafficPolicy: Cluster
  ipFamilies:
  - IPv4
  ipFamilyPolicy: SingleStack
  ports:
  - name: rest
    nodePort: 30851
    port: 8081
    protocol: TCP
    targetPort: 8081
  selector:
    app: flink-application-test
    component: jobmanager
    type: flink-native-kubernetes
  sessionAffinity: None
  type: NodePort
status:
  loadBalancer: {}
```

## 给 JobManager 的远程调试端口配置 Service 
有了上面的了解，那我们给 JobManager的调试端口也配置一个 Service，将它也设置 `selector`， YAML 内容如下：  
`jobmanager-service-remote-debug.yaml`  
```yaml
[root@master01 remote-debug]# cat jobmanager-service-remote-debug.yaml
apiVersion: v1
kind: Service
metadata:
  name: flink-jobmanager-service-remote-debug
  namespace: flink
  labels:
    app: flink-application-test
    type: flink-native-kubernetes
    component: service
spec:
  type: NodePort
  selector:
    app: flink-application-test
    component: jobmanager
    type: flink-native-kubernetes
  ports:
    - name: remote-debug
      port: 5005
      targetPort: 5005
      protocol: TCP
      nodePort: 30003
```

## 更复杂了
继续探索 Native Kubernetes Application Model Job 中 TaskManager 的远程调试，一开始我希望可以参考 JobManager 方式来配置它，当 TaskManager 有且只有1个时，我们再创建 TaskManager remote debug service ，这的确可以帮我们解决 TaskManager， 下面是 `TaskManager` YAML 的 labels。 

![remotedebugonk8s06](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s06.jpg)     

labels 中缺少 PodName 的标识，当存在多个 TaskManager 时，我们创建的 Service 中的 `Selector` 根据上面标签会筛选到多个 TaskManager POD， 这对于 `远程调试`来说是不行。   

这部分，多次尝试并未取得实质性结果，我的Kubernetes集群是二进制方式搭建的，Workers 节点与 Pod IP 是联通的。 所以我采用 `JeBrains Gateway`IDE 工具 和将 Flink 源码放入 Worker节点目录，使用 SSH 方式进行远程调试 TaskManager 代码。 注意，这种方式下，我使用的 Pod IP 链接的。     
![remotedebugonk8s07](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s07.jpg)  

>这的确让我头大，比较负责，每次作业启动，Pod IP 随之改动，我每次调试都会重新配置 IP。  

## 给 TaskManager 的远程端口配置 Service

### 单 TaskManager   
vim taskmanager-service-remote-debug.yaml 

```shell
aiVersion: v1
kind: Service
metadata:
  name: flink-taskmanager-service-remote-debug
  namespace: flink
  labels:
    app: flink-application-test
    type: flink-native-kubernetes
    component: service
spec:
  type: NodePort
  selector:
    app: flink-application-test
    component: taskmanager
    type: flink-native-kubernetes
  ports:
    - name: remote-debug
      port: 5005
      targetPort: 5005
      protocol: TCP
      nodePort: 30004 
```
### 多个 TaskManager  
使用 JeBrains Gateway IDE 下的 SSH 方式进行远程调试。 指定 Pod IP 地址 + 远程调试端口即可。 

>后续要是有更好的调试方法，会再给大家同步。  