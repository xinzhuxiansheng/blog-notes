## Docker的安装与配置

### 一、Docker的yum安装
1、Docker要求CentOS系统的内核版本高于3.10，查看本页面的前提条件来验证你的CentOS版本
是否支持Docker，通过`uname -r`命令查看你当前的内核版本
```shell
$ uname -r
4.4.227-1.el7.elrepo.x86_64
```
升级内核请参考 Linux目录下的 CentOS7升级内核文档

>备注：3.10的内核版本与Docker和Kubernetes会有兼容性问题，稳定性较差，建议升级到4.4版本

首先配置一下Docker的阿里yum源   

```
cat >/etc/yum.repos.d/docker.repo<<EOF
[docker-ce-edge]
name=Docker CE Edge - \$basearch
baseurl=https://mirrors.aliyun.com/docker-ce/linux/centos/7/\$basearch/edge
enabled=1
gpgcheck=1
gpgkey=https://mirrors.aliyun.com/docker-ce/linux/centos/gpg
EOF
```

> 此方法安装的 Docker 版本比较老 ，可参考 https://blog.csdn.net/skh2015java/article/details/127700161 ， 指定版本安装


然后yum方式安装docker   

```
# yum安装
yum -y install docker-ce

# 查看docker版本
docker --version  

# 启动docker
systemctl enable docker
systemctl start docker
```

配置docker的镜像源  
exec-opts参数一定要配置
```
{
  "exec-opts": ["native.cgroupdriver=systemd"],
  "insecure-registries": ["hub.xxxxx.com"]
}
```
然后重启Docker

```shell
systemctl daemon-reload
systemctl restart docker
``` 

## 配置Docker registries
vim /etc/docker/daemon.json
```
{
  "insecure-registries": ["hub.xxxxx.com"]
}
```

systemctl daemon-reload
systemctl restart docker

## Docker登录 Harbor

docker login hub.xxxxx.com




```shell
curl -o /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo
yum install -y yum-utils device-mapper-persistent-data lvm2 wget

```