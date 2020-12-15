**`正文`**

[TOC]


## 设置国内docker镜像地址
参考地址： https://www.daocloud.io/mirror#accelerator-doc
Docker For Windows
在桌面右下角状态栏中右键 docker 图标，修改在 Docker Daemon 标签页中的 json ，把下面的地址:
http://f1361db2.m.daocloud.io
加到" registry-mirrors"的数组里 , 点击 Apply 

## 测试安装
1. 打开终端(命令提示符或PowerShell)
2. 运动 `docker --version` 以确保你拥有受支持的Docker 版本：
    ```shell
    C:\Users\yzhou>docker --version
    Docker version 18.09.2, build 6247962
    ```

## 相关命令
```shell
#拉取镜像
docker pull centos
#创建container id
docker run -dit --privileged=true  <image id> 

#配置 磁盘映射 
docker run -it -v D:\docker-data\exchange\:/data centos


#进入container id内
docker exec -it <containerid> /bin/bash 

#查看正在运行的container
docker ps
#查看全部 container
docker ps -a

#参数含义：
-i 启动互式
-t 进入终端

```
>这个时候安装的centos很干净 需要安装 ifconfig和ssh
```shell

## 先启动docker container
docker start containerid

#安装ifconfig
yum search ifconfig
yum install net-tools.x86_64

#安装ssh服务器
yum list openssh
yum install -y openssh-server
#开始sshd服务
systemctl start sshd

# JDK环境中问题 不要配置在 /etc/profile ， 配置在 /root/.bashrc  （特别注意）

```
## 配置docker内部 与 外部的端口映射
`动态绑定端口`

1.使用 kitematic 工具
 


## 如何拷贝文件到 container中去
```shell
#docker cp 本地文件路径 ID全称:容器路径
docker cp zookeeper-3.4.13.tar.gz centos7_01:/usr/local/
```

## 备份container 成 镜像
```shell
docker commit <containerId|containerName> <imagesName> 
```


## docker打包镜像

```shell
# 注意后面 .  不能忘了
docker build -t="centos-jdk" .
```

docker run -it centos-jdk /bin/bash


## 删除docker镜像
```shell
docker iamges

docker rmi <image id>
```


docker stop $(docker ps -a -q)      停止所有容器

docker rm $(docker ps -a -q)        删除所有容器

docker rmi $(docker images -q)    删除所有镜像



## Q&A
1. 
    Q:
    Error response from daemon: Get https://registry-1.docker.io/v2/library/centos/manifests/latest: unauthorized: incorrect username or password ?

    ```shell
    C:\Users\yzhou>docker pull centos
    Using default tag: latest
    Error response from daemon: Get https://registry-1.docker.io/v2/library/centos/manifests/latest: unauthorized: incorrect username or password
    ```
    A:


2.
    Q:
    Error response from daemon: Get https://registry-1.docker.io/v2/: net/http: TLS handshake timeout



3.
    Q:
    [root@fe3f110e412a /]# systemctl start sshd
    Failed to get D-Bus connection: Operation not permitted

    A:
    参考链接：https://blog.csdn.net/zhenliang8/article/details/78330658
    ```shell
    #创建容器：
    docker run -d --privileged=true centos:7 /usr/sbin/init
    #进入容器：
    docker exec -it centos7 /bin/bash
    #这样可以使用systemctl启动服务了
    ```
4.
    Q:
    service命令不可用

    A:
    参考链接：https://blog.csdn.net/xiaowan206/article/details/79272418
    ```shell
    yum list | grep initscripts
    yum install  initscripts.x86_64

    ```
5.
    Q:
    环境变量配置在 /etc/profile 后，重启机会实效？

    A:
    解决办法是将环境变量设置在：/root/.bashrc


