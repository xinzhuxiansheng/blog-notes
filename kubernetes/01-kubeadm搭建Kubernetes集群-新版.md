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
$ yum remove docker docker-common docker-selinux docker-engine
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
yum -y install docker-ce-24.0.2-1.el7
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

#### 5.按照集群配置克隆虚机 

略

#### 6.安装Harbor   
