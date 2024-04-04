## kubeadm 搭建 Kubernetes v1.27 集群    

>CentOS: 7.9 , kubernetes version: v1.27, containerd version: 1.6.24 
>本环境基于 VMware 虚拟环境，但本篇 Blog 不涉及到 VMware 搭建虚拟环境的介绍。   


### 1.集群规划    
|   ip  |  域名  | 备注| 安装软件|
|  ----  | ----  |----  |----  |
|  192.168.0.131 | master | 主节点 |Docker Kubeadm kubelet kubectl flannel |
|  192.168.0.132 | node1 |从节点 1 |Docker Kubeadm kubelet kubectl |
|  192.168.0.133 | node2 |从节点 2 |Docker Kubeadm kubelet kubectl|

### 2.安装基础环境    

>针对所有节点   

可访问官网了解一些`准备开始`的工作（https://v1-27.docs.kubernetes.io/zh-cn/docs/setup/production-environment/tools/kubeadm/install-kubeadm/）  

**准备开始**   
```
* 一台兼容的 Linux 主机。Kubernetes 项目为基于 Debian 和 Red Hat 的 Linux 发行版以及一些不提供包管理器的发行版提供通用的指令。      
* 每台机器 2 GB 或更多的 RAM（如果少于这个数字将会影响你应用的运行内存）。      
* CPU 2 核心及以上。   
* 集群中的所有机器的网络彼此均能相互连接（公网和内网都可以）。   
* 节点之中不可以有重复的主机名、MAC 地址或 product_uuid。请参见这里了解更多详细信息。  
* 开启机器上的某些端口。请参见这里了解更多详细信息。  
* 禁用交换分区。为了保证 kubelet 正常工作，你必须禁用交换分区。  
  * 例如，sudo swapoff -a 将暂时禁用交换分区。要使此更改在重启后保持不变，请确保在如 /etc/fstab、systemd.swap 等配置文件中禁用交换分区，具体取决于你的系统如何配置
```

>所以在未安装 kubernetes 之前有较多的准备工作，下面将一一列举出来。 :)     

### 2.1 系统初始化   

#### 设置时间同步 & 修改时区   
```shell 
# 检查 chrony 是否安装 
rpm -qa |grep chrony

# 安装 chrony
yum install chrony -y  
```

配置 chrony, 执行 `vim /etc/chrony.conf ` ,添加以下内容：     
``` 
ntpdate ntp1.aliyun.com iburst
```

记得重启 chrony 服务    
```shell
# 重启 chrony 服务
systemctl restart chronyd

# 查看 chrony 服务状态
systemctl status chronyd
```

检查时区，若已经是`Asia/Shanghai` 则无需更改。      
```shell
# 检查时区
timedatectl 

# ouput: 
[root@localhost ~]# timedatectl
      Local time: Tue 2023-11-21 12:06:10 CST
  Universal time: Tue 2023-11-21 04:06:10 UTC
        RTC time: Tue 2023-11-21 04:06:10
       Time zone: Asia/Shanghai (CST, +0800)   // 已是上海时区，则无需更改
     NTP enabled: yes
NTP synchronized: yes
 RTC in local TZ: no
      DST active: n/a
```

设置时区：  
```shell
# 检查亚洲S开头的时区
timedatectl list-timezones |  grep  -E "Asia/S.*"

# output:
[root@localhost ~]# timedatectl list-timezones |  grep  -E "Asia/S.*"
Asia/Sakhalin
Asia/Samarkand
Asia/Seoul
Asia/Shanghai
Asia/Singapore
Asia/Srednekolymsk
```
通过 ` timedatectl set-timezone "Asia/Shanghai" ` 命令将时区设置为 `Asia/Shanghai` 

设置开机自启动：    
```shell
# 启动chrony服务
systemctl start chronyd

# 设置开机自启
systemctl enable chronyd

# 查看chrony服务状态
systemctl status chronyd
```

若需要手动同步系统时钟：    
```shell
# 手动同步系统时钟
chronyc -a makestep

# 查看时间同步源
chronyc sources -v
```

#### 关闭 firewalld 防火墙    
```shell  
# 停止 firewall
systemctl stop firewalld.service

# 禁止firewaall开机启动
systemctl disable firewalld.service

# 查看防火墙状态
firewall-cmd  --state
```

#### 关闭swap     
选择永久关闭即可
```shell
# 临时
swapoff -a  

# 永久关闭
sed -ri 's/.*swap.*/#&/' /etc/fstab 
```

#### 关闭 selinux        
选择永久关闭即可
```shell
# 临时关闭
setenforce 0  

# 永久关闭
sed -i 's/enforcing/disabled/' /etc/selinux/config  
```

#### 优化内核参数
```shell
# 将以下内容追加到  /etc/sysctl.d/sysctl.conf
cat > /etc/sysctl.d/sysctl.conf <<EOF
net.bridge.bridge-nf-call-iptables=1
net.bridge.bridge-nf-call-ip6tables=1
net.ipv4.ip_forward=1
net.ipv4.tcp_tw_recycle=0
vm.swappiness=0
vm.overcommit_memory=1
vm.panic_on_oom=0
fs.inotify.max_user_instances=8192
fs.inotify.max_user_watches=1048576
fs.file-max=52706963
fs.nr_open=52706963
net.ipv6.conf.all.disable_ipv6=1
net.netfilter.nf_conntrack_max=2310720
EOF

# 加载模块 (一定要执行) 
modprobe br_netfilter
lsmod |grep conntrack
modprobe ip_conntrack

# 生效 
sysctl -p /etc/sysctl.d/sysctl.conf
```
4.12之前版本内核需要关闭tcp_tw_recycle，否则与NAT冲突，可能导致服务不通。 需添加 `net.ipv4.tcp_tw_recycle=0`    

此处可参考：    
* "参考 tcp: remove tcp_tw_recycle " https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=4396e46187ca5070219b81773c4e65088dac50cc      
* "参考 Linux 从4.12内核版本开始移除了 tcp_tw_recycle 配置" https://www.cnblogs.com/papering/p/13402080.html      

#### 安装 ipvs 转发支持 
>请注意, 启动 ipvs而不使用 iptables的原因：   
ipvs 可以更快地重定向流量，并且在同步代理规则时具有更好的性能。此外，ipvs 为负载均衡算法提供了更多选项，例如：          
rr ：轮询调度       
lc ：最小连接数       
dh ：目标哈希       
sh ：源哈希       
sed ：最短期望延迟      
nq ： 不排队调度      

**1.安装 ipvs**
```shell
yum -y install ipvsadm ipset
# yum install -y conntrack ipvsadm ipset jq iptables curl sysstat libseccomp 
```

**2.检查是否加载**  
```shell
lsmod | grep ip_vs  
```

**3.若没有加载成功则手动执行**   
```shell
# 添加加载模块
cat > /etc/sysconfig/modules/ipvs.modules << EOF
#!/bin/bash
modprobe -- ip_vs
modprobe -- ip_vs_rr
modprobe -- ip_vs_wrr
modprobe -- ip_vs_sh
modprobe -- nf_conntrack_ipv4
EOF
 
# 生效
chmod 755 /etc/sysconfig/modules/ipvs.modules
source /etc/sysconfig/modules/ipvs.modules   
```

最后再通过 `lsmod | grep ip_vs` ,完整输出：     
```
[root@localhost ~]# lsmod | grep ip_vs
ip_vs_sh               12688  0 
ip_vs_wrr              12697  0 
ip_vs_rr               12600  0 
ip_vs                 145458  6 ip_vs_rr,ip_vs_sh,ip_vs_wrr
nf_conntrack          139264  2 ip_vs,nf_conntrack_ipv4
libcrc32c              12644  3 xfs,ip_vs,nf_conntrack
```

### 3.安装 Containerd       

#### 3.1 创建 /etc/modules-load.d/containerd.conf 配置文件
```shell
cat << EOF > /etc/modules-load.d/containerd.conf
overlay
br_netfilter
EOF

# 加载
modprobe overlay
modprobe br_netfilter
```

#### 3.2 安装
```shell
wget -O /etc/yum.repos.d/docker-ce.repo https://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo

yum install -y containerd.io
```

#### 3.3 配置 
```shell
# 生成containerd的配置文件
mkdir /etc/containerd -p 

# 生成配置文件
containerd config default > /etc/containerd/config.toml
```

编辑配置文件 `vim /etc/containerd/config.toml `    
```shell
# SystemdCgroup
SystemdCgroup = false 改为 SystemdCgroup = true

# sandbox_image
sandbox_image = "k8s.gcr.io/pause:3.6"  改为： sandbox_image = "registry.aliyuncs.com/google_containers/pause:3.6"
```

#### 3.4 启动 
```shell
# 开机自启动
systemctl enable containerd

Created symlink from /etc/systemd/system/multi-user.target.wants/containerd.service to /usr/lib/systemd/system/containerd.service.

# 启动
systemctl start containerd

# 查看镜像    
ctr images ls 
```

### 4.安装 K8s v1.27.x  

#### 4.1 配置k8s v1.27.x 的 yum 源
```shell
#添加阿里云YUM软件源
cat <<EOF > /etc/yum.repos.d/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=https://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64/
enabled=1
gpgcheck=0
repo_gpgcheck=0
gpgkey=https://mirrors.aliyun.com/kubernetes/yum/doc/yum-key.gpg https://mirrors.aliyun.com/kubernetes/yum/doc/rpm-package-key.gpg
EOF


yum makecache
```

#### 4.2 安装
```shell
# 查看所有可用版本   
yum list kubelet --showduplicates | sort -r |grep 1.27  

# 安装kubeadm，kubelet和kubectl
yum install -y kubectl-1.27.6 kubelet-1.27.6 kubeadm-1.27.6  
```

#### 4.3 配置  
为了实现docker使用的cgroupdriver与kubelet使用的cgroup的一致性，建议修改如下文件内容。
```shell
vim /etc/sysconfig/kubelet
KUBELET_EXTRA_ARGS="--cgroup-driver=systemd"

#设置kubelet为开机自启动即可，由于没有生成配置文件，集群初始化后自动启动
systemctl enable kubelet  
```

#### 4.4 准备镜像 & 初始化
```shell
kubeadm config images list --kubernetes-version=v1.27.6

# output: 
[root@localhost ~]# kubeadm config images list --kubernetes-version=v1.27.6
registry.k8s.io/kube-apiserver:v1.27.6
registry.k8s.io/kube-controller-manager:v1.27.6
registry.k8s.io/kube-scheduler:v1.27.6
registry.k8s.io/kube-proxy:v1.27.6
registry.k8s.io/pause:3.9
registry.k8s.io/etcd:3.5.7-0
registry.k8s.io/coredns/coredns:v1.10.1
```

使用kubeadm init命令初始化         
```shell
# 初始化
kubeadm init --apiserver-advertise-address=0.0.0.0 --apiserver-bind-port=6443 --kubernetes-version=v1.27.6 --pod-network-cidr=172.30.0.0/16 --service-cidr=10.254.0.0/16 --image-repository registry.aliyuncs.com/google_containers

# 参数介绍
--apiserver-advertise-address 集群通告地址
--apiserver-bind-port 
--image-repository 由于默认拉取镜像地址k8s.gcr.io国内无法访问，这里指定阿里云镜像仓库地址
--kubernetes-version K8s版本，与上面安装的一致
--pod-network-cidr 
--service-cidr 集群内部虚拟网络，Pod统一访问入口
--pod-network-cidr Pod网络，，与下面部署的CNI网络组件yaml中保持一致
```   

>若初始化失败，请执行 `kubeadm reset` 进行重置。        

```shell
[root@kmaster01 ~]# mkdir -p $HOME/.kube
[root@kmaster01 ~]# cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
[root@kmaster01 ~]# chown $(id -u):$(id -g) $HOME/.kube/config
[root@kmaster01 ~]# export KUBECONFIG=/etc/kubernetes/admin.conf
```

#### 4.5 Node 加入  
Node 节点，也同样按照 `4.1 配置k8s v1.27.x 的 yum 源` 章节开始操作，直到“初始化”。          

这里就不在重复阐述操作步骤了，直接进行 node join 操作。       

```
kubeadm join 192.168.0.131:6443 --token jr06o0.fo6mxvc5jwxnbeaj \
	--discovery-token-ca-cert-hash sha256:ffa5f34ef566ef3324ec0a003092fc5f1f1d7e3de272fbb770d3273d1522c7fb
```

#### 4.6 case 1 修改 Node01 节点的 hostname，该如何重新加入集群？   
* step01 在 master 上删除这个节点 node01  
```shell
kubectl delete node [node name]     
```

* step02 在 master 上打印出加入 master 的命令     
```shell
kubeadm token create --print-join-command   
```

* step03 node01 上执行 reset      
```shell
kubeadm reset   
```

* step04 node01 上再次加入 master node 
```shell
kubeadm join .......
```

### 5.部署集群网络插件  
网络组件有很多种，只需要部署其中一个即可，推荐Calico。      
Calico是一个纯三层的数据中心网络方案，Calico支持广泛的平台，包括Kubernetes、OpenStack等。       
Calico 在每一个计算节点利用 Linux Kernel 实现了一个高效的虚拟路由器（ vRouter） 来负责数据转发，而每个 vRouter 通过 BGP 协议负责把自己上运行的 workload 的路由信息向整个 Calico 网络内传播。              
此外，Calico 项目还实现了 Kubernetes 网络策略，提供ACL功能。            

#### 5.1 下载 Calico  
```
wget https://docs.tigera.io/archive/v3.24/manifests/calico.yaml --no-check-certificate   
```

修改下载的 `calico.yaml` 中的`CALICO_IPV4POOL_CIDR` 参数，注意 `CALICO_IPV4POOL_CIDR`值要与 `kubeadm init` 命令中的 `--pod-network-cidr`参数保持一直。        

```
vim calico.yaml
...
- name: CALICO_IPV4POOL_CIDR
  value: "172.30.0.0/16"
...
```

#### 5.2 安装

* 执行 `kubectl apply -f calico.yaml` , 安装 calico。           

* 执行 `kubectl get pod -n kube-system` , 查看 calico 状态。 (等待安装成功 ...)  

* 执行 `kubectl get node -o wide`, 验证 node ip 信息正常展示。    


### 6.安装 cert-manager  
请参考 `https://cert-manager.io/docs/installation/`  
```
kubectl apply -f https://github.com/cert-manager/cert-manager/releases/download/v1.13.2/cert-manager.yaml
```

### 7.安装 ingress-nginx 

#### 7.1 下载 yaml 
请参考 `https://github.com/kubernetes/ingress-nginx` 和 `https://kubernetes.github.io/ingress-nginx/deploy/`, 将下载后的`deploy.yaml` 重命名为`ingress-nginx.yaml` 
```shell
# 下载 deploy.yml
wget https://raw.githubusercontent.com/kubernetes/ingress-nginx/controller-v1.9.4/deploy/static/provider/cloud/deploy.yaml

# 重命名
mv deploy.yaml ingress-nginx.yaml
```  

#### 7.2 修改
* 注意，安装过程中，出现镜像拉取失败，可将 yaml文件中的 `registry.k8s.io`地址改成`k8s.dockerproxy.com`。      
* 将 ingress-nginx.yaml 中的 `type: LoadBalancer` 改成 `type: NodePort` （其实可以不改，安装 MetalLB , 博主是为了简单 ）    

#### 7.3 安装 
```shell
kubectl apply -f ingress-nginx.yaml   
```

#### 7.4 验证  
通过部署服务，然后使用域名访问。      
验证示例1： hello-server 
以下是 `ingress-test.yaml` 内容：    
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  labels:
    app: hello-server
    name: hello-server
  name: hello-server
spec:
  replicas: 2
  selector:
    matchLabels:
      app: hello-server
  template:
    metadata:
      labels:
        app: hello-server
    spec:
      containers:
      - name: hello-server
        image: registry.cn-hangzhou.aliyuncs.com/cm_ns01/hello-server:latest
        ports:
        - containerPort: 9000
---
apiVersion: v1
kind: Service
metadata:
  labels:
    app: hello-server
    name: hello-server
  name: hello-server
spec:
  selector:
    app: hello-server
  ports:
  - port: 8000
    protocol: TCP
    targetPort: 9000
---
apiVersion: networking.k8s.io/v1
kind: Ingress  
metadata:
  name: ingress-host-bar
  annotations: 
    nginx.ingress.kubernetes.io/default-backend: ingress-nginx-controller
    nginx.ingress.kubernetes.io/use-regex: 'true'
spec:
  ingressClassName: nginx
  rules:
  - host: "hello.k8s.com"
    http:
      paths:
      - pathType: Prefix
        path: "/"
        backend:
          service:
            name: hello-server
            port:
              number: 8000

```

**安装并配置宿主机 hosts**    
```shell
# 安装 ingress-test.yaml
kubectl apply -f ingress-test.yaml

# 请务必一定要关闭 宿主机的 vpn

# 宿主机配置 hosts, 请注意 ip 指向的是 ingress-nginx-controller 的 pod 所在的节点 ip。
# 查看 ingress-nginx-controller pod 所在节点   
kubectl get po -n ingress-nginx -owide

# 配置 hosts
vim /etc/hosts
[ingress-nginx-controller pod 所在节点的 ip ] hello.k8s.com  

# 查询 ingress-nginx-controller svc 暴露的端口（NodePort）
kubectl get svc -n ingress-nginx
# output: 
[root@kmaster01 k8s]# kubectl get svc -n ingress-nginx
NAME                                 TYPE        CLUSTER-IP       EXTERNAL-IP   PORT(S)                      AGE
ingress-nginx-controller             NodePort    10.254.216.210   <none>        80:31836/TCP,443:32570/TCP   10h
ingress-nginx-controller-admission   ClusterIP   10.254.205.0     <none>        443/TCP                      10h

```

最后，在宿主机浏览器访问 `http://hello.k8s.com:31836`。     
>注意，宿主机配置 hosts 映射时，ip 一定要是 ingress-nginx-controller 的 pod 所在的节点 ip, 例如，我这边 pod 部署在 knode02 上。       


### 8.安装 k9s 

### 9.安装 NFS StorageClass 插件    
在K8s上部署应用，大多数场景下需要使用持久化存储，通常采用PV+PVC的方式提供持久化存储。       

PersistentVolume (PV) 是外部存储系统中的一块存储空间，通常由管理员创建和维护。与 Volume 一样，PV 具有持久性，生命周期独立于 Pod，也就是说，即使Pod被删除，PV的存储仍在。        

PersistentVolumeClaim (PVC) 是对 PV 的申请 (Claim)。PVC 通常由普通用户创建和维护。需要为 Pod 分配存储资源时，用户可以创建一个 PVC，指明存储资源的容量大小和访问模式（比如只读）等信息，Kubernetes 会查找并提供满足条件的 PV。         

在一个大规模的Kubernetes集群里，可能有成千上万个PVC，这就意味着运维人员必须事先创建这些PV。为此，Kubernetes提供了一套可以自动创建PV的机制，即Dynamic Provisioning，而这个机制的核心在于StorageClass这个API对象。Kubernetes能够根据用户提交的PVC，找到一个对应的StorageClass，之后Kubernetes就会调用该StorageClass声明的存储插件，进而创建出需要的PV。       

在集群中，StorageClass的存储插件使用NFS。     

#### 9.1 安装 NFS
所有节点都全部执行，且该方案并非是 HA
```shell
yum install -y nfs-utils  
```

#### 9.2 配置 master 节点 
```shell
# 在 Master 执行以下命令    
echo "/nfs/data/ *(insecure,rw,sync,no_root_squash)" > /etc/exports

# 执行以下命令，启动 nfs 服务 &  创建共享目录
mkdir -p /nfs/data

# 在 Master 执行
systemctl enable rpcbind
systemctl enable nfs-server
systemctl start rpcbind
systemctl start nfs-server

# 使配置生效
exportfs -r

# 检查配置是否生效
exportfs
# output: 
[root@kmaster01 ~]# exportfs
/nfs/data     	<world>
```

#### 9.3 配置 NFS Client，在子节点执行
```shell
showmount -e kmaster01      

# 创建目录
mkdir -p /nfs/data        

# 挂载    
mount -t nfs kmaster01:/nfs/data /nfs/data        

```

#### 9.4 node 节点部署
请参考`https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner`的 `With Kustomize`章节。   

>注意： 目前 knode 节点并没有 kmaster 的 admin config 配置文件，所以，若在子节点执行 `kubectl apply -f  ...`, 会有以下异常信息：  
```
The connection to the server localhost:8080 was refused - did you specify the right host or port?
```
为了解决以上问题，需`将主节点的admin.conf拷贝到从节点，设置环境变量`    

```shell
# 在子节点 操作
# 上传 config 
mkdir -p $HOME/.kube
scp /root/.kube/config root@knode01:/root/.kube/
scp /root/.kube/config root@knode02:/root/.kube/
scp /root/.kube/config root@knode03:/root/.kube/

# 配置环境变量
vim /etc/profile
# 添加 KUBECONFIG
export KUBECONFIG=/root/.kube/conf

source /etc/profile
```

**部署**  

```shell
# 下载 deploy 文件  
wget https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner/tree/master/deploy    
```

**1> 创建 ServiceAccount**      
现在的 Kubernetes 集群大部分是基于 RBAC 的权限控制，所以创建一个一定权限的 ServiceAccount 与后面要创建的 “NFS Provisioner” 绑定，赋予一定的权限。   
```
kubectl apply -f deploy/rbac.yaml     
```
**2> 部署 NFS-Subdir-External-Provisioner**  
创建 NFS Provisioner 部署文件，这里将其部署到 “default” Namespace 中。   

>注意，`deploy/deployment.yaml` 需要修改，其原因是需要与 环境中配置的 nfs path 以及 master ip 保持一致。  

* 修改 NFS_SERVER 参数指向 kmaster， NFS_PATH 配置 /nfs/data       
```
    env:
      - name: PROVISIONER_NAME
        value: k8s-sigs.io/nfs-subdir-external-provisioner
      - name: NFS_SERVER
        value: kmaster01
      - name: NFS_PATH
        value: /nfs/data
volumes:
  - name: nfs-client-root
    nfs:
      server: kmaster01
      path: /nfs/data
```

* 将镜像源地址 `registry.k8s.io`地址改成`k8s.dockerproxy.com`    

kubectl apply -f deploy/deployment.yaml 。    


**3> 创建 NFS SotageClass**      
创建一个 StoageClass，声明 NFS 动态卷提供者名称为 “nfs-storage”。    
class.yaml        
```
apiVersion: storage.k8s.io/v1
kind: StorageClass
metadata:
  name: nfs-storage
  annotations:
    storageclass.kubernetes.io/is-default-class: "false"
provisioner: k8s-sigs.io/nfs-subdir-external-provisioner # or choose another name, must match deployment's env PROVISIONER_NAME'
parameters:
  archiveOnDelete: "false"
```

kubectl apply -f class.yaml 。

**4> 创建 PVC 和 Pod 进行测试**   

```
kind: PersistentVolumeClaim
apiVersion: v1
metadata:
  name: test-pvc
spec:
  storageClassName: nfs-storage
  accessModes:
    - ReadWriteMany
  resources:
    requests:
      storage: 1Mi
``

查看 PVC 状态是否与 PV 绑定
利用 Kubectl 命令获取 pvc 资源，查看 STATUS 状态是否为 `Bound`。            
```shell
[root@kmaster01 deploy]# kubectl get pvc test-pvc
NAME       STATUS   VOLUME                                     CAPACITY   ACCESS MODES   STORAGECLASS   AGE
test-pvc   Bound    pvc-102e49b0-d927-4ca5-a57b-6dc075ce6e8c   1Mi        RWX            nfs-storage    15s
```

**5> 创建测试 Pod 并绑定 PVC**         
创建一个测试用的 Pod，指定存储为上面创建的 PVC，然后创建一个文件在挂载的 PVC 目录中，然后进入 NFS 服务器下查看该文件是否存入其中. 
```
[root@kmaster01 deploy]# cat test-pod.yaml 
kind: Pod
apiVersion: v1
metadata:
  name: test-pod
spec:
  containers:
  - name: test-pod
    image: busybox:stable
    command:
      - "/bin/sh"
    args:
      - "-c"
      - "touch /mnt/SUCCESS && exit 0 || exit 1"  # 创建一个名称为"SUCCESS"的文件
    volumeMounts:
      - name: nfs-pvc
        mountPath: "/mnt"
  restartPolicy: "Never"
  volumes:
    - name: nfs-pvc
      persistentVolumeClaim:
        claimName: test-pvc   # 指向 pvc 名称
```

kubectl apply -f test-pod.yaml     。


**6> 进入 NFS 服务器验证是否创建对应文件**
进入 NFS Server 节点的 nfs 挂载目录，查看是否存在 Pod 中创建的文件：   
```shell
# 进入 kmaster01

[root@kmaster01 deploy]# cd /nfs/data/
[root@kmaster01 data]# ls
default-test-pvc-pvc-102e49b0-d927-4ca5-a57b-6dc075ce6e8c
[root@kmaster01 data]# cd default-test-pvc-pvc-102e49b0-d927-4ca5-a57b-6dc075ce6e8c/
[root@kmaster01 default-test-pvc-pvc-102e49b0-d927-4ca5-a57b-6dc075ce6e8c]# ls
SUCCESS
[root@kmaster01 default-test-pvc-pvc-102e49b0-d927-4ca5-a57b-6dc075ce6e8c]#
```


>可以看到已经生成 SUCCESS 该文件，并且可知通过 NFS Provisioner 创建的目录命名方式为“namespace名称-pvc名称-pv名称”，pv 名称是随机字符串，所以每次只要不删除 PVC，那么 Kubernetes 中的与存储绑定将不会丢失，要是删除 PVC 也就意味着删除了绑定的文件夹，下次就算重新创建相同名称的 PVC，生成的文件夹名称也不会一致，因为 PV 名是随机生成的字符串，而文件夹命名又跟 PV 有关,所以删除 PVC 需谨慎。        



refer 
1.https://v1-27.docs.kubernetes.io/zh-cn/docs/setup/production-environment/tools/kubeadm/install-kubeadm/     
2.https://kubernetes.io/zh-cn/releases/   
3.https://blog.csdn.net/jcmj123456/article/details/130275190      
4.https://blog.51cto.com/flyfish225/6187876   
5.https://developer.aliyun.com/article/856853     
6.https://blog.csdn.net/whatzhang007/article/details/112579182      


