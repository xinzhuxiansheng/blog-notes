## CentOS7 安装 MySQL5.7

3、卸载之前已安装的MySQL（若有安装）
yum remove mariadb
yum remove mysql-libs

4、安装依赖
yum install libaio

yum -y install autoconf

5、正式安装
rpm -ivh 01_mysql-community-common-5.7.37-1.el7.x86_64.rpm 02_mysql-community-libs-5.7.37-1.el7.x86_64.rpm 03_mysql-community-libs-compat-5.7.37-1.el7.x86_64.rpm 04_mysql-community-client-5.7.37-1.el7.x86_64.rpm 05_mysql-community-server-5.7.37-1.el7.x86_64.rpm

6、启动
systemctl start mysqld

7、查看状态
systemctl status mysqld

8、开机启动
systemctl enable mysqld

9、查看密码
cat /var/log/mysqld.log | grep password

10、修改密码
mysql -uroot -p

set global validate_password_length=4;
set global validate_password_policy=0;
set password=password("123456");

11、设置允许root远程登录
use mysql;
update user set host="%" where user="root";
flush privileges;

12、退出
quit;