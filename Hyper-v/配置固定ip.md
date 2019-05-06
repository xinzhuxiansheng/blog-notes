**`正文`**

[TOC]

## 配置虚拟网卡
### 1. 在Hyper-v Manager 中配置虚拟网络
```shell
1.1 配置Virtual Switch Manager
1.2 新建一个internal virtual switch(内部的虚拟网卡)
1.3 配置External network项，为当前网卡信息
1.4 点击确定
```

### 2. 配置虚机网卡
```shell
2.1 在Hyper-v Manager 页面管理中，选中要设置网卡的虚机，并点击 settting
2.2 配置虚机的Network Adapter为你创建的内部网卡
2.3 确定即可
```

### 3. 配置虚机(centos7)的网卡信息
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
IPADDR="192.168.43.136" #添加项
NETMASK="255.255.255.0" #添加项
GATEWAY="192.168.43.1"  #添加项
DNS1="192.168.43.1"     #添加项

重启网卡服务
service network restart

已经完成
```