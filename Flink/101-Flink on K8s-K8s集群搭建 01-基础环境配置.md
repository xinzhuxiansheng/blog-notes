# Flink on Kubernetes - Kubernetes集群搭建 - 基础环境搭建  

## 引言         
为了实战Flink on K8s,我们需要搭建一套K8s集群，所以本篇主要介绍如何在 VMware 搭建 Kubernetes的基础环境搭建。                

## 虚拟机规划  
建议每台虚机配置不低于 `2C 4G` 
| ip      |    role | config  |
| :-------- | --------:| :--: |
| 192.168.0.140  | kubernetes-master01 | hostname: k8s01 |
| 192.168.0.141  | kubernetes-master02 | hostname: k8s02 |
| 192.168.0.142  | kubernetes-node01 | hostname: k8s03   |
| 192.168.0.143  | kubernetes-node02 | hostname: k8s04   |
| 192.168.0.144  | kubernetes-node03 | hostname: k8s05   |
| 192.168.0.145  | kubernetes-node04 | hostname: k8s01   |
| 192.168.0.149  | kubernetes-node04 | 用于做 k8s VIP  |


## VMware Workstation 虚机克隆 & 静态IP 配置   
### 系统安装 & 虚机克隆
安装 Centos7系统 以及克隆等操作，还需自行搜索资料安装，这里就不过多介绍。       

### 静态IP 配置 
编辑centos7的网卡信息
vim /etc/sysconfig/network-scripts/ifcfg-eth0 (具体网卡名称，有的机器是 ifcfg-ens33, 请根据各自系统来定)       

```shell
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
```

博主示例：      
```shell
TYPE=Ethernet
PROXY_METHOD=none
BROWSER_ONLY=no
BOOTPROTO=static
DEFROUTE=yes
IPV4_FAILURE_FATAL=no
IPV6INIT=yes
IPV6_AUTOCONF=yes
IPV6_DEFROUTE=yes
IPV6_FAILURE_FATAL=no
IPV6_ADDR_GEN_MODE=stable-privacy
NAME=ens33
UUID=6a78dd10-84a7-4286-984e-8e760b0facf4
DEVICE=ens33
ONBOOT=yes
IPADDR=192.168.0.140
NETMASK=255.255.255.0
GATEWAY=192.168.0.1
DNS1=114.114.114.114
```

重启网卡服务    
```shell
service network restart
```

操作完后，通过xshell 终端 访问，以及 ping www.baidu.com ，两者都通畅则说明操作没有问题。   

## 关闭防火墙  
```shell
systemctl stop firewalld.service # 停止firewall
systemctl disable firewalld.service # 禁止firewall开机启动
```

## 修改 hostname 
```shell
hostnamectl set-hostname k8s01            # 使用这个命令会立即生效且重启也生效
```

## 配置 /etc/hosts 
```
192.168.0.140 master01.k8s.io k8s01   k8s-master-01
192.168.0.141 master02.k8s.io k8s02   k8s-master-02
192.168.0.142 node01.k8s.io   k8s03   k8s-node-01
192.168.0.143 node02.k8s.io   k8s04   k8s-node-02
192.168.0.144 node03.k8s.io   k8s05   k8s-node-03
192.168.0.145 node04.k8s.io   k8s06   k8s-node-06
192.168.0.149 master.k8s.io   k8s-vip
```

## 关闭 SeLinux
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

## 关闭 Swap 分区 和 关闭大页面
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

## 设置时间同步 

使用 `timedatectl` 命令查看时间和时区   
```shell
[root@k8s01 ~]# timedatectl
      Local time: Sun 2024-03-24 23:17:30 CST
  Universal time: Sun 2024-03-24 15:17:30 UTC
        RTC time: Sun 2024-03-24 15:17:30
       Time zone: Asia/Shanghai (CST, +0800)
     NTP enabled: yes
NTP synchronized: no
 RTC in local TZ: no
      DST active: n/a
```

若时区不对，通过以下方式设置时区
设置时区：  
```shell
# 检查亚洲S开头的时区
[root@localhost ~]# timedatectl list-timezones |  grep  -E "Asia/S.*"

# output:
[root@localhost ~]# timedatectl list-timezones |  grep  -E "Asia/S.*"
Asia/Sakhalin
Asia/Samarkand
Asia/Seoul
Asia/Shanghai
Asia/Singapore
Asia/Srednekolymsk

通过 ` timedatectl set-timezone "Asia/Shanghai" ` 命令将时区设置为 `Asia/Shanghai`   
```

安装 chrony     
```shell
yum install chrony -y 

# 启动chrony服务
systemctl start chronyd

# 设置开机自启
systemctl enable chronyd

# 查看chrony服务状态
systemctl status chronyd
```

其他相关命令：
```shell
# 若需要手动同步系统时钟：    
# 手动同步系统时钟
chronyc -a makestep

# 查看时间同步源
chronyc sources -v
```


>此时，所有虚机环境已经完成以下内容：            
1.关闭 防火墙                   
2.设置 hostname         
3.设置 hosts            
4.关闭 SeLinux          
5.关闭 Swap             
6.开启 时间同步                 
















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