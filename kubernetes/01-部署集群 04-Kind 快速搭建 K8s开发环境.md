# Kind 快速搭建 Kubernetes的开发环境    

>OS: Mac 

## 安装 Kind    
请访问官网 https://kind.sigs.k8s.io/docs/user/quick-start/#installing-from-release-binaries 下载 Kind安装包。       

安装什么架构的 Kubernetes，可访问官网 https://kind.sigs.k8s.io/docs/user/configuration/ 了解。      

## 部署 一个主节点,两个工作节点的集群           

### 编写 cluster.yaml，定义集群架构 
vim cluster.yaml, 内容如下：        
```yaml 
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
nodes:
- role: control-plane
  kubeadmConfigPatches:
  - |
    kind: InitConfiguration
    nodeRegistration:
      kubeletExtraArgs:
        node-labels: "ingress-ready=true"
  extraPortMappings:
  - containerPort: 80
    hostPort: 80
    protocol: TCP
  - containerPort: 443
    hostPort: 443
    protocol: TCP
- role: worker
- role: worker
```

### 创建集群并且指定 Kubernetes 版本 
执行以下命令：  
```bash
kind create cluster --image=kindest/node:v1.27.0 \
--name=yzhou-k8s-dev \
--config=cluster.yaml
```

output log:     
```bash
➜  kind kind create cluster --image=kindest/node:v1.27.0 \
--name=yzhou-k8s-dev \
--config=cluster.yaml
Creating cluster "yzhou-k8s-dev" ...
 ✓ Ensuring node image (kindest/node:v1.27.0) 🖼
 ✓ Preparing nodes 📦 📦 📦
 ✓ Writing configuration 📜
 ✓ Starting control-plane 🕹️
 ✓ Installing CNI 🔌
 ✓ Installing StorageClass 💾
 ✓ Joining worker nodes 🚜
Set kubectl context to "kind-yzhou-k8s-dev"
You can now use your cluster with:

kubectl cluster-info --context kind-yzhou-k8s-dev

Have a nice day! 👋
```


>注意：本篇Blog 仅介绍了如何通过 Kind快速搭建1个k8s集群，若需部署其他组件，还需阅读官网了解更多信息，例如 安装 ingress nginx（需注意 ingress 与 Kubernetes版本匹配）                


kubectl apply -f https://raw.githubusercontent.com/kubernetes/ingress-nginx/controller-v1.10.0/deploy/static/provider/kind/deploy.yaml   

若出现 raw.githubusercontent.com 无法访问，可以通过访问 `https://www.ipaddress.com/ip-lookup`，查看该域名解析的 IP地址，再通过修改 Hosts来隐射IP 地址。         

>安装完之后，记得通过官网的案例（https://kind.sigs.k8s.io/docs/user/ingress/#ingress-nginx） 来测试在 Ingress 安装是否成功。        

### 安装 kubectl 
请访问 kubectl 官网 https://kubernetes.io/zh-cn/docs/tasks/tools/， 安装 kubectl。 例如 Mac 系统，访问 “在 macOS 上安装 kubectl” 文档 https://kubernetes.io/zh-cn/docs/tasks/tools/install-kubectl-macos/

```bash
# 针对系统 CPU，下载对应 kubectl 
curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/darwin/arm64/kubectl"

chmod +x ./kubectl  

sudo mv ./kubectl /usr/local/bin/kubectl
# sudo chown root: /usr/local/bin/kubectl
``` 

或者使用 brew 安装  
```bash
brew install kubectl
```

### 查看安装结构        
```bash 
docker ps -a
CONTAINER ID   IMAGE                                            COMMAND                   CREATED             STATUS                     PORTS                       NAMES
f4bb1b7990e4   kindest/node:v1.27.0                             "/usr/local/bin/entr…"   About an hour ago   Up About an hour                                       yzhou-k8s-dev-worker2
151335202d41   kindest/node:v1.27.0                             "/usr/local/bin/entr…"   About an hour ago   Up About an hour                                       yzhou-k8s-dev-worker
b981f4c5fdba   kindest/node:v1.27.0                             "/usr/local/bin/entr…"   About an hour ago   Up About an hour           127.0.0.1:51052->6443/tcp   yzhou-k8s-dev-control-plane
```

## 使用 K8s Cluster 

### 切换已创建好的集群上下文    
```bash
kubectl cluster-info --context kind-yzhou-k8s-dev
```

output log: 
```bash
➜  kind kubectl cluster-info --context kind-yzhou-k8s-dev
Kubernetes control plane is running at https://127.0.0.1:63797
CoreDNS is running at https://127.0.0.1:63797/api/v1/namespaces/kube-system/services/kube-dns:dns/proxy

To further debug and diagnose cluster problems, use 'kubectl cluster-info dump'.
```


### 查看集群 Node
```bash
kubectl get node   
```

output log:     
```bash
➜  kind kubectl get nodes
NAME                          STATUS   ROLES           AGE   VERSION
yzhou-k8s-dev-control-plane   Ready    control-plane   47s   v1.27.0
yzhou-k8s-dev-worker          Ready    <none>          25s   v1.27.0
yzhou-k8s-dev-worker2         Ready    <none>          24s   v1.27.0
```

## 其他操作 

### 查看集群 
```bash
kind get clusters 
```

### 删除 clusters
```bash
kind delete cluster --name [集群名称] 
```

删除后，使用 `docker ps -a` 检查节点是否删除。  


refer                 
1.https://kind.sigs.k8s.io/                                     
2.https://xie.infoq.cn/article/b686d00d5a6049fefc00eddeb                

