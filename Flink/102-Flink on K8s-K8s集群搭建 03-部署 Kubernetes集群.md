# Flink on Kubernetes - Kubernetes集群搭建 - 部署 Kubernetes 集群  

>该篇是基于 上一篇 ’Flink on Kubernetes - Kubernetes集群搭建 - 安装 docker & keepalive & haproxy‘ 完成。     

## 添加阿里源 (所有节点操作)    
```shell
vim /etc/yum.repos.d/kubernetes.repo  

# 添加内容如下 
[kubernetes]
name=Kubernetes
baseurl=http://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64
enabled=1
gpgcheck=0
repo_gpgcheck=0
gpgkey=http://mirrors.aliyun.com/kubernetes/yum/doc/yum-key.gpg
http://mirrors.aliyun.com/kubernetes/yum/doc/rpm-package-key.gpg
```

## 安装和设置开机启动（所有节点操作）
```bash
$ yum install -y kubelet-1.23.16 kubeadm-1.23.16 kubectl-1.23.16
$ systemctl enable kubelet 
$ systemctl start kubelet (该命令会异常)，因为缺少 kubelet的 config.yaml。  
```

>如果kubelet异常，检查/var/lib/kubelet/ 目录下是否有config.yaml文件，没有则添加，并分发到每个节点
vi /var/lib/kubelet/config.yaml      


## 拉取镜像   
```bash 
$ kubeadm config images pull  \
--kubernetes-version v1.23.16 \
--image-repository registry.aliyuncs.com/google_containers
```

## 安装 Master 
创建kubeadmin配置文件，在vip所在的Master节点执行。注意，仓库地址使用registry.aliyuncs.com/google_containers，要不然会因为网络原因拉取不了镜像   

```bash 
$ mkdir /usr/local/kubernetes/manifests -p
$ cd /usr/local/kubernetes/manifests/
$ vim kubeadm-config.yaml
```

kubeadm-config.yaml 内容如下：  
```
apiServer:
  certSANs:
    - k8s-master-01
    - k8s-master-02
    - master.k8s.io
    - 192.168.0.140
    - 192.168.0.141
    - 192.168.0.149
    - 127.0.0.1
  extraArgs:
    authorization-mode: Node,RBAC
  timeoutForControlPlane: 4m0s
apiVersion: kubeadm.k8s.io/v1beta2
certificatesDir: /etc/kubernetes/pki
clusterName: kubernetes
controlPlaneEndpoint: "master.k8s.io:16443"
controllerManager: {}
dns:
  type: CoreDNS
etcd:
  local:
    dataDir: /var/lib/etcd
imageRepository: registry.aliyuncs.com/google_containers
kind: ClusterConfiguration
kubernetesVersion: v1.23.16
networking:
  dnsDomain: cluster.local
  podSubnet: 192.166.0.0/16
  serviceSubnet: 10.96.0.0/16
scheduler: {}
```
 
### 初始化master节点
```bash
$ kubeadm init --config kubeadm-config.yaml 
```     

output log: 
```shell
[addons] Applied essential addon: kube-proxy

Your Kubernetes control-plane has initialized successfully!

To start using your cluster, you need to run the following as a regular user:

  mkdir -p $HOME/.kube
  sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
  sudo chown $(id -u):$(id -g) $HOME/.kube/config

Alternatively, if you are the root user, you can run:

  export KUBECONFIG=/etc/kubernetes/admin.conf

You should now deploy a pod network to the cluster.
Run "kubectl apply -f [podnetwork].yaml" with one of the options listed at:
  https://kubernetes.io/docs/concepts/cluster-administration/addons/

You can now join any number of control-plane nodes by copying certificate authorities
and service account keys on each node and then running the following as root:

  kubeadm join master.k8s.io:16443 --token 1rxqmv.nhfwbcvxfahd2n8o \
	--discovery-token-ca-cert-hash sha256:90d66a1d97daff1ce89045c0f0cbc7c9ba6a5e13322bd76b16795ae063f17a1f \
	--control-plane 

Then you can join any number of worker nodes by running the following on each as root:

kubeadm join master.k8s.io:16443 --token 1rxqmv.nhfwbcvxfahd2n8o \
	--discovery-token-ca-cert-hash sha256:90d66a1d97daff1ce89045c0f0cbc7c9ba6a5e13322bd76b16795ae063f17a1f 
[root@k8s01 manifests]#
```

### 根据提示配置环境变量
```bash 
$ mkdir -p $HOME/.kube
$ sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
$ sudo chown $(id -u):$(id -g) $HOME/.kube/config
```

### 查看集群当前状态
```bash
$ kubectl get cs
$ kubectl get pods -n kube-system 
```

output log:     
```shell
[root@k8s01 kubelet]# kubectl get cs 
Warning: v1 ComponentStatus is deprecated in v1.19+
NAME                 STATUS    MESSAGE                         ERROR
scheduler            Healthy   ok                              
controller-manager   Healthy   ok                              
etcd-0               Healthy   {"health":"true","reason":""}   
[root@k8s01 kubelet]# kubectl get pods -n kube-system 
NAME                            READY   STATUS    RESTARTS   AGE
coredns-6d8c4cb4d-8wqzg         0/1     Pending   0          10m
coredns-6d8c4cb4d-bnsbs         0/1     Pending   0          10m
etcd-k8s01                      1/1     Running   0          11m
kube-apiserver-k8s01            1/1     Running   0          11m
kube-controller-manager-k8s01   1/1     Running   0          11m
kube-proxy-gkprc                1/1     Running   0          10m
kube-scheduler-k8s01            1/1     Running   0          11m
``` 

## 安装calico网络插件  
参考：https://projectcalico.docs.tigera.io/getting-started/kubernetes/quickstart

### 下载yaml文件，部署网络插件，pod-cidr无需修改，calico 自动识别   
```shell
wget https://docs.projectcalico.org/archive/v3.22/manifests/calico.yaml
kubectl apply -f calico.yaml
```

**calico 的镜像下载慢**
calico/kube-controllers                                           v3.22.5    d47f5eb9c9f8   17 months ago   124MB
calico/cni                                                        v3.22.5    893e6f0b6b18   17 months ago   227MB
calico/pod2daemon-flexvol                                         v3.22.5    97795c4240cb   17 months ago   18.8MB
calico/node                                                       v3.22.5    30308f97f742   17 months ago   204MB

### 查看节点变为Ready状态  
```shell
kubectl get nodes
```

## 加入 Master
拷贝文件到Master2，不能拷贝到worker。注意，不要多拷贝，也不要少拷贝
```bash 
$ scp /etc/kubernetes/admin.conf root@k8s02:/etc/kubernetes
$ scp /etc/kubernetes/pki/{ca.*,sa.*,front-proxy-ca.*} root@k8s02:/etc/kubernetes/pki
$ scp /etc/kubernetes/pki/etcd/ca.* root@k8s02:/etc/kubernetes/pki/etcd
```

加入master      
```bash
$ kubeadm join master.k8s.io:16443 --token 1rxqmv.nhfwbcvxfahd2n8o \
 --discovery-token-ca-cert-hash sha256:90d66a1d97daff1ce89045c0f0cbc7c9ba6a5e13322bd76b16795ae063f17a1f \
> --control-plane
```

output log: 
```shell
[mark-control-plane] Marking the node k8s02 as control-plane by adding the labels: [node-role.kubernetes.io/master(deprecated) node-role.kubernetes.io/control-plane node.kubernetes.io/exclude-from-external-load-balancers]
[mark-control-plane] Marking the node k8s02 as control-plane by adding the taints [node-role.kubernetes.io/master:NoSchedule]

This node has joined the cluster and a new control plane instance was created:

* Certificate signing request was sent to apiserver and approval was received.
* The Kubelet was informed of the new secure connection details.
* Control plane (master) label and taint were applied to the new node.
* The Kubernetes control plane instances scaled up.
* A new etcd member was added to the local/stacked etcd cluster.

To start administering your cluster from this node, you need to run the following as a regular user:

	mkdir -p $HOME/.kube
	sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
	sudo chown $(id -u):$(id -g) $HOME/.kube/config

Run 'kubectl get nodes' to see this node join the cluster.
```


## 加入 Worker  
```shell
kubeadm join master.k8s.io:16443 --token 1rxqmv.nhfwbcvxfahd2n8o \
	--discovery-token-ca-cert-hash sha256:90d66a1d97daff1ce89045c0f0cbc7c9ba6a5e13322bd76b16795ae063f17a1f  
```

output log:     
```
[root@k8s03 ~]# kubeadm join master.k8s.io:16443 --token 1rxqmv.nhfwbcvxfahd2n8o \
> --discovery-token-ca-cert-hash sha256:90d66a1d97daff1ce89045c0f0cbc7c9ba6a5e13322bd76b16795ae063f17a1f 
[preflight] Running pre-flight checks
[preflight] Reading configuration from the cluster...
[preflight] FYI: You can look at this config file with 'kubectl -n kube-system get cm kubeadm-config -o yaml'
[kubelet-start] Writing kubelet configuration to file "/var/lib/kubelet/config.yaml"
[kubelet-start] Writing kubelet environment file with flags to file "/var/lib/kubelet/kubeadm-flags.env"
[kubelet-start] Starting the kubelet
[kubelet-start] Waiting for the kubelet to perform the TLS Bootstrap...

This node has joined the cluster:
* Certificate signing request was sent to apiserver and a response was received.
* The Kubelet was informed of the new secure connection details.

Run 'kubectl get nodes' on the control-plane to see this node join the cluster.  
```

## 检查
```shell
kubectl get node
``` 
output log:     
```bash
[root@k8s01 ~]# kubectl get node 
NAME    STATUS   ROLES                  AGE     VERSION
k8s01   Ready    control-plane,master   119m    v1.23.16
k8s02   Ready    control-plane,master   65m     v1.23.16
k8s03   Ready    <none>                 4m4s    v1.23.16
k8s04   Ready    <none>                 57m     v1.23.16
k8s05   Ready    <none>                 3m54s   v1.23.16
k8s06   Ready    <none>                 3m48s   v1.23.16
```     

```shell
kubectl get pods -A
``` 
output log:     
```bash
[root@k8s01 ~]# kubectl get pods -A
NAMESPACE     NAME                                       READY   STATUS    RESTARTS      AGE
kube-system   calico-kube-controllers-5bb5d4f7f4-pkk2q   1/1     Running   0             67m
kube-system   calico-node-7xj79                          1/1     Running   0             3m45s
kube-system   calico-node-8vzbq                          1/1     Running   0             67m
kube-system   calico-node-b2h9k                          1/1     Running   0             4m1s
kube-system   calico-node-chc52                          1/1     Running   0             57m
kube-system   calico-node-rkwqk                          1/1     Running   0             3m51s
kube-system   calico-node-svmpb                          1/1     Running   0             8m9s
kube-system   coredns-6d8c4cb4d-8wqzg                    1/1     Running   0             118m
kube-system   coredns-6d8c4cb4d-bnsbs                    1/1     Running   0             118m
kube-system   etcd-k8s01                                 1/1     Running   0             118m
kube-system   etcd-k8s02                                 1/1     Running   0             65m
kube-system   kube-apiserver-k8s01                       1/1     Running   0             118m
kube-system   kube-apiserver-k8s02                       1/1     Running   0             65m
kube-system   kube-controller-manager-k8s01              1/1     Running   1 (65m ago)   118m
kube-system   kube-controller-manager-k8s02              1/1     Running   0             65m
kube-system   kube-proxy-492nf                           1/1     Running   0             3m45s
kube-system   kube-proxy-bn547                           1/1     Running   0             3m51s
kube-system   kube-proxy-fm4hn                           1/1     Running   0             4m1s
kube-system   kube-proxy-gkprc                           1/1     Running   0             118m
kube-system   kube-proxy-grpj2                           1/1     Running   0             57m
kube-system   kube-proxy-l6w5x                           1/1     Running   0             65m
kube-system   kube-scheduler-k8s01                       1/1     Running   1 (65m ago)   118m
kube-system   kube-scheduler-k8s02                       1/1     Running   0             65m
[root@k8s01 ~]#
```


