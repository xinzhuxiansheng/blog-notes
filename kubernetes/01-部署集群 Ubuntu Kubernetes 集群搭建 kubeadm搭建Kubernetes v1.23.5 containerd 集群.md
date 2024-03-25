## kubeadm 搭建 Kubernetes v1.27 集群    

>Ubuntu: 22.04.4 , kubernetes version: v1.23.5, containerd version: 1.6.24 
>本环境基于 VMware 虚拟环境，但本篇 Blog 不涉及到 VMware 搭建虚拟环境的介绍。    


### 1.集群规划    
|   ip  |  域名  | 备注| 安装软件|
|  ----  | ----  |----  |----  |
|  192.168.0.211 | master1 | 主节点 | |
|  192.168.0.212 | worker1 |从节点 1 | |
|  192.168.0.213 | worker2 |从节点 2 ||

### 2.安装基础环境    

#### 2.1 设置主机名（每个节点）   
```shell
hostnamectl set-hostname master1
hostnamectl set-hostname worker1
hostnamectl set-hostname worker2
```

#### 2.2 配置hosts解析 
```shell
cat >> /etc/hosts << EOF
192.168.0.211 master1
192.168.0.212 worker1
192.168.0.213 worker2
EOF
```

#### 2.3 更新系统 
```shell
sudo apt update
sudo apt -y full-upgrade
```

#### 2.4 关闭swap 
```shell
swapoff -a
cp /etc/fstab{,.bak}
sed -e '/swap/ s/^#*/#/' -i /etc/fstab
swapon --show
```

#### 2.5 时间同步
```shell
cp /usr/share/zoneinfo/Asia/Shanghai  /etc/localtime

apt install -y chrony
systemctl enable --now chrony
chronyc sources

#确认时间是否同步
timedatectl
```

#### 2.6 加载ipvs内核模块
参考：https://github.com/kubernetes/kubernetes/tree/master/pkg/proxy/ipvs
另外，针对Linux kernel 4.19以上的内核版本使用nf_conntrack 代替nf_conntrack_ipv4。 

```shell
# 查看内核
uname -srm
```

```shell
cat <<EOF | tee /etc/modules-load.d/ipvs.conf
# Load IPVS at boot
ip_vs
ip_vs_rr
ip_vs_wrr
ip_vs_sh
nf_conntrack
EOF

modprobe -- ip_vs
modprobe -- ip_vs_rr
modprobe -- ip_vs_wrr
modprobe -- ip_vs_sh
modprobe -- nf_conntrack

#确认内核模块加载成功
lsmod | grep -e ip_vs -e nf_conntrack

#安装ipset和ipvsadm
apt install -y ipset ipvsadm 
```

### 3.安装containerd  
备注：以下操作在所有节点执行。
#### 3.1 安装containerd容器运行时的前置条件   
```shell
cat <<EOF | sudo tee /etc/modules-load.d/containerd.conf
overlay
br_netfilter
EOF

sudo modprobe overlay
sudo modprobe br_netfilter

# 设置必需的 sysctl 参数，这些参数在重新启动后仍然存在。
cat <<EOF | sudo tee /etc/sysctl.d/99-kubernetes-cri.conf
net.bridge.bridge-nf-call-iptables  = 1
net.ipv4.ip_forward                 = 1
net.bridge.bridge-nf-call-ip6tables = 1
EOF

# 应用 sysctl 参数而无需重新启动
sudo sysctl --system
```

#### 3.2 安装containerd容器运行时，如果网络较差，建议使用浏览器下载到本地，在上传到服务器。
下载地址：https://github.com/containerd/nerdctl/releases    
```shell
wget https://github.com/containerd/nerdctl/releases/download/v0.18.0/nerdctl-full-0.18.0-linux-amd64.tar.gz
tar Cxzvvf /usr/local nerdctl-full-0.18.0-linux-amd64.tar.gz
```

#### 3.3 创建containerd配置文件 
```shell
sudo mkdir -p /etc/containerd
containerd config default | sudo tee /etc/containerd/config.toml
```

#### 3.4 配置使用 systemd cgroup 驱动程序 
```shell
sed -i "s#SystemdCgroup = false#SystemdCgroup = true#g" /etc/containerd/config.toml
```

#### 3.5 修改基础设施镜像 
```shell
sed -i 's#k8s.gcr.io/pause:3.6#registry.aliyuncs.com/google_containers/pause:3.6#g' /etc/containerd/config.toml
```

#### 3.6 启动containerd服务
```shell
systemctl enable --now containerd

# 查看containerd运行状态
systemctl status containerd
```

### 4.安装 kubeadm  
备注：以下操作在所有节点执行。
#### 4.1 添加kubernetes源，使用阿里云apt源进行替换：  
```shell
apt-get install -y apt-transport-https
curl https://mirrors.aliyun.com/kubernetes/apt/doc/apt-key.gpg | apt-key add -

cat <<EOF >/etc/apt/sources.list.d/kubernetes.list
deb https://mirrors.aliyun.com/kubernetes/apt/ kubernetes-xenial main
EOF
```

#### 4.2 安装kubeadm、kubelet及kubectl
```shell
#查看可安装的版本
sudo apt-get update
apt-cache madison kubectl | more

#执行安装
sudo apt-get install -y kubelet=1.23.5-00 kubeadm=1.23.5-00 kubectl=1.23.5-00

#锁定版本
sudo apt-mark hold kubelet kubeadm kubectl
```

#### 4.3 启动kubelet服务  
```shell
systemctl enable --now kubelet
```

### 5.部署master节点
备注：以下操作仅在master节点执行。    

#### 5.1 查看可安装的kubernetes版本     
```shell
kubectl version --short
```

#### 5.2 查看对应版本的容器镜像并提前拉取到本地 
```shell
kubeadm config images list \
  --kubernetes-version=v1.23.5 \
  --image-repository registry.aliyuncs.com/google_containers
```

在所有节点执行以下命令，提前拉取镜像 
```shell
kubeadm config images pull \
  --kubernetes-version=v1.23.5 \
  --image-repository registry.aliyuncs.com/google_containers
```

配置 Crictl 
```shell
cat >/etc/crictl.yaml<<EOF
runtime-endpoint: unix:///run/containerd/containerd.sock
EOF
```

查看拉取的镜像  
```shell
crictl images
``` 

#### 5.3 开始初始化master节点 
仅在master节点运行以下命令开始初始化master节点：    
```shell
kubeadm init --kubernetes-version=v1.23.5 \
    --apiserver-advertise-address=192.168.72.30 \
    --image-repository registry.aliyuncs.com/google_containers \
    --pod-network-cidr=172.16.0.0/16
```

参数说明：
* kubernetes-version=v1.23.5：关闭版本探测，默认值是stable-1，会导致从https://dl.k8s.io/release/stable-1.txt下载最新的版本号，可以将其指定为固定版本来跳过网络请求。    

* apiserver-advertise-address：kubernetes会使用默认网关所在的网络接口广播其主节点的 IP 地址，若需使用其他网络接口需要配置该参数   

* image-repository：Kubenetes默认registry地址是k8s.gcr.io，国内无法访问，该参数将其指定为可访问的镜像地址，这里使用registry.aliyuncs.com/google_containers    

* pod-network-cidr：与所选网络插件cidr一致，并且不能与集群节点网段重叠    


#### 5.4 master节点初始化完成后参考最后提示配置kubectl客户端连接  
```shell
mkdir -p $HOME/.kube
cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
chown $(id -u):$(id -g) $HOME/.kube/config
```

#### 5.5 查看节点状态，当前还未安装网络插件节点处于NotReady状态 
```shell
root@master1:/home/yzhou# kubectl get pods -A
NAMESPACE     NAME                              READY   STATUS    RESTARTS   AGE
kube-system   coredns-6d8c4cb4d-b9czx           0/1     Pending   0          2m28s
kube-system   coredns-6d8c4cb4d-dfpjn           0/1     Pending   0          2m28s
kube-system   etcd-master1                      1/1     Running   0          2m36s
kube-system   kube-apiserver-master1            1/1     Running   0          2m36s
kube-system   kube-controller-manager-master1   1/1     Running   0          2m35s
kube-system   kube-proxy-zdhjq                  1/1     Running   0          2m29s
kube-system   kube-scheduler-master1            1/1     Running   0          2m35s
```

### 6.安装calico网络插件  
参考：https://projectcalico.docs.tigera.io/getting-started/kubernetes/quickstart

#### 6.1 下载yaml文件，部署网络插件，pod-cidr无需修改，calico自动识别   
```shell
wget https://docs.projectcalico.org/archive/v3.22/manifests/calico.yaml
kubectl apply -f calico.yaml
```

#### 6.2 查看节点变为Ready状态  
```shell
kubectl get nodes
```

#### 6.3 查看coredns pod状态变为Running 
```shell
root@master1:/home/yzhou# kubectl get pods -n kube-system 
NAMESPACE     NAME                                       READY   STATUS              RESTARTS   AGE
kube-system   calico-kube-controllers-5bb5d4f7f4-vzzpg   0/1     ContainerCreating   0          2m39s
kube-system   calico-node-tznp9                          1/1     Running             0          2m39s
kube-system   coredns-6d8c4cb4d-b9czx                    1/1     Running             0          6m41s
kube-system   coredns-6d8c4cb4d-dfpjn                    1/1     Running             0          6m41s
kube-system   etcd-master1                               1/1     Running             0          6m49s
kube-system   kube-apiserver-master1                     1/1     Running             0          6m49s
kube-system   kube-controller-manager-master1            1/1     Running             0          6m48s
kube-system   kube-proxy-zdhjq                           1/1     Running             0          6m42s
kube-system   kube-scheduler-master1                     1/1     Running             0          6m48s
```

### 7.worker节点加入集群  
如果master初始化后未记录节点加入集群命令，可以通过运行以下命令重新生成    
```shell
kubeadm token create --print-join-command --ttl 0
```

在 worker1 和 worker2 上分别执行如下命令，将其注册到 Cluster 中：   
```shell
kubeadm join 192.168.72.30:6443 --token ty560y.urplnyysammwdpy0 --discovery-token-ca-cert-hash sha256:ac393799426898705613fe78debfe034fe7b708bd0043ecf75f013acb09de12a
```

通过 kubectl get nodes 查看节点的状态 
```shell
kubectl get nodes -o wide
```

最终运行的pod   
```shell
root@master1:/home/yzhou# kubectl get pods -A -o wide
NAMESPACE     NAME                                       READY   STATUS    RESTARTS   AGE     IP              NODE      NOMINATED NODE   READINESS GATES
kube-system   calico-kube-controllers-5bb5d4f7f4-vzzpg   1/1     Running   0          15m     172.16.137.66   master1   <none>           <none>
kube-system   calico-node-h27nn                          1/1     Running   0          5m33s   192.168.0.212   worker1   <none>           <none>
kube-system   calico-node-nxt2l                          1/1     Running   0          5m9s    192.168.0.213   worker2   <none>           <none>
kube-system   calico-node-tznp9                          1/1     Running   0          15m     192.168.0.211   master1   <none>           <none>
kube-system   coredns-6d8c4cb4d-b9czx                    1/1     Running   0          19m     172.16.137.65   master1   <none>           <none>
kube-system   coredns-6d8c4cb4d-dfpjn                    1/1     Running   0          19m     172.16.137.67   master1   <none>           <none>
kube-system   etcd-master1                               1/1     Running   0          19m     192.168.0.211   master1   <none>           <none>
kube-system   kube-apiserver-master1                     1/1     Running   0          19m     192.168.0.211   master1   <none>           <none>
kube-system   kube-controller-manager-master1            1/1     Running   0          19m     192.168.0.211   master1   <none>           <none>
kube-system   kube-proxy-kt2vg                           1/1     Running   0          2m5s    192.168.0.212   worker1   <none>           <none>
kube-system   kube-proxy-rd4bd                           1/1     Running   0          2m7s    192.168.0.211   master1   <none>           <none>
kube-system   kube-proxy-sl4gp                           1/1     Running   0          2m3s    192.168.0.213   worker2   <none>           <none>
kube-system   kube-scheduler-master1                     1/1     Running   0          19m     192.168.0.211   master1   <none>           <none>
```

### 8.集群其他配置  

#### 8.1 在master节点执行以下操作，开启ipvs模式   
修改kube-proxy configmap，添加mode：ipvs      
```shell
kubectl -n kube-system get cm kube-proxy -o yaml | sed 's/mode: ""/mode: "ipvs"/g' | kubectl replace -f - 
kubectl -n kube-system patch daemonset kube-proxy -p "{\"spec\":{\"template\":{\"metadata\":{\"annotations\":{\"date\":\"`date +'%s'`\"}}}}}"
```

验证工作模式  
```shell
# curl 127.0.0.1:10249/proxyMode
ipvs
```

查看代理规则  
```shell
ipvsadm -ln
```

#### 8.2 master节点调度pod
默认情况下，出于安全原因，群集不会在master节点上调度pod，如果希望能够在master节点上调度pod，例如，对于用于开发的单机Kubernetes集群，请运行以下命令：    
```shell  
#master节点默认打了taints
[root@master ~]# kubectl describe nodes | grep Taints
Taints:             node-role.kubernetes.io/master:NoSchedule

#执行以下命令去掉taints污点
[root@master ~]# kubectl taint nodes master node-role.kubernetes.io/master- 
node/master untainted

#再次查看 taint字段为none
[root@master ~]# kubectl describe nodes | grep Taints
Taints:             <none>

#如果要恢复Master Only状态，执行如下命令：
kubectl taint node k8s-master node-role.kubernetes.io/master=:NoSchedule
```

### 9.部署应用验证集群  
要检查是否可以成功创建 k8s 工作负载，请登录控制平面节点并使用 kubectl 命令创建名为 nginx 的新部署：   
```shell
kubectl create deployment nginx --image=nginx
```

公开 nginx pod 以通过互联网访问。为此目的创建一个新的服务节点端口： 
```shell
kubectl create service nodeport nginx --tcp=80:80
```

使用下面给出的 kubectl 命令检查 nginx pod 和服务的状态: 
```shell
root@master:~# kubectl get pods
NAME                     READY   STATUS    RESTARTS   AGE
nginx-85b98978db-48589   1/1     Running   0          11h

root@master:~# kubectl get svc
NAME         TYPE        CLUSTER-IP      EXTERNAL-IP   PORT(S)        AGE
kubernetes   ClusterIP   10.96.0.1       <none>        443/TCP        12h
nginx        NodePort    10.96.166.220   <none>        80:30379/TCP   11h
```

### 10.清理集群
在所有节点执行以下操作：    
```shell
kubeadm reset -f
systemctl restart containerd
rm -rf /etc/cni/net.d/*
rm -rf /var/lib/calico
ip link delete vxlan.calico
ip link delete kube-ipvs0
```

### 11.其他可选配置项

安装 Kubernetes dashboard 仪表板    
安装 Metrics Server（用于检查 Pod 和节点资源使用情况）    
部署 Prometheus / Grafana 监控      
部署EFK、Grafana Loki日志系统     
部署持久化存储，可选NFS、Rook-ceph、Openebs、Longhorn等     
安装Ingress Controller、官方Ingress-Nginx、traefic、apache apisix等     
安装负载均衡插件MetaLB、OpenELB等       

refer     
1.https://blog.csdn.net/networken/article/details/124071068    