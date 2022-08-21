## Docker部署Spring Boot应用

* 简单的spring boot项目
* 搭建Mysql
* 参数配置


### 创建Mysql（无状态）

```shell
# 创建data目录
mkdir /opt/mysql/data

# 创建配置目录
mkdir /opt/mysql/conf.d

# 创建my.cnf
[mysqld]
user=mysql
character-set-server=utf8
default_authentication_plugin=mysql_native_password
secure_file_priv=/var/lib/mysql
expire_logs_days=7
sql_mode=STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION
max_connections=1000
[client]
default-character-set=utf8
[mysql]
default-character-set=utf8
```

```shell
# 根据mysql:5.7镜像，创建 CONTAINER
docker run \
 --name mysql57 \
 -p 3306:3306 \
 -v /opt/mysql/data:/var/lib/mysql \
 -v /opt/mysql/log:/var/log/mysql \
 -v /opt/mysql/my.cnf:/etc/mysql/my.cnf:rw \
 -e MYSQL_ROOT_PASSWORD=password \
 -d mysql:5.7 --default-authentication-plugin=mysql_native_password
```

### 编写Dockerfile
以下是Dockerfile内容
```
FROM openjdk:8-jdk-alpine3.7
MAINTAINER ZhouYang
VOLUME /tmp
ADD target/spring-boot.jar /spring-boot.jar
EXPOSE 5000
ENTRYPOINT ["java","-jar","/spring-boot.jar"]
```

```shell
# 编译
docker build -t xxx:1.0 .
```

> 这时需要注意 spring boot的配置，例如
application-dev.yaml
```
spring:
  datasource:
    driver-class-name: com.mysql.jdbc.Driver
    url: jdbc:mysql://mysql57:3306/blogDB?useUnicode=true&characterEncoding=utf-8
    username: xxxx
    password: xxxx
```

### 启动Spring Boot
在`application-dev.yaml`的mysql57，按照物理机或者虚机需要本地配置hosts。 那对于Docker来说是如何访问mysql?
在Docker中支持 --link参数来配置需要访问的`CONTAINER`
* --link 指定mysql CONTAINER名称
* -d 守护进程

```shell
docker run --name xxxx -p 5000:5000 -d --link mysql57 xxxx:1.0
```
当运行Spring Boot应用之后,登录查看
```shell
# 登录
docker exec -it xxxxx sh  

# 查看evn
```shell
# 输入 env
JAVA_ALPINE_VERSION=8.171.11-r0
HOSTNAME=6f6fe7742858
SHLVL=1
MYSQL57_ENV_MYSQL_MAJOR=5.7
HOME=/root
MYSQL57_PORT_3306_TCP_ADDR=172.17.0.2
MYSQL57_ENV_MYSQL_ROOT_PASSWORD=password
MYSQL57_ENV_GOSU_VERSION=1.14
JAVA_VERSION=8u171
MYSQL57_PORT_3306_TCP_PORT=3306
MYSQL57_PORT_3306_TCP_PROTO=tcp
TERM=xterm
MYSQL57_PORT_33060_TCP_ADDR=172.17.0.2
MYSQL57_PORT=tcp://172.17.0.2:3306
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/lib/jvm/java-1.8-openjdk/jre/bin:/usr/lib/jvm/java-1.8-openjdk/bin
MYSQL57_ENV_MYSQL_VERSION=5.7.39-1.el7
MYSQL57_PORT_33060_TCP_PORT=33060
MYSQL57_PORT_3306_TCP=tcp://172.17.0.2:3306
MYSQL57_PORT_33060_TCP_PROTO=tcp
LANG=C.UTF-8
MYSQL57_NAME=/kubeblog/mysql57
MYSQL57_ENV_MYSQL_SHELL_VERSION=8.0.30-1.el7
MYSQL57_PORT_33060_TCP=tcp://172.17.0.2:33060
PWD=/
JAVA_HOME=/usr/lib/jvm/java-1.8-openjdk
```

查看Spring Boot的CONTAINER的hosts，--link参数会将 CONTAINER为mysql57 自动添加到hosts中，所以程序可以通过mysql57:3306 访问到mysql DB。

```shell
vi /etc/hosts

127.0.0.1       localhost
::1     localhost ip6-localhost ip6-loopback
fe00::0 ip6-localnet
ff00::0 ip6-mcastprefix
ff02::1 ip6-allnodes
ff02::2 ip6-allrouters
172.17.0.2      mysql57 f30aea58aff8
172.17.0.3      6f6fe7742858
```

>另一种 参数配置
利用docker -e 参数
```shell
docker run --name xxxx -p 5000:5000 -d --link mysql57 xxxx:1.0 -e "xxxx" -e "xxxx"
```

在Dockerfile中定义环境变量
```
FROM openjdk:8-jdk-alpine3.7
MAINTAINER ZhouYang
VOLUME /tmp

#设置变量 
ENV username=""
ENV password=""
ENV url=""

ADD target/spring-boot.jar /spring-boot.jar
EXPOSE 5000
ENTRYPOINT ["sh","-c","java -jar xxxx.jar --spring.datasource.username=$username --spring.datasource.url=$url --spring.datasource.password=$password"]
```
