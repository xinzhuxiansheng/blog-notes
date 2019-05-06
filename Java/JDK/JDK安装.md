**`正文`**
[TOC]

### 1.JDK配置
* Centos卸载系统自带的openjdk
    查看目前系统的jdk：
    ```shell
    rpm -qa|grep jdk
    ```
    卸载命令：
    ```shell
    yum -y remove java-xxx-openjdk-*****
    ```
    也可以使用`yum groupremove java` 命令全部删除(`不过有时不好用`)  
<br/>
* 安装jdk 并且配置环境变量
```shell
vim /etc/profile

#set java environment
JAVA_HOME=/usr/local/jdk7
JRE_HOME=/usr/local/jdk7/jre
PATH=$PATH:$JAVA_HOME/bin:$JRE_HOME/bin
CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar:$JRE_HOME/lib
export JAVA_HOME JRE_HOME PATH CLASSPATH

source profile
```