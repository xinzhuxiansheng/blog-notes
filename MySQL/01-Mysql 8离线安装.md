## Centos7离线安装MySQL8

### 安装

#### 1. 下载MySQL离线安装包
下载地址：https://dev.mysql.com/downloads/mysql/

```
MySQL Community Server 8.0.27

Select Operating System:
    Rad Hat Enterprise Linux/Oracle Linux

Select OS Version:
    Red Hat Enterprise Linux 7/Oracle Linux 7(x86,64-bit)

选择 RPM Bundle （mysql-8.0.27-1.el7.x86_64.rpm-bundle.tar）    
```

#### 2.删除原有的mariadb

```shell
# 先查看是否已经安装了
rpm -qa|grep mariadb

# 若存在，则删除 mariadb
rpm -e --nodeps mariadb-libs
```

#### 3.解压MySQL离线包及手动安装
```shell
# 解压
tar -xvf mysql-8.0.27-1.el7.x86_64.rpm-bundle.tar

# ls
-rw-r--r-- 1 root root  837345280 Dec  1 11:42 mysql-8.0.27-1.el7.x86_64.rpm-bundle.tar
-rw-r--r-- 1 7155 31415  55178328 Sep 29 15:33 mysql-community-client-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415   5932800 Sep 29 15:34 mysql-community-client-plugins-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415    641616 Sep 29 15:34 mysql-community-common-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415   7760100 Sep 29 15:34 mysql-community-devel-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415  23637616 Sep 29 15:34 mysql-community-embedded-compat-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415   4935900 Sep 29 15:34 mysql-community-libs-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415   1264256 Sep 29 15:34 mysql-community-libs-compat-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415 470252428 Sep 29 15:36 mysql-community-server-8.0.27-1.el7.x86_64.rpm
-rw-r--r-- 1 7155 31415 267724484 Sep 29 15:38 mysql-community-test-8.0.27-1.el7.x86_64.rpm
```

**安装顺序**
```shell
rpm -ivh mysql-community-common-8.0.27-1.el7.x86_64.rpm
rpm -ivh mysql-community-client-plugins-8.0.27-1.el7.x86_64.rpm
rpm -ivh mysql-community-libs-8.0.27-1.el7.x86_64.rpm
rpm -ivh mysql-community-client-8.0.27-1.el7.x86_64.rpm
rpm -ivh mysql-community-server-8.0.27-1.el7.x86_64.rpm

# 其他非必须安装，请自行了解
```

#### 4.配置MySQL

**1.初始化数据库**  
```shell
mysqld --initialize --console
``` 

**2.目录授权**  
```shell
chown -R mysql:mysql /var/lib/mysql/
```

**3.启动MySQL服务**
```shell
# 启动
systemctl start mysqld

# 查看
systemctl status mysqld

# 停止
systemctl stop mysqld
``` 

#### 5.数据库操作

**1.查看临时密码**
```shell
cat /var/log/mysqld.log
```

**2.用临时密码登录数据库**
```shell
mysql -u root -p 回车键
然后输入临时密码（输入时不会显示出来，输入完直接回车）
```

**3.修改MySQL密码**
```shell
alter USER 'root'@'localhost' IDENTIFIED BY '123456';
```

>注意MySQL8的安全策略默认比较严格。密码 123456 不通过，先设置较复杂的密码，再通过修改安全策略，再设置简单密码

3.1 例如先设置 root密码为 !QAZ2wsx12
3.2 查看MySQL8的策略：
```shell
SHOW VARIABLES LIKE 'validate_password%';

# 调整安全策略
set global validate_password.policy=0;
set global validate_password.length=1;
```
3.3 再重新执行修改密码命令即可：
```shell
alter USER 'root'@'localhost' IDENTIFIED BY '123456';   
```

**4.设置远程登录**
```shell

# 选择mysql库
show databases;
use mysql;
# 查看root的host
select host, user, authentication_string, plugin from user;

# 修改root的host为%
# 修改
update user set host = "%" where user='root';
# 查看
select host, user, authentication_string, plugin from user;
# 重新加载权限表
flush privileges;

```

**5.针对Native 客户端修改加密规则**
```shell
# 登录
mysql -u root -p（回车后，输入更改后的密码123456）
use mysql;
# mysql8 之前的版本中加密规则是mysql_native_password，而在mysql8之后,加密规则是caching_sha2_password
# 把mysql用户登录密码加密规则还原成mysql_native_password.
alter USER 'root'@'%' IDENTIFIED WITH mysql_native_password BY '123456';
# 重新加载
flush privileges;
``` 

