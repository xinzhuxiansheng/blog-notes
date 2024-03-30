# Flink on Kubernetes - Kubernetes Operator - 安装 Operator    

## 引言 
请访问 Flink Kubernetes Operator 官网  `https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/`  

>引用官网介绍：  
```bash 
The Flink Kubernetes Operator extends the Kubernetes API with the ability to manage and operate Flink Deployments. The operator features the following amongst others:

    Deploy and monitor Flink Application and Session deployments
    Upgrade, suspend and delete deployments
    Full logging and metrics integration
    Flexible deployments and native integration with Kubernetes tooling
    Flink Job Autoscaler
```

Flink on Kubernetes 方案中有2种 Job 部署的方式：    
1.Kubernetes Native     
2.Kubernetes Operator   

了解更多的 Operator Features，可访问 https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/concepts/overview/#features        

## 了解 Flink Kubernetes Operator 与 Flink 版本关系 
访问官方文档：https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/concepts/overview/    

在`Features` 章节中，会有介绍当前 Operator 版本所支持的Flink Version，内容如下：   
```bash 
 Core #

    Fully-automated Job Lifecycle Management
        Running, suspending and deleting applications
        Stateful and stateless application upgrades
        Triggering and managing savepoints
        Handling errors, rolling-back broken upgrades
    Multiple Flink version support: v1.15, v1.16, v1.17, v1.18
```

## 安装 Operator 
官方安装文档： https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-main/docs/try-flink-kubernetes-operator/quick-start/   

### 安装 helm   
根据 heml下载地址 https://github.com/helm/helm/releases 下载 helm， 解压后，将 `helm` 移动到 `/usr/local/bin/` 目录下。     

需特别注意：helm 对 Kubernetes 版本的要求，请参考 https://helm.sh/zh/docs/topics/version_skew/ ， `例如Kubernetes版本是 1.23.16 ，则建议 helm 版本是 3.8.x 版本`    

>所有节点都需要安装 helm    

`以下步骤参考官网`   

### 安装cert-manager  
```
wget https://github.com/jetstack/cert-manager/releases/download/v1.8.2/cert-manager.yaml   
kubectl apply -f cert-manager.yaml  
```

### 安装 flink-kubernetes-operator  
可通过 --namespace flink， 指定命名空间， 注意 `--create-namespace ` 若命名空间不存在，则创建   
```bash 
helm repo add flink-operator-repo https://downloads.apache.org/flink/flink-kubernetes-operator-1.8.0/
helm install flink-kubernetes-operator flink-operator-repo/flink-kubernetes-operator --namespace flink --create-namespace 
```

### 注意事项   

#### cert-manager 相关镜像拉取失败  
需特别注意 --platform，指定平台 
```bash 
# 可开发机器上拉取后，打包离线 tar，再安装镜像 
docker pull --platform=linux/amd64 quay.io/jetstack/cert-manager-webhook:v1.8.2

# 打包 
docker save quay.io/jetstack/cert-manager-webhook:v1.8.2 -o cert-manager-webhook-v1.8.2.tar

# 所有 K8s节点 导入镜像
docker load -i cert-manager-webhook-v1.8.2.tar
```

#### 重新安装 flink-kubernetes-operator
```bash 
# 若 需要重新安装 flink-kubernetes-operator, 先卸载后，再安装   
helm uninstall flink-kubernetes-operator 
```


refer   
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/try-flink-kubernetes-operator/quick-start/              

