# Flink on Kubernetes - Kubernetes集群搭建 - 安装 docker & keepalive & haproxy   

>该篇是基于 上一篇 ’Flink on Kubernetes - Kubernetes集群搭建 - 基础环境搭建‘ 完成。   

## 安装 Docker (20.10.7)
Docker安装官方指引  https://docs.docker.com/engine/install/centos/  

>注意：`建议搭建再还没安装 Docker 之前，先了解下官网的文档，建议指定版本安装`。 

### 1.移除以前docker相关包 
```shell
$ sudo yum remove docker \
                  docker-client \
                  docker-client-latest \
                  docker-common \
                  docker-latest \
                  docker-latest-logrotate \
                  docker-logrotate \
                  docker-engine
```

### 2.配置yum源
```shell
$ sudo yum install -y yum-utils

$ sudo yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo 
```

### 3.安装Docker（使用20.10.7版本）
```bash
yum install -y docker-ce-20.10.7 docker-ce-cli-20.10.7  containerd.io-1.4.6
```

### 4.启动Docker
```bash 
systemctl enable docker --now
```

### 5.配置加速，使用163源
```bash
$ mkdir -p /etc/docker

$ vi /etc/docker/daemon.json
{
  "registry-mirrors": ["https://hub-mirror.c.163.com"],
  "exec-opts": ["native.cgroupdriver=systemd"],
  "log-driver": "json-file",
  "log-opts": {
    "max-size": "100m"
  },
  "storage-driver": "overlay2"
}

$ systemctl daemon-reload
$ systemctl restart docker
$ systemctl status docker
```

>此时，所有虚机环境已经完成以下内容：     
1.安装 Docker
2.配置 Docker 参数   

## 安装 Kubernetes  

### 集群规划  
本次搭建的高可用 K8s集群由 2个Master + 4个Worker 组成。Master中 apiserver 是集群的入口，集群中的 三个Master通过 Keepalived 提供一个vip实现高可用，并且通过 Haproxy 来为apiserver 提供反向代理的作用，这样来自 Haproxy的所有请求都将轮询转发到后端的Master节点上。 如果仅仅使用 Keepalived，当集群正常工作时，所有流量还是会到具有 vip的那台Master上，因此加上了Haproxy 能使整个集群的 Master都能参与进来，集群的健壮性更强。    

本次使用Kubernetes版本为`1.23.16`。 Kubernetes1.24及后续版本正式移除对dockershim的支持，但在实际应用中Docker仍是K8s的运行时环境，K8s 1.24要支持Docker需要做额外的安装和配置处理。         

```
192.168.0.140 master01.k8s.io k8s01   k8s-master-01
192.168.0.141 master02.k8s.io k8s02   k8s-master-02
192.168.0.142 node01.k8s.io   k8s03   k8s-node-01
192.168.0.143 node02.k8s.io   k8s04   k8s-node-02
192.168.0.144 node03.k8s.io   k8s05   k8s-node-03
192.168.0.145 node04.k8s.io   k8s06   k8s-node-06
192.168.0.149 master.k8s.io   k8s-vip
```

### 安装前环境准备  

#### 1.开启路由转发 
```bash 
$ vim /etc/sysctl.d/k8s.conf
net.ipv4.ip_forward = 1
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1


$ modprobe br_netfilter

$ sysctl -p /etc/sysctl.d/k8s.conf
net.ipv4.ip_forward = 1
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1
``` 

#### 2.设置资源配置文件 
```bash 
echo "* soft nofile 65536" >> /etc/security/limits.conf
echo "* hard nofile 65536" >> /etc/security/limits.conf
echo "* soft nproc 65536"  >> /etc/security/limits.conf
echo "* hard nproc 65536"  >> /etc/security/limits.conf
echo "* soft  memlock  unlimited"  >> /etc/security/limits.conf
echo "* hard memlock  unlimited"  >> /etc/security/limits.conf
``` 

#### 3.安装其他包 
```bash
yum install -y conntrack-tools libseccomp libtool-ltdl 
```

### 安装 Keepalive（在2台Master操作） 
192.168.0.140 master01.k8s.io k8s01   k8s-master-01
192.168.0.141 master02.k8s.io k8s02   k8s-master-02   

#### 1.安装 keepalive 
```bash 
yum install -y keepalived 
```

#### 2.k8s-master-01（k8s01）配置，输入以下内容  
注意： 系统网卡的名称，

注意此处内容全部替换成以下内容，且注意`VIP的IP`
```bash 
$ vim /etc/keepalived/keepalived.conf # 内容如下： 

! Configuration File for keepalived

global_defs {
   router_id k8s
}

vrrp_script check_haproxy {
    script "killall -0 haproxy"
    interval 3
    weight -2
    fall 10
    rise 2
}

vrrp_instance VI_1 {
    state MASTER
    interface ens33 # 需注意该参数，要与系统网卡名称保持一致
    virtual_router_id 51
    priority 250
    advert_int 1
    authentication {
        auth_type PASS
        auth_pass ceb1b3ec013d66163d6ab
    }
    virtual_ipaddress {
        192.168.0.149
    }
    track_script {
        check_haproxy
    }
}
```

#### 3.k8s-master-02（k8s02）配置，输入以下内容 
注意此处内容全部替换成以下内容，且注意`VIP的IP` 
```bash
$ vim /etc/keepalived/keepalived.conf

! Configuration File for keepalived

global_defs {
   router_id k8s
}

vrrp_script check_haproxy {
    script "killall -0 haproxy"
    interval 3
    weight -2
    fall 10
    rise 2
}

vrrp_instance VI_1 {
    state BACKUP
    interface ens33
    virtual_router_id 51
    priority 200
    advert_int 1
    authentication {
        auth_type PASS
        auth_pass ceb1b3ec013d66163d6ab
    }
    virtual_ipaddress {
        192.168.0.149
    }
    track_script {
        check_haproxy
    }
}
```

#### 4.启动，在2台Master上执行
```bash
# 设置开机启动
$ systemctl enable keepalived.service
# 启动keepalived
$ systemctl start keepalived.service
# 查看启动状态
$ systemctl status keepalived.service
```

output log: 
```
[root@k8s01 ~]# systemctl status keepalived.service  
● keepalived.service - LVS and VRRP High Availability Monitor
   Loaded: loaded (/usr/lib/systemd/system/keepalived.service; enabled; vendor preset: disabled)
   Active: active (running) since Sun 2024-03-24 20:05:32 CST; 7s ago
  Process: 13408 ExecStart=/usr/sbin/keepalived $KEEPALIVED_OPTIONS (code=exited, status=0/SUCCESS)
 Main PID: 13409 (keepalived)
    Tasks: 3
   Memory: 1.4M
   CGroup: /system.slice/keepalived.service
           ├─13409 /usr/sbin/keepalived -D
           ├─13410 /usr/sbin/keepalived -D
           └─13411 /usr/sbin/keepalived -D

Mar 24 20:05:34 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:34 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:34 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:34 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:39 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:39 k8s01 Keepalived_vrrp[13411]: VRRP_Instance(VI_1) Sending/queueing gratuitous ARPs on ens33 for 192.168.0.149
Mar 24 20:05:39 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:39 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:39 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
Mar 24 20:05:39 k8s01 Keepalived_vrrp[13411]: Sending gratuitous ARP on ens33 for 192.168.0.149
[root@k8s01 ~]# 
```

#### 5.启动后查看k8s-master-01的网卡信息，到k8s-master-02也执行相同命令，查看vip在哪台 Master上 
```shell
ip a s ens33 (机器网卡) 
```

对比master-01，master-02 两边输出的结果，若包含 192.168.0.149 ip 则说明 vip落在哪台 Master上。 

master-01 output log:    
```
[root@k8s01 ~]# ip a s ens33
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether 00:50:56:3b:70:10 brd ff:ff:ff:ff:ff:ff
    inet 192.168.0.140/24 brd 192.168.0.255 scope global noprefixroute ens33
       valid_lft forever preferred_lft forever
    inet 192.168.0.149/32 scope global ens33
       valid_lft forever preferred_lft forever
    inet6 fe80::2171:295f:466:b1f0/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
```

master-02 output log:    
```
[root@k8s02 ~]# ip a s ens33
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether 00:50:56:35:75:8c brd ff:ff:ff:ff:ff:ff
    inet 192.168.0.141/24 brd 192.168.0.255 scope global noprefixroute ens33
       valid_lft forever preferred_lft forever
    inet6 fe80::2171:295f:466:b1f0/64 scope link tentative noprefixroute dadfailed 
       valid_lft forever preferred_lft forever
    inet6 fe80::9ef1:bc6b:6bcd:ca0e/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
```

### 安装Haproxy（在2台Master操作）    

#### 1.安装 haproxy 
```shell
yum install -y haproxy  
```
#### 2.k8s-master-01（k8s01）配置，输入以下内容:  
`注意 IP 修改`   
```shell
vim /etc/haproxy/haproxy.cfg
```

```
#---------------------------------------------------------------------
# Global settings
#---------------------------------------------------------------------
global
    # to have these messages end up in /var/log/haproxy.log you will
    # need to:
    # 1) configure syslog to accept network log events.  This is done
    #    by adding the '-r' option to the SYSLOGD_OPTIONS in
    #    /etc/sysconfig/syslog
    # 2) configure local2 events to go to the /var/log/haproxy.log
    #   file. A line like the following can be added to
    #   /etc/sysconfig/syslog
    #
    #    local2.*                       /var/log/haproxy.log
    #
    log         127.0.0.1 local2

    chroot      /var/lib/haproxy
    pidfile     /var/run/haproxy.pid
    maxconn     4000
    user        haproxy
    group       haproxy
    daemon

    # turn on stats unix socket
    stats socket /var/lib/haproxy/stats
#---------------------------------------------------------------------
# common defaults that all the 'listen' and 'backend' sections will
# use if not designated in their block
#---------------------------------------------------------------------
defaults
    mode                    http
    log                     global
    option                  httplog
    option                  dontlognull
    option http-server-close
    option forwardfor       except 127.0.0.0/8
    option                  redispatch
    retries                 3
    timeout http-request    10s
    timeout queue           1m
    timeout connect         10s
    timeout client          1m
    timeout server          1m
    timeout http-keep-alive 10s
    timeout check           10s
    maxconn                 3000
#---------------------------------------------------------------------
# kubernetes apiserver frontend which proxys to the backends
#---------------------------------------------------------------------
frontend kubernetes-apiserver
    mode                 tcp
    bind                 *:16443
    option               tcplog
    default_backend      kubernetes-apiserver
#---------------------------------------------------------------------
# round robin balancing between the various backends
#---------------------------------------------------------------------
backend kubernetes-apiserver
    mode        tcp
    balance     roundrobin
    server      master01.k8s.io   192.168.0.140:6443 check
    server      master02.k8s.io   192.168.0.141:6443 check
#---------------------------------------------------------------------
# collection haproxy statistics message
#---------------------------------------------------------------------
listen stats
    bind                 *:1080
    stats auth           admin:awesomePassword
    stats refresh        5s
    stats realm          HAProxy\ Statistics
    stats uri            /admin?stats
```

#### 3.k8s-master-02（k8s02）配置，输入以下内容:  
```shell
vim /etc/haproxy/haproxy.cfg  
``` 

```
#---------------------------------------------------------------------
# Global settings
#---------------------------------------------------------------------
global
    # to have these messages end up in /var/log/haproxy.log you will
    # need to:
    # 1) configure syslog to accept network log events.  This is done
    #    by adding the '-r' option to the SYSLOGD_OPTIONS in
    #    /etc/sysconfig/syslog
    # 2) configure local2 events to go to the /var/log/haproxy.log
    #   file. A line like the following can be added to
    #   /etc/sysconfig/syslog
    #
    #    local2.*                       /var/log/haproxy.log
    #
    log         127.0.0.1 local2

    chroot      /var/lib/haproxy
    pidfile     /var/run/haproxy.pid
    maxconn     4000
    user        haproxy
    group       haproxy
    daemon

    # turn on stats unix socket
    stats socket /var/lib/haproxy/stats
#---------------------------------------------------------------------
# common defaults that all the 'listen' and 'backend' sections will
# use if not designated in their block
#---------------------------------------------------------------------
defaults
    mode                    http
    log                     global
    option                  httplog
    option                  dontlognull
    option http-server-close
    option forwardfor       except 127.0.0.0/8
    option                  redispatch
    retries                 3
    timeout http-request    10s
    timeout queue           1m
    timeout connect         10s
    timeout client          1m
    timeout server          1m
    timeout http-keep-alive 10s
    timeout check           10s
    maxconn                 3000
#---------------------------------------------------------------------
# kubernetes apiserver frontend which proxys to the backends
#---------------------------------------------------------------------
frontend kubernetes-apiserver
    mode                 tcp
    bind                 *:16443
    option               tcplog
    default_backend      kubernetes-apiserver
#---------------------------------------------------------------------
# round robin balancing between the various backends
#---------------------------------------------------------------------
backend kubernetes-apiserver
    mode        tcp
    balance     roundrobin
    server      master01.k8s.io   192.168.0.140:6443 check
    server      master02.k8s.io   192.168.0.141:6443 check
#---------------------------------------------------------------------
# collection haproxy statistics message
#---------------------------------------------------------------------
listen stats
    bind                 *:1080
    stats auth           admin:awesomePassword
    stats refresh        5s
    stats realm          HAProxy\ Statistics
    stats uri            /admin?stats
``` 

#### 4.启动，在2台Master上执行
```bash
# 设置开机启动
$ systemctl enable haproxy
# 开启haproxy
$ systemctl start haproxy
# 查看启动状态
$ systemctl status haproxy
```

#### 5.启动后检查端口，在2台Master上执行

master-01 output log:    
```bash
[root@k8s01 ~]# netstat -lntup |grep haproxy
tcp        0      0 0.0.0.0:1080            0.0.0.0:*               LISTEN      13480/haproxy       
tcp        0      0 0.0.0.0:16443           0.0.0.0:*               LISTEN      13480/haproxy       
udp        0      0 0.0.0.0:45763           0.0.0.0:*                           13479/haproxy
```

master-02 output log:      
```bash
[root@k8s02 ~]# netstat -lntup |grep haproxy 
tcp        0      0 0.0.0.0:1080            0.0.0.0:*               LISTEN      11008/haproxy       
tcp        0      0 0.0.0.0:16443           0.0.0.0:*               LISTEN      11008/haproxy       
udp        0      0 0.0.0.0:53632           0.0.0.0:*                           11007/haproxy
```


>此时，所有虚机环境已经完成以下内容：            
1.安装 Docker                           

2个Master 节点 安装 keepalive，haproxy                  