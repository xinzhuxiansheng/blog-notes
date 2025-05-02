

## 配置静态IP
```java
vi /etc/NetworkManager/system-connections/ens160.nmconnection
```

内容如下：  
```bash
[connection]
id=ens160
uuid=fc94e303-d1b5-3dcc-bfdf-1e5226959fa9
type=ethernet
autoconnect-priority=-999
interface-name=ens160

[ethernet]

# 只需修改该项 
[ipv4]   
method=manual
address=192.168.0.202/24,192.168.0.1
dns=192.168.0.1

[ipv6]
addr-gen-mode=eui64
method=auto

[proxy]
```

生效：
```bash
nmcli c reload  
nmcli c up ens160
```

## 修改主机名  
```bash
hostnamectl set-hostname 主机名
```

## 关闭防火墙
```bash
systemctl stop firewalld.service
systemctl disable firewalld.service
systemctl status firewalld.service
```

## 安装 Docker + Docker Compose   
```bash
yum install -y yum-utils

y

# 安装 Docker

yum install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# 安装 Docker-Compose
mv docker-compose-linux-x86_64 docker-compose
chmod +x docker-compose 
mv docker-compose /usr/local/bin/
ln -s /usr/local/bin/docker-compose /usr/bin/docker-compose  
docker-compose version
```