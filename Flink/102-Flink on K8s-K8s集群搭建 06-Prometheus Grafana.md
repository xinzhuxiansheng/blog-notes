# Flink on Kubernetes - Kubernetes集群搭建 - 部署 Prometheus Grafana     

>后续采集 Flink Metrics 

## 引言     
Prometheus、Grafana 在中间件领域是神一样的存在，这里就不过阐述它的理论知识，若你还不了解，那请务必动起来实践它。`Trust me`, 它们对我们后面介绍 Flink Job 监控来说非常重要！！！。下面通过 Chatgpt 给一些它的介绍：                   
```bash 
Prometheus 和 Grafana 是一对强大的工具，经常被一起使用来监控和可视化各种系统和应用的性能指标。这两个工具可以协同工作，提供全面的监控解决方案，下面是关于每个工具的详细介绍：        

### Prometheus

Prometheus 是一个开源的监控和警报工具，它由 SoundCloud 开发。自从 2016 年成为 Cloud Native Computing Foundation 的一部分以来，它迅速成为了云原生生态系统中最流行的监控工具之一。            

**主要特点包括：**

- **多维数据模型**：Prometheus 使用时间序列数据来存储监控信息，每个时间序列由指标名和一组键值对（称为标签）唯一标识。
- **灵活的查询语言**：Prometheus 自带一种功能强大的查询语言 PromQL，允许用户检索和处理数据。
- **无需分布式存储**：Prometheus 服务器是独立的，不依赖于网络存储，所有数据都存储在本地磁盘上。
- **服务发现**：支持多种服务发现机制，可以自动发现目标服务并收集数据，例如 Kubernetes、EC2 等。
- **警报功能**：Prometheus 可以定义警报规则，当某些条件满足时自动触发警报并通过 Alertmanager 进行处理。

### Grafana

Grafana 是一个开源的指标分析和可视化套件。它最初是为了与 Graphite 配合使用而开发的，但现在支持多种数据源，包括 Prometheus、InfluxDB、Elasticsearch 等。

**主要特点包括：**

- **丰富的可视化选项**：提供多种图表类型，如折线图、柱状图、饼图、热图等。
- **灵活的仪表板**：用户可以创建和配置仪表板，展示实时监控数据，支持拖放式的编辑功能。
- **数据源兼容性**：支持多种数据源，使其能与不同的后端系统无缝对接。
- **警报功能**：Grafana 也提供了警报工具，可以基于数据阈值直接在 Grafana 中设置警报。
- **社区和插件**：拥有活跃的社区和大量的插件，可以扩展 Grafana 的功能和外观。

### Prometheus 和 Grafana 的集成

Prometheus 和 Grafana 经常被一起使用来实现监控解决方案：

- **数据收集**：Prometheus 负责从配置好的目标中抓取指标数据。
- **数据存储**：Prometheus 将这些数据以时间序列的形式存储在本地。
- **数据展示**：Grafana 作为前端来展示这些数据，可以从 Prometheus 读取数据并将其以图形的形式展示。
- **警报管理**：可以配置 Prometheus 规则来发送警报到 Grafana，或者利用 Grafana 自身的警报系统。

这种集成提供了一个强大且灵活的监控平台，能够适应从小型到大型、复杂的环境。这对于希望获得深入洞察其系统性能的开发者和系统管理员来说是非常有用的。            
``` 

接下来，开始我们搭建之旅...             

## 部署 Prometheus、Grafana         
 
### 1.选择 kube-prometheus 版本 
以下的 Kubernetes 版本是受支持的，并且我们在相应的分支中对这些版本进行了测试。但请注意，其他版本也可能适用！  因我们 Kubernetes 环境是 1.23.16 版本，所以选择 `release-0.11`  

| kube-prometheus stack                                                                      | Kubernetes 1.22 | Kubernetes 1.23 | Kubernetes 1.24 | Kubernetes 1.25 | Kubernetes 1.26 | Kubernetes 1.27 | Kubernetes 1.28 |
|--------------------------------------------------------------------------------------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|
| [`release-0.10`](https://github.com/prometheus-operator/kube-prometheus/tree/release-0.10) | ✔               | ✔               | ✗               | ✗               | x               | x               | x               |
| [`release-0.11`](https://github.com/prometheus-operator/kube-prometheus/tree/release-0.11) | ✗               | ✔               | ✔               | ✗               | x               | x               | x               |
| [`release-0.12`](https://github.com/prometheus-operator/kube-prometheus/tree/release-0.12) | ✗               | ✗               | ✔               | ✔               | x               | x               | x               |
| [`release-0.13`](https://github.com/prometheus-operator/kube-prometheus/tree/release-0.13) | ✗               | ✗               | ✗               | x               | ✔               | ✔               | ✔               |
| [`main`](https://github.com/prometheus-operator/kube-prometheus/tree/main)                 | ✗               | ✗               | ✗               | x               | x               | ✔               | ✔               |


### 2.下载 kube-promethus       
```shell   
wget https://github.com/prometheus-operator/kube-prometheus/tree/release-0.11   

tar -zxf kube-prometheus-0.11.0.tar.gz          
```

### 3.了解 kube-prometheus-0.11.0/manifests/ 目录   
kube-prometheus-0.11.0/manifests/ 目录下，包含了 prometheus、grafana、alert等等组件, 而  kube-prometheus-0.11.0/manifests/setup 目录负责初始化，涉及到以下内容：        
```bash
Custom Resource Definitions (CRDs)：Prometheus Operator 使用 CRDs 来定义和管理 Prometheus、Alertmanager 实例以及相关的监控和告警规则。setup/ 通常包含这些 CRDs 的定义。     

Prometheus Operator Deployment: 在部署 Prometheus 和 Alertmanager 实例之前，您首先需要部署 Prometheus Operator。setup/ 可能包含了 Prometheus Operator 的 Deployment 清单。      

RBAC Resources: 这可能包括与 Prometheus Operator 和其他监控组件相关的 Roles, RoleBindings, ServiceAccounts 等。         

Initial Configurations: 这可能包括一些初步的配置，如 ConfigMaps，用于初始化组件的设置。         

在应用整个 manifests/ 目录之前，通常首先应用 manifests/setup/。这确保了所有必要的基础设施和设置都到位，然后再部署其他监控组件，例如 Prometheus、Alertmanager、Grafana 等。          
```             

### 4.配置 Prometheus-server 存储持久化  
因为 prometheus 是 Statefulset 部署模式， 这里使用 StorageClass动态创建 PVC 方式（并不需要提前创建 PVC），并且命名规则会参考 POD的命名。 修改内容如下：           
vim kube-prometheus-0.11.0/manifests/prometheus-prometheus.yaml           
```yaml
# 在 yaml 最后一行，添加以下内容：  
  serviceAccountName: prometheus-k8s
  serviceMonitorNamespaceSelector: {}
  serviceMonitorSelector: {}
  version: 2.36.1 # 该行以下内容是 添加内容  
  retention: 2d # 数据保留时长
  storage: 
    volumeClaimTemplate: 
      spec: 
        storageClassName: nfs-storage # 配置 StorageClass
        resources: 
          requests: 
            storage: 5Gi
```

### 5.配置 Grafana 存储持久化   
由于 ​​Grafana​​​ 资源 kind是 ​​Deployment​​​，这里需要补充一个知识点：`在 Kubernetes 的 YAML 中不能直接在 Deployment 的配置文件中声明 volumeClaimTemplate。volumeClaimTemplate 是专门用于 StatefulSets 中的，用来为每个 Pod 创建一个新的 Persistent Volume（PV）`, 所以我们提前为其创建一个 ​​grafana-pvc.yaml ​​​文件，加入下面 ​​PVC​​ 配置。                    
创建 manifests/grafana-pvc.yaml          
```yaml
kind: PersistentVolumeClaim
apiVersion: v1
metadata:
  name: grafana
  namespace: monitoring  # namespace
spec:
  storageClassName: nfs-storage # 配置 pvc的storageClass
  accessModes:
    - ReadWriteOnce # 允许一个持久卷被单个节点以读写方式使用
  resources:
    requests:
      storage: 5Gi
```

修改 manifests/grafana-deployment.yaml，将上面创建`name: grafana`的 volume 配置在 grafana-deployment.yaml 中，内容如下：        
```yaml
# 原文内容
- emptyDir: {}
  ame: grafana-storage

# 将上面原文内容 修改为下面内容，注意 claimName。
- name: grafana-storage
  persistentVolumeClaim:
    claimName: grafana
```

```shell
# 若 monitoring 命名空间不存在，则创建
kubectl create namespace monitoring

kubectl apply -f grafana-pvc.yaml           
```

### 6.将 Prometheus，Grafana 的 Service Type 设置为 NodePort        
vim manifests/prometheus-service.yaml   
```yaml
spec:
  type: NodePort # 新增
  ports:
  - name: web
    port: 9090
    targetPort: web
    nodePort: 32101 # 新增
  - name: reloader-web
    port: 8080
    targetPort: reloader-web
    nodePort: 32102 # 新增
  selector:
``` 

vim manifests/grafana-service.yaml 
```yaml
spec:
  type: NodePort # 新增
  ports:
  - name: http
    port: 3000
    targetPort: http
    nodePort: 32103 # 新增
  selector:
```     

### 7.修改副本数  
若资源充足可忽略该修改，因为是测试，所以将副本数调整为 1， 修改如下：       
1）vim manifests/prometheus-prometheus.yaml  将 `replicas` 设置为 1 （默认是2）         
2）vim manifests/prometheusAdapter-deployment.yaml  将 `replicas` 设置为 1 （默认是2）    
3) vim alertmanager-alertmanager.yaml 将 `replicas` 设置为 1 （默认是3）  

### 8.部署  
```shell
kubectl apply --server-side -f manifests/setup  

kubectl wait \
	--for condition=Established \
	--all CustomResourceDefinition \
	--namespace=monitoring

kubectl apply -f manifests/         
```

### 9.删除 networkpolicy  
因为 Prometheus，Grafana Service Type 为 NodePort，那么 安装包中的 networkpolicy相关需要删除。 操作如下  
```shell
[root@k8s01 ~]# kubectl delete networkpolicy --all -n monitoring  
networkpolicy.networking.k8s.io "alertmanager-main" deleted
networkpolicy.networking.k8s.io "blackbox-exporter" deleted
networkpolicy.networking.k8s.io "grafana" deleted
networkpolicy.networking.k8s.io "kube-state-metrics" deleted
networkpolicy.networking.k8s.io "node-exporter" deleted
networkpolicy.networking.k8s.io "prometheus-adapter" deleted
networkpolicy.networking.k8s.io "prometheus-k8s" deleted
networkpolicy.networking.k8s.io "prometheus-operator" deleted
```

等待镜像 pull ...       

## 最后一步     
下面列举组件 Web：          
* Grafana Web: http://192.168.0.140:32103  默认账户和密码： admin/admin         
* Prometheus Web: http://192.168.0.140:32101                

>针对镜像下载慢问题，大伙还得自行 google ..., 基本上都是通过 拉取代理镜像，再重新打 tag。                

已完成部署 Prometheus Grafana。         

refer       
1.https://github.com/prometheus-operator/kube-prometheus                
