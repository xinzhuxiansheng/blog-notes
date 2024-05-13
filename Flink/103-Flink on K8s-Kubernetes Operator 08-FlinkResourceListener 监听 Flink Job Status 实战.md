# Flink on Kubernetes - Kubernetes Operator - FlinkResourceListener 监听 Flink Kubernetes Operator Job Status 实战                     

>Operator version: 1.8, Akka version: 2.7.0  

## 引言         

> #### Custom Flink Resource Listeners          
```bash
The Flink Kubernetes Operator allows users to listen to events and status updates triggered for the Flink Resources managed by the operator. This feature enables tighter integration with the user’s own data platform.

By implementing the FlinkResourceListener interface users can listen to both events and status updates per resource type (FlinkDeployment / FlinkSessionJob). These methods will be called after the respective events have been triggered by the system. Using the context provided on each listener method users can also get access to the related Flink resource and the KubernetesClient itself in order to trigger any further events etc on demand.

Similar to custom validator implementations, resource listeners are loaded via the Flink Plugins mechanism.

In order to enable your custom FlinkResourceListener you need to:

    Implement the interface
    Add your listener class to org.apache.flink.kubernetes.operator.api.listener.FlinkResourceListener in META-INF/services
    Package your JAR and add it to the plugins directory of your operator image (/opt/flink/plugins)
```

## 打包 Flink Kubernetes Operator 镜像             

### 1.查看 flink Kubernetes operator YAML 的 Docker Image       
```yaml   
kubectl -n flink get deployment flink-kubernetes-operator -o yaml   
```

Output log:     
```
ghcr.io/apache/flink-kubernetes-operator:91d67d9        
```

### 2.自定义 Flink Kubernetes Operator Dockerfile               
```bash
FROM ghcr.io/apache/flink-kubernetes-operator:91d67d9
COPY target/javamain-flinkoperatorplugin-1.0-SNAPSHOT.jar /opt/flink/plugins/javamain-flinkoperatorplugin-1.0-SNAPSHOT.jar      
```

打包镜像 & 上传 harbor 
```shell
docker build -t harbor01.io/yzhou/flink-kubernetes-operator:91d67d9-01 .
docker push harbor01.io/yzhou/flink-kubernetes-operator:91d67d9-01       
```

## 更新 Flink Kubernetes Operator              

### 1.卸载 flink-kubernetes-operator        
在之前 Blog “Flink on Kubernetes - Kubernetes Operator - Operator 和 Flink Job 时区配置” 介绍过 `卸载 flink-kubernetes-operator 对处于运行中的 Flink Job 没有影响`  

```shell
helm uninstall flink-kubernetes-operator -n flink       
```

### 2.添加 指定镜像参数  
通过 `--set` 来指定安装参数,但`参数较多时，这点不符合 docker manifest 简化思想`，并且shell 参数的指定也无法持久化，当然也可编写脚本 sh，来保存当时运行的命令明细，但 helm/flink-kubernetes-operator 也提供了自定义 values file, 所以为了简化启动命令，后续使用 `自定义 values file`。       
```shell 
# 不推荐 
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink --set operatorPod.env[0].name=TZ,operatorPod.env[0].value=Asia/Shanghai,image.repository=harbor01.io/yzhou/flink-kubernetes-operator,image.tag=91d67d9-01  
``` 

>注意：访问官网文档`https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/operations/helm/#overriding-configuration-parameters-during-helm-install`, 可了解到它支持传入私有自定义 values file         

**自定义 values file:**         
1）访问 Flink downloads(https://flink.apache.org/zh/downloads/), 下载 Apache Flink Kubernetes Operator 1.8.0 Helm Chart, 参考内容如下：           
```bash
 Apache Flink Kubernetes Operator #
    Apache Flink Kubernetes Operator 1.8.0 - 2024-03-21 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.7.0 - 2023-11-22 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.6.1 - 2023-10-27 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.6.0 - 2023-08-15 (Source, Helm Chart)
    ...     
```     

2）解压 & 根据 `--set` 参数配置 myvalues.yaml  
```shell
# 解压   
tar -zxf flink-kubernetes-operator-1.8.0-helm.tgz    
```

vim values.yaml 修改内容如下：            
```yaml
# 1.添加 时区配置   
operatorPod:
  priorityClassName: null
  annotations: {}
  labels: {}
  env:
  - name: TZ     # 新增
    value: Asia/Shanghai  # 新增     


# 2.修改 docker images
# 将 “ghcr.io/apache/flink-kubernetes-operator” 修改为 “harbor01.io/yzhou/flink-kubernetes-operator”        
# 将 “91d67d9” 修改为 “91d67d9-01”      
image:
  repository: harbor01.io/yzhou/flink-kubernetes-operator  # 修改 
  pullPolicy: IfNotPresent
  tag: "91d67d9-01"   # 修改
```

3）部署 flink kubernetes operator(helm 本地安装 )   
```shell
helm install -f myvalues.yaml flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink                 
``` 
Output log:     
```bash
[root@k8s01 flink-kubernetes-operator]# helm install -f myvalues.yaml flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink 
NAME: flink-kubernetes-operator
LAST DEPLOYED: Mon May 13 14:53:12 2024
NAMESPACE: flink
STATUS: deployed
REVISION: 1
TEST SUITE: None
```

>注意：官网参数使用 helm install -f myvalues.yaml flink-kubernetes-operator `helm/flink-kubernetes-operator` , helm/flink-kubernetes-operator 参数需注意调整为 `flink-operator-repo/flink-kubernetes-operator`    
```bash
[root@k8s01 flink-kubernetes-operator]# helm repo list
NAME               	URL                                                                
flink-operator-repo	https://downloads.apache.org/flink/flink-kubernetes-operator-1.8.0/
```

### 验证  
1）自定义 Plugin jar 是否存在 `/opt/flink/plugins`目录下  
`javamain-flinkoperatorplugin-1.0-SNAPSHOT.jar` 存在。            
```bash
flink@flink-kubernetes-operator-5d699495dd-x4b2b:~/plugins$ ls
flink-metrics-datadog	flink-metrics-influxdb	flink-metrics-prometheus  flink-metrics-statsd
flink-metrics-graphite	flink-metrics-jmx	flink-metrics-slf4j	  javamain-flinkoperatorplugin-1.0-SNAPSHOT.jar
```

2）部署 Flink Job，观察 log




























在之前 Blog “Flink on Kubernetes - Kubernetes Operator - 安装 Operator” 介绍了 安装 Operator，下面是安装的命令：    
```shell
helm repo add flink-operator-repo https://downloads.apache.org/flink/flink-kubernetes-operator-1.8.0/
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink 
```

后续，我们会定制 开发 Operator 的镜像以及 Plugins，为了方便后续的部署，我们再介绍下 helm 的本地安装 Operator ...        

### helm 离线部署 Operator          

1.访问 Flink downloads(https://flink.apache.org/zh/downloads/), 下载 Apache Flink Kubernetes Operator 1.8.0 Helm Chart, 参考内容如下：           
```bash
 Apache Flink Kubernetes Operator #
    Apache Flink Kubernetes Operator 1.8.0 - 2024-03-21 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.7.0 - 2023-11-22 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.6.1 - 2023-10-27 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.6.0 - 2023-08-15 (Source, Helm Chart)
    ...     
```

2.















refer            
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/operations/listeners/     
2.https://flink.apache.org/zh/downloads/        
3.https://ghcr.io/apache/flink-kubernetes-operator      



