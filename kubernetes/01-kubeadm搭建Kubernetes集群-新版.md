## kubeadm搭建Kubernetes集群    

>本环境基于VMware虚拟环境，

### 1.集群规划    
|   ip  |  域名  | 备注| 安装软件|
|  ----  | ----  |----  |----  |
|  192.168.0.121 | master | 主节点 |Docker Kubeadm kubelet kubectl flannel |
|  192.168.0.122 | node1 |从节点 1 |Docker Kubeadm kubelet kubectl |
|  192.168.0.123 | node2 |从节点 2 |Docker Kubeadm kubelet kubectl|

### 2.基础环境  

>针对所有节点   

- 3台虚拟机CentOS7.x-86_x64 
- 硬件配置：2GB或更多RAM，2个CPU或更多CPU，硬盘30GB或更多   
- 集群中所有机器之间网络互通    
- 可以访问外网，需要拉取镜像    
- 禁止swap分区  

#### 2.1升级系统内核   
Docker要求CentOS系统的内核版本高于3.10，查看本页面的前提条件来验证你的CentOS版本
是否支持Docker，通过uname -r命令查看你当前的内核版本

>CentOS7升级内核，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/02-CentOS7%E5%8D%87%E7%BA%A7%E5%86%85%E6%A0%B8.md`。   

#### 2.2关闭防火墙  

>CentOS7关闭防火墙，请参考`https://github.com/xinzhuxiansheng/blog-notes/blob/master/Linux/05-Centos7%E8%AE%BE%E7%BD%AEfirewall%E9%98%B2%E7%81%AB%E5%A2%99.md`。    

#### 2.3关闭swap    

>CentOS7关闭Swap，请参考``。    





### Docker安装  


### 