## Flink on Kubernetes - Kubernetes集群搭建 - 安装 docker & k8s  

Docker安装官方指引  https://docs.docker.com/engine/install/centos/

1、移除以前docker相关包
$ sudo yum remove docker \
                  docker-client \
                  docker-client-latest \
                  docker-common \
                  docker-latest \
                  docker-latest-logrotate \
                  docker-logrotate \
                  docker-engine

2、配置yum源
$ sudo yum install -y yum-utils

$ sudo yum-config-manager \
--add-repo \
http://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo

3、安装Docker（使用20.10.7版本）
$ yum install -y docker-ce-20.10.7 docker-ce-cli-20.10.7  containerd.io-1.4.6

4、启动Docker
$ systemctl enable docker --now

5、配置加速，使用163源
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

6、Docker常用命令演示

# 查看docker信息
$ docker info

# 查看命令帮助
$ docker cp --help

# 查看docker本地镜像
$ docker images

# 搜索镜像
$ docker search hello-world
$ docker search redis --limit 10

# 拉取镜像
$ docker pull hello-world

# 查看docker统计信息
$ docker system df

# 删除容器镜像
$ docker rmi image_id