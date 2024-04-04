## harbor安装

### 1、安装Docker，前面有说明不再叙述   

### 2、安装Docker-compose，要先装Docker，Docker-compose是一个单机上容器管理编排工作
a. 下载二进制文件【https://github.com/docker/compose/releases】，解压到/usr/local/bin/
b. 赋予二进制文件可执行权限
# chmod +x /usr/local/bin/docker-compose
c. 测试是否安装成功
# docker-compose --version

### 3、安装Harbor
a. 下载二进制文件【https://github.com/goharbor/harbor/releases】
 # wget  https://github.com/goharbor/harbor/releases/download/v2.8.2/harbor-online-installer-v2.8.2.tgz

b. 解压到/usr/local/
 # tar zxf harbor-online-installer-v1.8.1.tgz  -C /usr/local/
 # cd /usr/local/harbor/

c. 修改配置文件，配置文件为：/usr/local/harbor/harbor.yml
 # vim /usr/local/harbor/harbor.yml

修改配置项：
     #设置hostname
     hostname = IP或域名
     #禁止用户注册
     self_registration = off
     #设置只有管理员可以创建项目
     project_creation_restriction = adminonly
     #配置admin的密码
     harbor_admin_password = 123456
### 4、执行安装脚本
   # /usr/local/harbor/prepare.sh
   # /usr/local/harbor/install.sh
   执行过程比较长，会有很多日志，耐心等待
### 5、Harbor启动和停止
   上一步完成后，Harbor就启动了，可以用【docker ps】或【docker-compose ps 】命令查看
   共有8个容器运行

   # cd /usr/local/harbor/

   启动Harbor
   # docker-compose start
   停止Harbor
   # docker-comose stop
   重启Harbor
   # docker-compose restart
### 6、访问测试
   浏览器输入IP端口，或域名，可以在本地/etc/hosts配置自定义域名
   使用配置文件里的admin帐号密码登录，默认管理帐号名：admin   密码如配置文件里

### 7、测试上传和下载镜像
   a. 修改各docker client配置
      $ vim /etc/docker/daemon.json
  {"insecure-registries": ["http://harbor01.io"]}

   b. 重启Docker
      # systemctl  restart docker

   c. 登录Harbor
      # docker login harbor.io
      # docker pull harbor.io/library/nginx:1.13
      # docker push harbor.io/library/nginx:1.14
### 8、使用SSL证书加密，作为作业，自己实现
参照例子：
# https related config
https:
#   # https port for harbor, default is 443
   port: 443
#   # The path of cert and key files for nginx
   certificate: /opt/cert/harbor.pem
   private_key: /opt/cert/harbor-key.pem
