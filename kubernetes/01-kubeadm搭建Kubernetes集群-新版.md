## kubeadm搭建Kubernetes集群    

>本环境基于VMware虚拟环境，

### 1.集群规划    
|   ip  |  域名  | 备注| 安装软件|
|  ----  | ----  |----  |----  |
|  192.168.0.121 | master | 主节点 |Docker Kubeadm kubelet kubectl flannel |
|  192.168.0.122 | node1 |从节点 1 |Docker Kubeadm kubelet kubectl |
|  192.168.0.123 | node2 |从节点 2 |Docker Kubeadm kubelet kubectl|

### 2.基础环境（每个节点）  

>针对所有节点   

- 3台虚拟机CentOS7.x-86_x64 
- 硬件配置：2GB或更多RAM，2个CPU或更多CPU，硬盘30GB或更多   
- 集群中所有机器之间网络互通    
- 可以访问外网，需要拉取镜像    
- 禁止swap分区  

#### 升级系统内核   
Docker要求CentOS系统的内核版本高于3.10，查看本页面的前提条件来验证你的CentOS版本
是否支持Docker，通过uname -r命令查看你当前的内核版本

>CentOS7升级内核，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/02-CentOS7%E5%8D%87%E7%BA%A7%E5%86%85%E6%A0%B8.md`。   

#### 关闭防火墙  

>CentOS7关闭防火墙，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/05-Centos7%E8%AE%BE%E7%BD%AEfirewall%E9%98%B2%E7%81%AB%E5%A2%99.md`。    

#### 安装软件包 
```shell
yum install -y epel-release
yum install -y chrony conntrack ipvsadm ipset jq iptables curl sysstat libseccomp wget socat git vim  lrzsz wget man tree rsync gcc gcc-c++ cmake telnet
```
kube-proxy使用ipvs模式，ipvsadm为ipvs的管理工具 
etcd集群各机器需要时间同步，chrony用于系统时间同步  

#### 关闭swap    

>CentOS7关闭Swap，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/07-CentOS7%E5%85%B3%E9%97%ADSwap.md`。    

#### 关闭selinux 

>CentOS7关闭selinux，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/08-CentOS7%E5%85%B3%E9%97%ADselinux.md`。   

#### 添加Hosts映射关系   
修改hostname    
```shell
# vim /etc/hostname   # 编辑成相应的节点名称
kubeadm01
```

>CentOS7修改Hosts映射，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/09-CentOS7%E4%BF%AE%E6%94%B9Hosts%E6%98%A0%E5%B0%84.md`。 


#### 设置时区和同步时间  


>CentOS7设置时区和同步时间，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/06-CentOS7%E8%AE%BE%E7%BD%AE%E6%97%B6%E5%8C%BA%E5%92%8C%E5%90%8C%E6%AD%A5%E6%97%B6%E9%97%B4.md`。    

#### 其他操作
```shell
## 关闭无关的服务
systemctl stop postfix && systemctl disable postfix 

## 创建相关目录
mkdir -p /opt/k8s/{bin,work} /etc/{kubernetes,etcd}/cert    
```


#### 优化内核参数
```shell
cat > sysctl.conf <<EOF
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

cp sysctl.conf  /etc/sysctl.d/sysctl.conf
sysctl -p /etc/sysctl.d/sysctl.conf
```
关闭tcp_tw_recycle，否则与NAT冲突，可能导致服务不通 

#### 设置开机加载k8s配置文件 
```shell
cat >>/etc/modules-load.d/kubernetes.conf<<EOF
ip_vs_dh
ip_vs_ftp
ip_vs
ip_vs_lblc
ip_vs_lblcr
ip_vs_lc
ip_vs_nq
ip_vs_pe_sip
ip_vs_rr
ip_vs_sed
ip_vs_sh
ip_vs_wlc
ip_vs_wrr
nf_conntrack_ipv4
overlay
br_netfilter
EOF
``` 

设置开机加载IPVS模块：  
```shell
systemctl enable systemd-modules-load.service   # 设置开机加载内核模块
lsmod | grep -e ip_vs -e nf_conntrack_ipv4      # 重启后检查ipvs模块是否加载
```


### 3.Docker安装（每个节点）      

#### 3.1卸载旧版本（如果安装过旧版本的）    
```shell
$ rpm -qa | grep docker
$ yum remove docker docker-common docker-selinux docker-ce docker-ce-cli
``` 

#### 3.2安装需要的软件包    
yum-util提供yum-config-manager功能，另外两个是devicemapper驱动依赖    
```shell
yum install -y yum-utils device-mapper-persistent-data lvm2 
```

#### 3.3设置yum源   
```shell
$ yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo
$ yum-config-manager --add-repo http://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo
```

#### 3.4更新yum缓存 
查看所有仓库中所有docker版本，并选择特定版本安装   
```shell
$ yum clean all                  # 清理缓存
$ yum makecache fast      # 生成新的缓存
$ yum list docker-ce --showduplicates | sort -r
``` 

#### 3.5安装最新稳定版Docker     
```shell    
yum -y install docker-ce-20.10.9-3.el7 docker-ce-cli-20.10.9-3.el7
``` 

#### 3.6配置Docker  
vim /etc/docker/daemon.json
```
{
  "exec-opts": ["native.cgroupdriver=systemd"]
}
```
然后重启Docker

```shell
systemctl restart docker
``` 


### 4.安装Kubernetes基础组建（kubeadm，kubelet和kubectl）   

#### 4.1添加kubernetes软件源
然后我们还需要配置一下yum的k8s软件源    

```
cat > /etc/yum.repos.d/kubernetes.repo << EOF
[kubernetes]
name=Kubernetes
baseurl=https://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64
enabled=1
gpgcheck=0
repo_gpgcheck=0
gpgkey=https://mirrors.aliyun.com/kubernetes/yum/doc/yum-key.gpg https://mirrors.aliyun.com/kubernetes/yum/doc/rpm-package-key.gpg
EOF
```     

#### 4.2安装kubeadm，kubelet和kubectl(指定版本)
```shell
# 查看安装包
yum list kubelet kubeadm kubectl  --showduplicates|sort -r  

# 制定版本安装  
yum install -y kubelet-1.23.17 kubeadm-1.23.17 kubectl-1.23.17

# 设置开机启动和启动kubelet
systemctl enable kubelet && systemctl start kubelet
```

### 5.按照集群配置克隆虚机 

基于当前的虚拟机，关机，使用VMWare或VirutalBox的虚拟机克隆功能，自制kubeadm02~kubeadm05等4台虚拟机，由于IP，hostname等会重复，启动后修改hostname及IP，几台做了初始化配置，下载了镜像的主机就ready了 

### 6.安装Harbor   

>请参考：`https://github.com/xinzhuxiansheng/blog-notes/blob/master/kubernetes/02-harbor%E5%AE%89%E8%A3%85.md`。   

### 7.配置集群      

#### 7.1下载基础镜像

`环境准备工作`: kubeadm工具安装时，需从``仓库拉取镜像，因国内无法正常访问，所以需先从阿里云下载并重新打包标签，操作如下：   
```shell
[root@kubeadm01 ~]# kubeadm config images list
I0613 10:15:37.425466    9626 version.go:256] remote version is much newer: v1.27.2; falling back to: stable-1.23
registry.k8s.io/kube-apiserver:v1.23.17
registry.k8s.io/kube-controller-manager:v1.23.17
registry.k8s.io/kube-scheduler:v1.23.17
registry.k8s.io/kube-proxy:v1.23.17
registry.k8s.io/pause:3.6
registry.k8s.io/etcd:3.5.6-0
registry.k8s.io/coredns/coredns:v1.8.6
```

`使用脚本自动下载镜像`      
镜像下载域名：  
registry.aliyuncs.com/google_containers
registry.cn-hangzhou.aliyuncs.com/google_containers 

>注意 脚本中的变量版本，请参考`kubeadm config images list`得到的软件版本号  

```shell    
[root@kubeadm01 ~]# cat>kubeadm_config_images_list.sh<<EOF
#!/bin/bash

KUBE_VERSION=v1.23.9
PAUSE_VERSION=3.6
CORE_DNS_VERSION=1.8.6
CORE_DNS_VVERSION=v1.8.6
ETCD_VERSION=3.5.1-0

# pull kubernetes images from hub.docker.com
docker pull registry.aliyuncs.com/google_containers/kube-proxy-amd64:$KUBE_VERSION
docker pull registry.aliyuncs.com/google_containers/kube-controller-manager-amd64:$KUBE_VERSION
docker pull registry.aliyuncs.com/google_containers/kube-apiserver-amd64:$KUBE_VERSION
docker pull registry.aliyuncs.com/google_containers/kube-scheduler-amd64:$KUBE_VERSION
# pull aliyuncs mirror docker images
docker pull registry.aliyuncs.com/google_containers/pause:$PAUSE_VERSION
docker pull coredns/coredns:$CORE_DNS_VERSION
docker pull registry.aliyuncs.com/google_containers/etcd:$ETCD_VERSION

# retag to k8s.gcr.io prefix
docker tag registry.aliyuncs.com/google_containers/kube-proxy-amd64:$KUBE_VERSION  k8s.gcr.io/kube-proxy:$KUBE_VERSION
docker tag registry.aliyuncs.com/google_containers/kube-controller-manager-amd64:$KUBE_VERSION k8s.gcr.io/kube-controller-manager:$KUBE_VERSION
docker tag registry.aliyuncs.com/google_containers/kube-apiserver-amd64:$KUBE_VERSION k8s.gcr.io/kube-apiserver:$KUBE_VERSION
docker tag registry.aliyuncs.com/google_containers/kube-scheduler-amd64:$KUBE_VERSION k8s.gcr.io/kube-scheduler:$KUBE_VERSION
docker tag registry.aliyuncs.com/google_containers/pause:$PAUSE_VERSION k8s.gcr.io/pause:$PAUSE_VERSION
docker tag coredns/coredns:$CORE_DNS_VERSION k8s.gcr.io/coredns:$CORE_DNS_VVERSION
docker tag registry.aliyuncs.com/google_containers/etcd:$ETCD_VERSION k8s.gcr.io/etcd:$ETCD_VERSION

# untag origin tag, the images won't be delete.
docker rmi registry.aliyuncs.com/google_containers/kube-proxy-amd64:$KUBE_VERSION
docker rmi registry.aliyuncs.com/google_containers/kube-controller-manager-amd64:$KUBE_VERSION
docker rmi registry.aliyuncs.com/google_containers/kube-apiserver-amd64:$KUBE_VERSION
docker rmi registry.aliyuncs.com/google_containers/kube-scheduler-amd64:$KUBE_VERSION
docker rmi registry.aliyuncs.com/google_containers/pause:$PAUSE_VERSION
docker rmi coredns/coredns:$CORE_DNS_VERSION
docker rmi registry.aliyuncs.com/google_containers/etcd:$ETCD_VERSION

EOF

#修改权限，执行
[root@kubeadm01 ~]# chmod +x kubeadm_config_images_list.sh
[root@kubeadm01 ~]# sh kubeadm_config_images_list.sh
```

`确认镜像下载完成：`    
```shell
[root@kubeadm01 ~]# docker images | grep k8s.gcr.io
k8s.gcr.io/kube-proxy                v1.23.9             c3d62d6fe412        13 days ago         117MB
k8s.gcr.io/kube-controller-manager   v1.23.9             ffce5e64d915        13 days ago         162MB
k8s.gcr.io/kube-apiserver            v1.23.9            56acd67ea15a        13 days ago         173MB
k8s.gcr.io/kube-scheduler            v1.23.9            0e0972b2b5d1        13 days ago         95.3MB
k8s.gcr.io/pause                     3.6                 80d28bedfe5d        5 months ago        683kB
k8s.gcr.io/coredns                   1.8.6               67da37a9a360        6 months ago        43.8MB
k8s.gcr.io/etcd                      3.5.1-0             303ce5db0e90        9 months ago        288MB
```

#### 7.2初始化master节点

```shell
kubeadm init \
    --apiserver-advertise-address=0.0.0.0 \
    --apiserver-bind-port=6443 \
    --kubernetes-version=v1.23.17 \
    --pod-network-cidr=172.30.0.0/16 \
    --service-cidr=10.254.0.0/16 \
    --image-repository=k8s.gcr.io \
    --ignore-preflight-errors=swap \
    --token-ttl=0
```
Pod容器实例的IP规划   --pod-network-cidr=172.30.0.0/16      
集群内ServiceIP规划   --service-cidr=10.254.0.0/16    

>如果初始化过程被中断可以使用下面命令来恢复 `kubeadm reset`     

`配置kubectl的权限：`   
```shell
# 在当前用户家目录下创建.kube目录并配置访问集群的config 文件
[root@kubeadm01 ~]# mkdir -p $HOME/.kube
[root@kubeadm01 ~]# sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
[root@kubeadm01 ~]# sudo chown $(id -u):$(id -g) $HOME/.kube/config   #如果用户不同，不是使用root，可以改变下属主

[root@kubeadm01 ~]# [root@kubeadm01 ~]# kubectl get pods -n kube-system
NAME                                READY   STATUS    RESTARTS   AGE
coredns-66bff467f8-jldn6            0/1     Pending   0          7h18m
coredns-66bff467f8-lvzsm            0/1     Pending   0          7h18m
etcd-kubeadm01                      1/1     Running   1          7h19m
kube-apiserver-kubeadm01            1/1     Running   1          7h19m
kube-controller-manager-kubeadm01   1/1     Running   1          7h19m
kube-proxy-k2jpn                    1/1     Running   1          7h18m
kube-scheduler-kubeadm01            1/1     Running   1          7h19m
```

`配置网络kube-flannel`（只在master节点进行）    
```
[root@kubeadm01 ~]# mkdir flannel && cd flannel
[root@kubeadm01 ~]# wget https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml
或者
[root@kubeadm01 ~]# wget https://raw.githubusercontent.com/flannel-io/flannel/master/Documentation/kube-flannel.yml

[root@kubeadm01 ~]# vim kube-flannel.yml
 net-conf.json: |
    {
      "Network": "172.30.0.0/16",
      "Backend": {
        "Type": "vxlan"
      }
    }
# 修改Network的value为172.30.0.0/16
# 因为在启动Master时指定了相关参数  --pod-network-cidr=172.30.0.0/16

[root@kubeadm01 ~]# kubectl create -f kube-flannel.yml
podsecuritypolicy.policy/psp.flannel.unprivileged created
clusterrole.rbac.authorization.k8s.io/flannel created
clusterrolebinding.rbac.authorization.k8s.io/flannel created
serviceaccount/flannel created
configmap/kube-flannel-cfg created
daemonset.apps/kube-flannel-ds-amd64 created
daemonset.apps/kube-flannel-ds-arm64 created
daemonset.apps/kube-flannel-ds-arm created
daemonset.apps/kube-flannel-ds-ppc64le created
daemonset.apps/kube-flannel-ds-s390x created

# 查看网络情况，所有Pod都是Running状态，存在kube-flannel-ds-amd64的Pod
[root@kubeadm01 ~]# kubectl get pods -n kube-system
NAMESPACE     NAME                                READY   STATUS    RESTARTS   AGE
kube-system   coredns-66bff467f8-jldn6            0/1     Running   0          7h28m
kube-system   coredns-66bff467f8-lvzsm            1/1     Running   0          7h28m
kube-system   etcd-kubeadm01                      1/1     Running   1          7h28m
kube-system   kube-apiserver-kubeadm01            1/1     Running   1          7h28m
kube-system   kube-controller-manager-kubeadm01   1/1     Running   1          7h28m
kube-system   kube-flannel-ds-amd64-pdprm         1/1     Running   0          30s
kube-system   kube-proxy-k2jpn                    1/1     Running   1          7h28m
kube-system   kube-scheduler-kubeadm01            1/1     Running   1          7h28m

# 查看节点情况
[root@kubeadm01 ~]# kubectl get nodes
NAME        STATUS   ROLES    AGE     VERSION
kubeadm01   Ready    master   7h34m   v1.18.6
```


`移除节点`  
```shell
kubectl drain 主机名
kubectl delete node 主机名
``` 

### 创建node节点
注意：要在添加的Node节点执行
 加入集群，注意在命令尾部加上–ignore-preflight-errors=Swap ，以忽略k8s对主机swap的检查  

```
[root@kubeadm02 ~]# kubeadm join 192.168.10.71:6443  \
       --ignore-preflight-errors=swap \
     --token 521zmo.cvejn9obrtvhmsem \
    --discovery-token-ca-cert-hash sha256:df7486a25aa29f79bb6f6014933bdfa17ac8d8536b2cb5c274909286f35fc207

W0730 18:29:52.911032   78801 join.go:346] [preflight] WARNING: JoinControlPane.controlPlane settings will be ignored when control-plane flag is not set.
[preflight] Running pre-flight checks
[preflight] Reading configuration from the cluster...
[preflight] FYI: You can look at this config file with 'kubectl -n kube-system get cm kubeadm-config -oyaml'
[kubelet-start] Downloading configuration for the kubelet from the "kubelet-config-1.18" ConfigMap in the kube-system namespace
[kubelet-start] Writing kubelet configuration to file "/var/lib/kubelet/config.yaml"
[kubelet-start] Writing kubelet environment file with flags to file "/var/lib/kubelet/kubeadm-flags.env"
[kubelet-start] Starting the kubelet
[kubelet-start] Waiting for the kubelet to perform the TLS Bootstrap...

This node has joined the cluster:
* Certificate signing request was sent to apiserver and a response was received.
* The Kubelet was informed of the new secure connection details.

Run 'kubectl get nodes' on the control-plane to see this node join the cluster.
``` 

`查看节点`    
```
kubectl get nodes
``` 

以上已完成集群安装  
```
[root@kubeadm01 ~]# kubectl  get pods --all-namespaces -o wide
NAMESPACE      NAME                                READY   STATUS    RESTARTS   AGE     IP              NODE        NOMINATED NODE   READINESS GATES
kube-flannel   kube-flannel-ds-66t9v               1/1     Running   0          7m19s   192.168.0.123   kubeadm03   <none>           <none>
kube-flannel   kube-flannel-ds-7nr9n               1/1     Running   0          33m     192.168.0.122   kubeadm02   <none>           <none>
kube-flannel   kube-flannel-ds-jnrpq               1/1     Running   0          54m     192.168.0.121   kubeadm01   <none>           <none>
kube-flannel   kube-flannel-ds-kqkfx               1/1     Running   0          7m19s   192.168.0.125   kubeadm05   <none>           <none>
kube-flannel   kube-flannel-ds-rkh8n               1/1     Running   0          7m19s   192.168.0.124   kubeadm04   <none>           <none>
kube-system    coredns-74f7f66b6f-cbxgw            1/1     Running   0          61m     172.30.0.3      kubeadm01   <none>           <none>
kube-system    coredns-74f7f66b6f-l55c7            1/1     Running   0          61m     172.30.0.2      kubeadm01   <none>           <none>
kube-system    etcd-kubeadm01                      1/1     Running   1          61m     192.168.0.121   kubeadm01   <none>           <none>
kube-system    kube-apiserver-kubeadm01            1/1     Running   1          61m     192.168.0.121   kubeadm01   <none>           <none>
kube-system    kube-controller-manager-kubeadm01   1/1     Running   1          61m     192.168.0.121   kubeadm01   <none>           <none>
kube-system    kube-proxy-4l9f5                    1/1     Running   0          61m     192.168.0.121   kubeadm01   <none>           <none>
kube-system    kube-proxy-fxsh8                    1/1     Running   0          7m19s   192.168.0.123   kubeadm03   <none>           <none>
kube-system    kube-proxy-kblk2                    1/1     Running   0          7m19s   192.168.0.124   kubeadm04   <none>           <none>
kube-system    kube-proxy-rls54                    1/1     Running   0          33m     192.168.0.122   kubeadm02   <none>           <none>
kube-system    kube-proxy-zgng9                    1/1     Running   0          7m19s   192.168.0.125   kubeadm05   <none>           <none>
kube-system    kube-scheduler-kubeadm01            1/1     Running   1          61m     192.168.0.121   kubeadm01   <none>           <none>
```