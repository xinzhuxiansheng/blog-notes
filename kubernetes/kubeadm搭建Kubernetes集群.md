
## 机器分布
| ip      |    role | hostname  |
| :-------- | --------:| :--: |
| 10.4.7.151  | master |  k8smaster  |
| 10.4.7.152     |   node1 |  k8snode1  |
| 10.4.7.153      |    node2 | k8snode2  |


## 升级内核(每台机器)
```
内核版本升级
1.操作系统版本选择：centOS 7.9，内核选择5.4
2.导入ELRepo公钥
rpm --import https://www.elrepo.org/RPM-GPG-KEY-elrepo.org
3.安装ELRepo
yum install https://www.elrepo.org/elrepo-release-7.el7.elrepo.noarch.rpm
4.查看ELRepo提供的内核版本
yum --disablerepo="*" --enablerepo="elrepo-kernel" list available
5.安装kernel-lt内核
yum --disablerepo='*' --enablerepo=elrepo-kernel install kernel-lt
6.查看启动器
awk -F\' '$1=="menuentry " {print i++ " : " $2}' /etc/grub2.cfg
7.设置默认启动器为新内核
grub2-set-default 0      # grub2-set-default & grub2-editenv & grub2-mkconfig 测试一下这几个命令

8.生成grub文件
grub2-mkconfig -o /boot/grub2/grub.cfg

9.移除旧版本的内核包 #注意下这里(先忽略)
yum remove kernel-tools-libs.x86_64 kernel-tools.x86_64

8.重启后进行验证
reboot
uname -r
9.检查可更新的rpm包
yum check-update
10.更新软件包
yum update -y 
11.重启机器
``` 

## 环境配置(每台机器)
```
# 关闭防火墙
systemctl stop firewalld
systemctl disable firewalld

# 关闭selinux
# 永久关闭
sed -i 's/enforcing/disabled/' /etc/selinux/config  
# 临时关闭
setenforce 0  

# 关闭swap
# 临时
swapoff -a 
# 永久关闭
sed -ri 's/.*swap.*/#&/' /etc/fstab

# 根据规划设置主机名【master节点上操作】
hostnamectl set-hostname k8smaster
# 根据规划设置主机名【node1节点操作】
hostnamectl set-hostname k8snode1
# 根据规划设置主机名【node2节点操作】
hostnamectl set-hostname k8snode2

# 在master添加hosts
cat >> /etc/hosts << EOF
192.168.177.130 k8smaster
192.168.177.131 k8snode1
192.168.177.132 k8snode2
EOF

# 将桥接的IPv4流量传递到iptables的链
cat > /etc/sysctl.d/k8s.conf << EOF
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1
EOF
# 生效
sysctl --system 
```

## 时间同步配置(每台机器)
```
# ntp的安装配置
# 安装ntp
$ yum -y install ntp

# 启动ntp服务器
$ systemctl start ntpd

# 配置计划任务，使用ntpdate同步时间
# 启动并开机启动计划任务cron
$ systemctl start crond
$ systemctl enable crond

# 配置计划任务，每5分钟同步一次
$ crontab -e
*/5 * * * * /usr/sbin/ntpdate ntp1.aliyun.com
```

## 安装Docker/kubeadm/kubelet(每台机器)

### 安装Docker
首先配置一下Docker的阿里yum源   

```
cat >/etc/yum.repos.d/docker.repo<<EOF
[docker-ce-edge]
name=Docker CE Edge - \$basearch
baseurl=https://mirrors.aliyun.com/docker-ce/linux/centos/7/\$basearch/edge
enabled=1
gpgcheck=1
gpgkey=https://mirrors.aliyun.com/docker-ce/linux/centos/gpg
EOF
```

然后yum方式安装docker   

```
# yum安装
yum -y install docker-ce

# 查看docker版本
docker --version  

# 启动docker
systemctl enable docker
systemctl start docker
```

配置docker的镜像源  
exec-opts参数一定要配置
```
{
  "registry-mirrors": ["https://b9pmyelo.mirror.aliyuncs.com"],
  "exec-opts": ["native.cgroupdriver=systemd"]
}
```
然后重启Docker

```shell
systemctl restart docker
``` 

### 添加kubernetes软件源
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

### 安装kubeadm，kubelet和kubectl
```
# 安装kubelet、kubeadm、kubectl
# 如果需要指定版本, yum install -y kubelet-1.18.0 kubeadm-1.18.0 kubectl-1.18.0
yum install -y kubelet kubeadm kubectl
# 设置开机启动
systemctl enable kubelet
```

## 部署Kubernetes Master(Master节点)
```
kubeadm init --apiserver-advertise-address=10.4.7.151 --image-repository registry.aliyuncs.com/google_containers --kubernetes-version v1.22.3 --service-cidr=10.96.0.0/12  --pod-network-cidr=xx.xx4.0.0/16
```

配置Kubectl工具   
```
mkdir -p $HOME/.kube
cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
chown $(id -u):$(id -g) $HOME/.kube/config
``` 

执行完成后，我们使用下面命令，查看我们正在运行的节点    
```
kubectl get nodes
```
能够看到，目前有一个master节点已经运行了，但是还处于未准备状态
下面我们还需要在Node节点执行其它的命令，将node1和node2加入到我们的master节点上。


## 加入Kubernetes Node(2个node节点)
下面我们需要到 node1 和 node2服务器，执行下面的代码向集群添加新节点
执行在kubeadm init输出的kubeadm join命令：      

>注意，以下的命令是在master初始化完成后，每个人的都不一样！！！需要复制自己生成的   
```
kubeadm join 10.4.7.151:6443 --token igwt4e.6zorqgh5uj8c6as6 \
	--discovery-token-ca-cert-hash sha256:e8e6caf69f74fefd02cfbf3fd551f2ba880b8dc1db8874efb7030f74a425f1e9 
```

默认token有效期为24小时，当过期之后，该token就不可用了。这时就需要重新创建token，操作如下： 
```
kubeadm token create --print-join-command
```
当我们把两个节点都加入进来后，我们就可以去Master节点 执行下面命令查看情况   
```
kubectl get node
```     

## 部署CNI网络插件
安装Flannel网络     
```
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml
``` 

## 集群验证 
注：以下操作只需要在master节点上进行即可
1.检查集群状态  
```
kubectl get nodes
``` 
观察STATUS,如果为Ready时，则说明集群状态正常。  

2.创建Pod验证
```
kubectl create deployment nginx --image=nginx
kubectl expose deployment nginx --port=80 --type=NodePort
kubectl get pod,svc
``` 
执行结果如下，则集群健康        
![kubeadm部署kubernetes01](images/kubeadm部署kubernetes01.jpg)  

>此时 宿主机访问 10.4.7.151:32233   
Welcome to nginx!