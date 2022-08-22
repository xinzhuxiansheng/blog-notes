
## Apisix rpm离线安装.md

### 查看centos版本
```shell

```

### OpenResty 安装
refer: https://openresty.org/cn/linux-packages.html#centos

查看openresty安装版本
```shell
openresty -v
```

若出现以下问题： 

1. https://repo.saltproject.io/py3/redhat/7/x86_64/archive/3004/repodata/repomd.xml: [Errno 14] HTTPS Error 404 - Not Found
```
openresty                                                                                                   | 2.9 kB  00:00:00
https://repo.saltproject.io/py3/redhat/7/x86_64/archive/3004/repodata/repomd.xml: [Errno 14] HTTPS Error 404 - Not Found
Trying other mirror.
To address this issue please refer to the below wiki article
```

解决方法：
refer: https://github.com/openresty/openresty/issues/444
```shell
sudo yum-config-manager --save --setopt=openresty.baseurl=https://openresty.org/package/centos/7/x86_64/
sudo yum install openresty
```

2. GPG key retrieval failed: [Errno 14] HTTPS Error 404 - Not Found

```
warning: /var/cache/yum/x86_64/7/openresty/packages/openresty-1.21.4.1-1.el7.x86_64.rpm: Header V4 RSA/SHA1 Signature, key ID d5edeb74: NOKEY
Retrieving key from https://openresty.org/yum/openresty/openresty/pubkey.gpg


GPG key retrieval failed: [Errno 14] HTTPS Error 404 - Not Found
```

解决方法: 
refer: https://www.cnblogs.com/peterone/p/5241471.html
```
命令后加上  --nogpgcheck  选项
```

### Apisix 安装

```shell
# 下载rpm安装包
安装包下载地址 https://github.com/apache/apisix/releases

# rpm安装
rpm -ivh apisix-2.9-0.el7.x86_64.rpm
```