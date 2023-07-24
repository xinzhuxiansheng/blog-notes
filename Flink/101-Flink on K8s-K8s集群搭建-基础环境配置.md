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

### 关闭 Swap 分区 和 关闭大页面
```shell
# 设置swap分区、关闭大页面压缩——性能考虑
# Linux中Swap分区（即：交换分区），类似于Windows的虚拟内存，就是当内存不足的时候，把一部分硬盘空间虚拟成# 内存使用，从而解决内存容量不足的情况。
# 在大数据应用中，使用Swap分区会降低性能，通常需要关闭掉。

（1）swap分区设置
swappiness=0：表示最大限度使用物理內存，之后才是swap分区；
swappiness=100：表示积极使用swap分区，並且把內存上的数据及时转移到swap分区；

$ vi /etc/sysctl.conf
vm.swappiness = 0

（2）关闭swap分区
$ vi /etc/fstab 

# 注释掉swap那一行 永久关闭
# /dev/mapper/centos-swap swap                    swap    defaults        0 0

（3）如果启用透明大页面压缩，可能会导致重大性能问题，建议禁用此设置。
$ vi /sys/kernel/mm/transparent_hugepage/defrag
[always] madvise never

$ vi /sys/kernel/mm/transparent_hugepage/enabled
[always] madvise never
```

### 安装时间同步
K8s对集群中各个机器的时间同步要求比较高，要求各个机器的系统时间不能相差太多，不然会造成很多问题。可以配置集群中各个机器和互联网的时间服务器进行时间同步，但是在实际生产环境中，集群中大部分服务器是不能连接外网的，这时候可以在内网搭建一个自己的时间服务器（NTP服务器），集群的各个机器与这个时间服务器进行时间同步。

在我们的集群中，我们选择k8s01作为NTP服务器，其他机器和它自动同步。

（1）安装NTP（所有节点）
$ yum -y install ntp

（2）配置NTP主服务器（k8s01执行）
$ vi /etc/ntp.conf

```
driftfile /var/lib/ntp/drift
restrict 192.168.56.80 mask 255.255.255.0 nomodify notrap
server ntp.aliyun.com
fudge ntp.aliyun.com stratum 10

# Enable public key cryptography.
#crypto

includefile /etc/ntp/crypto/pw

# Key file containing the keys and key identifiers used when operating
# with symmetric key cryptography. 
keys /etc/ntp/keys

# Specify the key identifiers which are trusted.
#trustedkey 4 8 42

# Specify the key identifier to use with the ntpdc utility.
#requestkey 8

# Specify the key identifier to use with the ntpq utility.
#controlkey 8

# Enable writing of statistics records.
#statistics clockstats cryptostats loopstats peerstats

# Disable the monitoring facility to prevent amplification attacks using ntpdc
# monlist command when default restrict does not include the noquery flag. See
# CVE-2013-5211 for more details.
# Note: Monitoring will not be disabled with the limited restriction flag.
disable monitor
```

（3）其他节点配置（k8s02~k8s06执行）
$ vi /etc/ntp.conf

```
driftfile /var/lib/ntp/drift
server k8s01

# Enable public key cryptography.
#crypto

includefile /etc/ntp/crypto/pw

# Key file containing the keys and key identifiers used when operating
# with symmetric key cryptography. 
keys /etc/ntp/keys

# Specify the key identifiers which are trusted.
#trustedkey 4 8 42

# Specify the key identifier to use with the ntpdc utility.
#requestkey 8

# Specify the key identifier to use with the ntpq utility.
#controlkey 8

# Enable writing of statistics records.
#statistics clockstats cryptostats loopstats peerstats

# Disable the monitoring facility to prevent amplification attacks using ntpdc
# monlist command when default restrict does not include the noquery flag. See
# CVE-2013-5211 for more details.
# Note: Monitoring will not be disabled with the limited restriction flag.
disable monitor
```

（4）重新启动 ntp 服务和设置开机自启（所有节点）：
$ service ntpd restart
$ systemctl enable ntpd.service

（5）查看和测试：
$ ntpdc -c loopinfo  #查看与时间同步服务器的时间偏差
$ ntpq -p  #查看当前同步的时间服务器
$ ntpstat  #查看状态定时同步crontab


### 便捷脚本
（1）**xcall.sh，用于在每个节点机器上执行命令**
$ mkdir /opt/bin

$ vi /opt/bin/xcall.sh

```
#! /bin/bash
for i in k8s01 k8s02 k8s03 k8s04 k8s05 k8s06
do
        echo --------- $i ----------
        ssh $i "$*"
done
```

（2）**xsync，用于在每个节点机器上复制文件**
$ vi /opt/bin/xsync

```
#!/bin/bash
#1 获取输入参数个数，如果没有参数，直接退出
pcount=$#
if((pcount==0)); then
echo no args;
exit;
fi

#2 获取文件名称
p1=$1
fname=`basename $p1`
echo fname=$fname

#3 获取上级目录到绝对路径
pdir=`cd -P $(dirname $p1); pwd`
echo pdir=$pdir

#4 获取当前用户名称
user=`whoami`

#5 循环
for((host=1; host<7; host++)); do
        echo ------------------- k8s0$host --------------------
        rsync -av $pdir/$fname $user@k8s0$host:$pdir
done
```

（3）授权   
```
chmod +x *
```

（4）添加到PATH环境变量，方便快捷使用
$ vi /etc/profile

export PATH=$PATH:/opt/bin