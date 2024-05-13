# Flink on Kubernetes - Kubernetes Operator - Helm 自定义 value.yaml 本地安装 Operator   

>Operator version: 1.8    

## 引言   
在之前 Blog “Flink on Kubernetes - Kubernetes Operator - 安装 Operator" 介绍使用 Helm 安装 Operator，下面是当时安装时使用的命令：     
```shell
helm repo add flink-operator-repo https://downloads.apache.org/flink/flink-kubernetes-operator-1.8.0/
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink --create-namespace       
```

在 Blog “Flink on Kubernetes - Kubernetes Operator - Operator 和 Flink Job 时区配置” 介绍使用 Helm 安装 Operator时，可添加自定义配置参数，下面是当时安装时使用的命令：    
```shell
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink --set operatorPod.env[0].name=TZ,operatorPod.env[0].value=Asia/Shanghai      
```

>flink-operator-repo/flink-kubernetes-operator 参数解释：           
>- 添加 repo 地址      
helm repo add flink-operator-repo https://downloads.apache.org/flink/flink-kubernetes-operator-1.8.0/         
>- 查看 repo 列表       
[root@k8s01 flink-kubernetes-operator]# helm repo list                                                                 
flink-operator-repo	https://downloads.apache.org/flink/flink-kubernetes-operator-1.8.0/
>- 指定 repo         
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator ...        


**从可测试部署Flink Job的角度来看，这可能满足 Operator 的使用，但目前以上部署的Operator 与实际使用还`相差甚远`**。例如：        
* 1.Operator Log 配置 & 持久化           
* 2.Operator Custom Operator Plugins 使用         
* 3.Operator JVM 配置         
* 4.Operator 自定义 Operator Docker Image   
* 5.Operator 版本升级     
等等 ...   

以上这些开发或者操作，都会涉及到 Operator 卸载和安装，后续的 Blog 中多多少少会涉及到 Operator 卸载和安装，为了符合 `docker manifest 简化思想`，`该篇介绍 使用自定义 value.yaml 安装 Operator`   

## 自定义 value.yaml 安装 Operator        

### 了解 自定义 values.yaml   
访问官网文档`https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/operations/helm/#overriding-configuration-parameters-during-helm-install`, 可了解到它支持传入私有自定义 values file    

### 下载 Operator helm 离线包         
访问 Flink downloads(https://flink.apache.org/zh/downloads/), 下载 Apache Flink Kubernetes Operator 1.8.0 Helm Chart, 参考内容如下：           
```bash
 Apache Flink Kubernetes Operator #
    Apache Flink Kubernetes Operator 1.8.0 - 2024-03-21 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.7.0 - 2023-11-22 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.6.1 - 2023-10-27 (Source, Helm Chart)
    Apache Flink Kubernetes Operator 1.6.0 - 2023-08-15 (Source, Helm Chart)
    ...     
```  

### 解压 & 根据 `--set` 参数配置 values.yaml    
>注意：后面 myvalues.yaml 是 copy values.yaml 得到（保持原始 conf 备份是个好习惯）     
```shell
# 解压   
tar -zxf flink-kubernetes-operator-1.8.0-helm.tgz   

cp flink-kubernetes-operator/values.yaml  flink-kubernetes-operator/myvalues.yaml     
```

vim flink-kubernetes-operator/values.yaml 修改内容如下：              
```yaml
# 1.添加 时区配置   
operatorPod:
  priorityClassName: null
  annotations: {}
  labels: {}
  env:
  - name: TZ     # 新增
    value: Asia/Shanghai  # 新增     
```

在后续的 Blog 会涉及到其他修改，目前只增加时区配置 ...      
            
### 卸载 flink-kubernetes-operator（若已部署，先卸载）        
在之前 Blog “Flink on Kubernetes - Kubernetes Operator - Operator 和 Flink Job 时区配置” 介绍过 `卸载 flink-kubernetes-operator 对处于运行中的 Flink Job 没有影响`        
```shell
helm uninstall flink-kubernetes-operator -n flink        
```

### 部署 flink kubernetes operator(helm 本地安装)   
>注意：helm 本地安装，在shell 命令中 包含“.”, 与 docker build 中的 “.” 一样。 别忽略了 
```shell
helm install -f myvalues.yaml flink-kubernetes-operator . --namespace flink                 
``` 
Output log:     
```bash
[root@k8s01 flink-kubernetes-operator]# helm install -f myvalues.yaml flink-kubernetes-operator . --namespace flink 
NAME: flink-kubernetes-operator
LAST DEPLOYED: Mon May 12 17:05:43 2024
NAMESPACE: flink
STATUS: deployed
REVISION: 1
TEST SUITE: None
```

helm 本地安装，命令中没有指定 `flink-operator-repo/flink-kubernetes-operator`, 而使用 "." 指定当前目录。      

## 总结   
可通过 `kubectl logs -n flink --tail=100 --follow [POD Name]` 验证时区是否生效。      

本地部署是 自定义 Operator 不可或缺的一小步 :) 。       

refer:    
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/operations/helm/#overriding-configuration-parameters-during-helm-install        
2.https://flink.apache.org/zh/downloads/             