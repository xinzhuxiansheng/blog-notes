
1. 下载mysql rpm
```shell
wget https://dev.mysql.com/get/mysql80-community-release-el7-6.noarch.rpm
``` 

2. 安装
```shell
yum -y install ysql80-community-release-el7-6.noarch.rpm
```

3. 查看rpm安装效果
```shell
yum repolist enabled | grep mysql.* 
```

4. 安装Mysql
```shell
yum -y install mysql-community-server
```