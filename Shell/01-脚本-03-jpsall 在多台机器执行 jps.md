# 在多台机器上执行 jps 命令 

## 1.在 /etc/profile.d 目录下配置环境变量  
vim /etc/profile.d/my_env.sh  

配置 jdk 环境变量，内容如下： 
```bash
# java env
JAVA_HOME=/root/jdk1.8.0_202
JRE_HOME=/root/jdk1.8.0_202/jre
PATH=$PATH:$JAVA_HOME/bin:$JRE_HOME/bin
CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar:$JRE_HOME/lib
export JAVA_HOME JRE_HOME PATH CLASSPATH
```

source /etc/profile 


## 2.创建 jpsall.sh 脚本 
vim jpsall.sh 
```bash
#!/bin/bash
for host in bigdata01 bigdata02 bigdata03
do
        echo =============== $host ===============
        ssh $host jps $@ | grep -v Jps
done
``` 

chmod +x jpsall.sh  

运行结果如下：  
```bash
[root@bigdata01 shell]# ./jpsall
=============== bigdata01 ===============
=============== bigdata02 ===============
=============== bigdata03 ===============
```