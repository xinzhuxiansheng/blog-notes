## Harbor搭建文档

>关闭防火墙

1、安装Docker，前面有说明不再叙述

2、安装Docker-compose，要先装Docker，Docker-compose是一个单机上容器管理编排工作最新的稳定版本：
version v2.7.1. 

下载二进制文件【https://github.com/docker/compose/releases】，解压到/usr/local/bin/ 
```shell
curl -L https://github.com/docker/compose/releases/download/v2.7.1/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
```

赋予二进制文件可执行权限
```shell
chmod +x /usr/local/bin/docker-compose
```

测试是否安装成功 
```shell
docker-compose --version3
```


3、安装Harbor最新的稳定版本: version 1.8.1a. 
下载二进制文件【https://github.com/goharbor/harbor/releases/download/v2.7.1/harbor-online-installer-v2.7.1.tgz】 
```shell
wget  https://github.com/goharbor/harbor/releases/download/v2.7.1/harbor-online-installer-v2.7.1.tgz
```

```shell
# 解压到/usr/local/ 
tar zxf harbor-online-installer-v2.7.1.tgz  -C /usr/local/ 

# 修改配置文件
vim harbor/harbor.yml
```

修改配置项：     
#设置hostname     
hostname = IP或域名     
#禁止用户注册     
self_registration = off     
#设置只有管理员可以创建项目     
project_creation_restriction = adminonly     
#配置admin的密码     
harbor_admin_password = 1234564

4、执行安装脚本
```shell
./prepare.sh
./install.sh
```
执行过程比较长，会有很多日志，耐心等待  

5、Harbor启动和停止
```shell
# 启动Harbor
docker-compose start   

# 停止Harbor   
docker-comose stop   

#重启Harbor   
docker-compose restart
```

6、访问测试
浏览器输入IP端口，或域名，可以在本地/etc/hosts配置自定义域名,使用配置文件里的admin帐号密码登录，默认管理帐号名：admin   密码如配置文件里
