
curl -o /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo
yum install -y yum-utils device-mapper-persistent-data lvm2 wget


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


docker run \
 --name mysql57 \
 -p 3306:3306 \
 -v /opt/mysql/data:/var/lib/mysql \
 -v /opt/mysql/log:/var/log/mysql \
 -v /opt/mysql/my.cnf:/etc/mysql/my.cnf:rw \
 -e MYSQL_ROOT_PASSWORD=password \
 -d mysql:5.7 --default_authentication_plugin=mysql_native_password


#set java environment
JAVA_HOME=/usr/local/java-se-8u42-ri
JRE_HOME=/usr/local/java-se-8u42-ri/jre
PATH=$PATH:$JAVA_HOME/bin:$JRE_HOME/bin
CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar:$JRE_HOME/lib
export JAVA_HOME JRE_HOME PATH CLASSPATH



docker run --name kubeblog  -p 5000:5000  --link mysql57 -e MYSQL_SERVER=mysql75 