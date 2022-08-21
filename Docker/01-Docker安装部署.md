
### 安装Docker
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


```