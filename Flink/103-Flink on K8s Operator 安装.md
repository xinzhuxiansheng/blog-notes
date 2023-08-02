## Flink on K8s Operator 安装  

1.查阅官方安装指引
Flink主页： https://flink.apache.org/
Flink Operator主页： https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-stable/  
flink-kubernetes-operator heml包下载网站：https://downloads.apache.org/flink/
flink-kubernetes-operator 源码下载网站：https://github.com/apache/flink-kubernetes-operator/tags    

 
2.安装helm 
根据 heml下载地址 https://github.com/helm/helm/releases 下载 helm， 解压后，将 `helm` 移动到 `/usr/local/bin/` 目录下。 

>所有节点都需要安装 helm    

3.添加常用的 helm 源    
helm repo add flink-operator-repo https://downloads.apache.org/flink/flink-kubernetes-operator-1.5.0/
helm repo add stable http://mirror.azure.cn/kubernetes/charts
helm repo add bitnami https://charts.bitnami.com/bitnami
helm repo add aliyun https://kubernetes.oss-cn-hangzhou.aliyuncs.com/charts 


4.安装cert-manager  
kubectl create -f https://github.com/jetstack/cert-manager/releases/download/v1.8.2/cert-manager.yaml   


refer   
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.6/docs/try-flink-kubernetes-operator/quick-start/ 


