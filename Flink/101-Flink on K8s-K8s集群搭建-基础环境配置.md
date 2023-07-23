## Flink on Kubernetes - Kubernetes集群搭建 - 基础环境搭建  

### 引言    
为了实战Flink on K8s,我们需要搭建一套K8s集群，所以本篇主要介绍虚拟机环境搭建、CentOS7安装以及其他部分组建安装   


### 虚拟机规划 
| ip      |    role | config  |
| :-------- | --------:| :--: |
| 192.168.0.140  | kubernetes-master01 |  2C 4G  |
| 192.168.0.141  | kubernetes-master02 |  2C 4G  |
| 192.168.0.142  | kubernetes-node01 |  2C 4G  |
| 192.168.0.143  | kubernetes-node02 |  2C 4G  |
| 192.168.0.144  | kubernetes-node03 |  2C 4G  |
| 192.168.0.145  | kubernetes-node04 |  2C 4G  |
| 192.168.0.149  | kubernetes-node04 |  2C 4G  |


### 配置 VMware Workstation 虚机静态 IP
```shell
3.1 编辑centos7的网卡信息
vim /etc/sysconfig/network-scripts/ifcfg-eth0 (具体网卡名称 查看 ifconfig,windows机器使用ipconfig/all 全部信息)

编辑ifcfg-xxx 网卡文件：
TYPE="Ethernet"
BOOTPROTO="static" #修改项
DEFROUTE="yes"
PEERDNS="yes"
PEERROUTES="yes"
IPV4_FAILURE_FATAL="no"
IPV6INIT="yes"
IPV6_AUTOCONF="yes"
IPV6_DEFROUTE="yes"
IPV6_PEERDNS="yes"
IPV6_PEERROUTES="yes"
IPV6_FAILURE_FATAL="no"
IPV6_ADDR_GEN_MODE="stable-privacy"
NAME="eth0"
UUID="4cf6c015-f0df-4b9e-ba24-d9ed76dd4848"
DEVICE="eth0"
ONBOOT="yes"    #修改项
IPADDR=IP #添加项
NETMASK="255.255.255.0" #添加项
GATEWAY=网关IP  #添加项
DNS1=网关IP     #添加项

重启网卡服务
service network restart
```


### 修改 hostname


### 关闭防火墙  
```shell
systemctl stop firewalld.service #停止firewall
systemctl disable firewalld.service #禁止firewall开机启动
```

### 配置 /etc/hosts 
```
192.168.0.140 master01.k8s.io k8s01   k8s-master-01
192.168.0.141 master02.k8s.io k8s02   k8s-master-02
192.168.0.142 node01.k8s.io   k8s03   k8s-node-01
192.168.0.143 node02.k8s.io   k8s04   k8s-node-02
192.168.0.144 node03.k8s.io   k8s05   k8s-node-03
192.168.0.145 node04.k8s.io   k8s06   k8s-node-06
192.168.0.149 master.k8s.io   k8s-vip
```

### 关闭 SeLinux
```shell
# 执行 getenforce 指令查看 selinux 状态，如果输出为：enforcing，则需要处理一下，否则可以跳过这一步。  
$ getenforce    

# 修改 `/etc/selinux/config` 文件，将`SELINUX=enforcing` 修改为 SELINUX=disabled。更新配置之后要重启服务器生效，或者执行：`setenforce 0`，使其立即生效。  
$ vi /etc/selinux/config    
SELINUX=disabled    

$ setenforce 0 // 立即生效  
# 再次执行getenforce指令查看selinux状态   
$ getenforce    
```

### 配置免密登录    
k8s01节点 生成 ssh，并且将 k8s01 ssh 分发到 其他节点    
```
生成ssh key：
$ ssh-keygen -t rsa  #（每个节点执行，敲三个回车，就会生成两个文件id_rsa（私钥）、id_rsa.pub（公钥））

k8s01上操作互信配置，将公钥拷贝到要免密登录的目标机器上。
$ ssh-copy-id -i ~/.ssh/id_rsa.pub k8s02
$ ssh-copy-id -i ~/.ssh/id_rsa.pub k8s03
$ ssh-copy-id -i ~/.ssh/id_rsa.pub k8s04
$ ssh-copy-id -i ~/.ssh/id_rsa.pub k8s05
$ ssh-copy-id -i ~/.ssh/id_rsa.pub k8s06
k8s02~k8s06上同上面操作类似，完成互信配置
```