# 镜像仓库 Harbor 安装  


## 安装 Docker-compose   
1.下载二进制文件【https://github.com/docker/compose/releases】，解压到/usr/local/bin/     
2.授权
```shell
chmod +x /usr/local/bin/docker-compose
docker-compose --version
```

### 下载Harbor & 配置 
1.下载二进制文件【https://github.com/goharbor/harbor/releases】      
```shell
wget  https://github.com/goharbor/harbor/releases/download/v2.8.2/harbor-online-installer-v2.8.2.tgz
```

2.解压 & 修改配置   
```shell
tar zxf harbor-online-installer-v2.8.2.tgz
```

vim harbor.yml 
```shell
修改配置项：
#设置hostname
hostname = IP或域名
#配置admin的密码
harbor_admin_password = 123456  


# 将 https 禁用，否则需要配置证书  

# https related config
https:
  # https port for harbor, default is 443
  port: 443
  # The path of cert and key files for nginx
  certificate: /your/certificate/path
  private_key: /your/private/key/path
```

### 执行安装脚本
```shell
./prepare.sh
./install.sh

# 执行过程比较长，会有很多日志，耐心等待
```

output log:    
```shell
[+] Running 10/10
 ✔ Network harbor_harbor        Created                                                                             0.1s 
 ✔ Container harbor-log         Started                                                                             1.7s 
 ✔ Container redis              Started                                                                             3.2s 
 ✔ Container harbor-portal      Started                                                                             3.4s 
 ✔ Container registry           Started                                                                             3.4s 
 ✔ Container harbor-db          Started                                                                             2.9s 
 ✔ Container registryctl        Started                                                                             3.6s 
 ✔ Container harbor-core        Started                                                                             4.0s 
 ✔ Container harbor-jobservice  Started                                                                             5.3s 
 ✔ Container nginx              Started                                                                             5.5s 
✔ ----Harbor has been installed and started successfully.----
[root@yzhou harbor]# ls
common  common.sh  docker-compose.yml  harbor.yml  install.sh  LICENSE  prepare
``` 

使用 `docker ps` 命令查看是否一切正确         

此时，浏览器输入 IP:80, username/password: admin/123456。 则 harbor已完成安装。     


## Docker 配置 harbor地址  
修改各docker client配置    
```shell 
vim /etc/docker/daemon.json   
# 添加以下内容
{"insecure-registries": ["http://harbor01.io"]}
```   
配置节点 hosts，将 harbor01.io 地址映射到 harbor ip 上。       

重启 docker 
```shell
systemctl daemon-reload
systemctl restart docker
```

## 其他操作 

### Harbor 命令 启动 & 停止
可以用【docker ps】或【docker-compose ps 】命令查看      
共有8个容器运行
```shell
cd harbor/

启动Harbor
# docker-compose start
停止Harbor
# docker-comose stop
重启Harbor
# docker-compose restart
```
