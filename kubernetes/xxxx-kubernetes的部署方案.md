
## Kubernetes的部署方案

### 部署目标
- 在所有节点上安装Docker和kubeadm
- 部署Kubernetes Master
- 部署容器网络插件
### 部署架构
|   ip  |  域名  | 备注| 安装软件|
|  ----  | ----  |----  |----  |
|  192.168.99.101 | master | 主节点 |Docker Kubeadm kubelet kubectl flannel |
|  192.168.99.102 | node1 |从节点 1 |Docker Kubeadm kubelet kubectl |
|  192.168.99.103 | node2 |从节点 2 |Docker Kubeadm kubelet kubectl|
### 环境准备
- 3台虚拟机CentOS7.x-86_x64
- 硬件配置：2GB或更多RAM，2个CPU或更多CPU，硬盘30GB或更多
- 集群中所有机器之间网络互通
- 可以访问外网，需要拉取镜像
- 禁止swap分区

# 5-5 Virtualbox 虚拟机配置双网卡实现固定IP
- Virtualbox安装 CentOS
- 配置虚机双网卡,实现固定 IP，且能访问外网
网卡 1： 仅主机host-only
网卡 2： 网络转换地址NAT
查看虚拟机网络，点击管理—>主机网络管理器，记住ip地址（192.168.99.1），并选择“手动配置网卡”。
- 重启虚拟机，此时在虚拟机 ping www.baidu.com 是返回成功的。
- 设置外部网络访问虚拟机
设置静态ip地址，编辑网络配置文件，编辑网络设置文件
```
vi /etc/sysconfig/network-scripts/ifcfg-enp0s3
TYPE=Ethernet
PROXY_METHOD=none
BROWSER_ONLY=no
#BOOTPROTO=dhcp
DEFROUTE=yes
IPV4_FAILURE_FATAL=no
IPV6INIT=yes
IPV6_AUTOCONF=yes
IPV6_DEFROUTE=yes
IPV6_FAILURE_FATAL=no
IPV6_ADDR_GEN_MODE=stable-privacy
NAME=enp0s3
UUID=08012b4a-d6b1-41d9-a34d-e0f52a123e7a
DEVICE=enp0s3
ONBOOT=yes
BOOTPROTO=static
IPADDR=192.168.99.101
```
- 重启网络
```
systemctl restart network
```

- 查看 enp0s3 网卡的 ip
```
[root@localhost Final]#ip addr |grep 192
inet 192.168.99.101/24 brd 192.168.99.255 scope global noprefixroute enp0s3
```
- 此时虚拟机既可以访问外网，也能够和宿主机( 192.168.31.178)进行通信
```
ping 192.168.31.178
PING 192.168.31.178 (192.168.31.178): 56 data bytes
64 bytes from 192.168.31.178: icmp_seq=0 ttl=64 time=0.060 ms
```
- 使用iTerm2 连接虚拟机


# 5-6 配置虚拟机 Yum 源，iptables
- 删除原有的yum源：
```
rm -f /etc/yum.repos.d/*
```
- 重新下载阿里云的yum源：
```
wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo

```

- 列出yum各软件包：
```
yum list
```
- 清除缓存：
```
yum clean all
```

# 5-7 Master节点安装 kubeadm, kubelet and kubectl

- 安装基本软件包
```
[root@localhost ~]# yum install wget net‐tools vim bash‐comp* ‐y

```

- 设置主机名，管理节点设置主机名为master

```
[root@master ~]# hostnamectl set‐hostname master
[root@master ~]# su ‐
```
- 配置 Master 和 work 节点的域名
```
vim /etc/hosts
 192.168.99.101 master
 192.168.99.102 node1
 192.168.99.103 node2
```
- 关闭 防火墙
```
systemctl stop firewalld
systemctl disable firewalld
```

- 关闭 SeLinux
```
setenforce 0
sed -i "s/SELINUX=enforcing/SELINUX=disabled/g" /etc/selinux/config
```

- 关闭 swap
```
swapoff -a
yes | cp /etc/fstab /etc/fstab_bak
cat /etc/fstab_bak |grep -v swap > /etc/fstab
```

- 配置Docker, K8S的阿里云yum源
```
[root@master ~]# cat >>/etc/yum.repos.d/kubernetes.repo <<EOF
[kubernetes]
name=Kubernetes
baseurl=https://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://mirrors.aliyun.com/kubernetes/yum/doc/yum-key.gpg https://mirrors.aliyun.com/kubernetes/yum/doc/rpm-package-key.gpg
EOF
[root@master ~]# wget https://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo -O /etc/yum.repos.d/docker-ce.repo
[root@master ~]# yum clean all
[root@master ~]# yum repolist
```
- 安装并启动 docker
```
yum install -y docker-ce.x86_64 docker-ce-cli.x86_64 containerd.io.x86_64

mkdir /etc/docker

cat > /etc/docker/daemon.json <<EOF
{
  "registry-mirrors": ["https://registry.cn-hangzhou.aliyuncs.com"],
  "exec-opts": ["native.cgroupdriver=systemd"]
}
EOF
```
- 编辑/usr/lib/systemd/system/docker.service (这一步 无需做)
```
ExecStart=/usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock --exec-opt native.cgroupdriver=systemd
```
```
# Restart Docker
systemctl daemon-reload
systemctl enable docker
systemctl restart docker
```
此时查看 docker info，可以看到默认 Cgroup Driver为 systemd

- 卸载旧版本
```
yum remove -y kubelet kubeadm kubectl （无需做）
```
- 安装kubelet、kubeadm、kubectl
```
yum install -y kubelet kubeadm kubectl
yum install -y --setopt=obsoletes=0 kubelet-1.23.6 kubeadm-1.23.6 kubectl-1.23.6 （推荐用，因为v1.24.0以后，Kubernetes不支持docker）
yum install -y kubelet.x86_64 kubeadm.x86_64 kubectl.x86_64（用上面替换就行）
```

- 重启 docker，并启动 kubelet
```
systemctl enable kubelet && systemctl start kubelet
```

# 5-8 初始化Master 节点
- 将桥接的IPv4流量传递到iptables的链
```
echo "1" >/proc/sys/net/bridge/bridge-nf-call-iptables
vi /etc/sysctl.d/k8s.conf 
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1 
```
- 初始化主节点
```
kubeadm init --kubernetes-version=1.19.2 \
--apiserver-advertise-address=10.168.99.101 \
--image-repository registry.aliyuncs.com/google_containers \
--service-cidr=10.1.0.0/16 \
--pod-network-cidr=10.244.0.0/16

mkdir -p $HOME/.kube
sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
sudo chown $(id -u):$(id -g) $HOME/.kube/config
```
- 安装网络插件 Flannel
```
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml
```
- 查看是否成功创建flannel网络
```
ifconfig |grep flannel
```

- 重置 kubeadm
```
kubeadm reset
```

# 5-9 安装配置 worker Node节点
- 初始虚拟机，Centos，配置双网卡
![图片描述](//img.mukewang.com/wiki/5f81ad9709adac0715340828.jpg)
![图片描述](//img.mukewang.com/wiki/5f81ada70971ea2315140770.jpg)
注意 clone snapshot 虚拟机时，选择'Generate new MAC address'。
- ssh 免密登录
- 设置 ip 地址为 192.168.99.102
- 配置域名
```
hostnamectl set-hostname node1
vi /etc/hosts
192.168.99.101 master
192.168.99.102 node1
192.168.99.103 node2
```
- 配置阿里云 yum 源
```
[root@master ~]# cat >>/etc/yum.repos.d/kubernetes.repo <<EOF
[kubernetes]
name=Kubernetes
baseurl=https://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://mirrors.aliyun.com/kubernetes/yum/doc/yum-key.gpg https://mirrors.aliyun.com/kubernetes/yum/doc/rpm-package-key.gpg
EOF
[root@master ~]# wget https://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo -O /etc/yum.repos.d/docker-ce.repo
[root@master ~]# yum clean all
[root@master ~]# yum repolist
```
- 安装基础软件
```
yum install bash‐comp* vim net‐tools wget ‐y
```
- 安装 Docker，Kubeadm，Kubectl，kubelet
```
yum install docker-ce -y
systemctl start docker
systemctl enable docker
yum install kubelet kubeadm kubectl -y
systemctl enable kubelet
```
- kubadm join 加入集群
```
kubeadm join 192.168.99.101:6443 --token vrqf1w.dyg1wru7nz0ut9jz    --discovery-token-ca-cert-hash sha256:1832d6d6c8386de5ecb1a7f512cfdef27a6d14ef901ffbe7d3c01d999d794f90
```
默认token的有效期为24小时，当过期之后，该token就不可用了。解决方法如下：

重新生成新的token，在master端执行
```
kubeadm token create --print-join-command
```
- 将 master 节点的 admin.conf 拷贝到 node1
```
scp /etc/kubernetes/admin.conf root@node1:/etc/kubernetes/
```
- 配置 Kubeconfig 环境变量
```
echo "export KUBECONFIG=/etc/kubernetes/admin.conf" >> ~/.bash_profile
source ~/.bash_profile
```
- 安装 flannel 网络插件
```
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml
```
- 将master节点下面 /etc/cni/net.d/下面的所有文件拷贝到node节点上

在node1节点上面创建目录：
`mkdir -p /etc/cni/net.d/`

在master： 
`scp /etc/cni/net.d/* root@nodeip:/etc/cni/net.d/`

执行命令：
`kubectl get nodes 查看 node 节点处于ready状态`

- 检查集群状态
稍等几分钟，在master节点输入命令检查集群状态.
```
kubectl get nodes
```