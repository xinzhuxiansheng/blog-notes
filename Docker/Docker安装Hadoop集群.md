



命令步骤: 

```shell

#### 安装 centos 开始


#拉取centos7镜像
docker pull centos:7

#查看镜像
docker images

#启动镜像 如果不指定/bin/bash,容器运行后会自动停止
docker run -d -i -t <IMAGE ID> /bin/bash
docker run -d -i -t 7e6257c9f8d8 /bin/bash 

#查看容器ID
docker ps

#进入镜像
docker exec -it <CONTAINER ID> bash
docker exec -it 850b306b9788 bash 

#进入shell命令行后，安装ifconfig
yum install -y net-tools


#### 安装 centos 结束

****************************************************************************************************************

#### 安装 java 开始

#拷贝jdk文件
docker cp D:\docker\yzhou-kafka01\jdk-8u212-linux-x64.tar.gz  850b306b9788:/usr/local/

#安装vim
yum install -y vim

#设置jvm环境变量
vim /etc/profile

#set java environment
JAVA_HOME=/usr/local/jdk1.8.0_212
JRE_HOME=/usr/local/jdk1.8.0_212/jre
PATH=$PATH:$JAVA_HOME/bin:$JRE_HOME/bin
CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar:$JRE_HOME/lib
export JAVA_HOME JRE_HOME PATH CLASSPATH

#生效环境变量
source /etc/profile
#### 安装 java 结束

****************************************************************************************************************

#### 打包镜像 开始

#先退出容器ID中
exit

#制作镜像
docker  commit  -m  "镜像描述"  -a  '制作者'  <CONTAINER ID>  镜像名
docker commit -m "java install" 850b306b9788 centos7:java8

#按照tag删除
 docker rmi centos7:java8

#打tag
docker tag centos7:java8 xinzhuxiansheng/centos:centos7_java8

#### 打包镜像 
docker push xinzhuxiansheng/centos:centos7_java8

文档后面整理：

## Windows 10 将 Docker Desktop for Windows（WSL 2 方式）文件存储移出系统盘放置到其它目录
https://blog.csdn.net/Pointer_v/article/details/106910766?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~all~first_rank_v2~rank_v25-5-106910766.nonecase&utm_term=wsl2%20%E7%9A%84%E7%9B%AE%E5%BD%95


https://github.com/AliyunContainerService/k8s-for-docker-desktop/tree/v1.16.5


token:

eyJhbGciOiJSUzI1NiIsImtpZCI6InVFY3ZoTHNZOHgwaTNvcHNGVE9Jbm5mZkhXTktaWkVkeW5uMVF0MXJlc1kifQ.eyJpc3MiOiJrdWJlcm5ldGVzL3NlcnZpY2VhY2NvdW50Iiwia3ViZXJuZXRlcy5pby9zZXJ2aWNlYWNjb3VudC9uYW1lc3BhY2UiOiJrdWJlLXN5c3RlbSIsImt1YmVybmV0ZXMuaW8vc2VydmljZWFjY291bnQvc2VjcmV0Lm5hbWUiOiJkZWZhdWx0LXRva2VuLXdtYnB4Iiwia3ViZXJuZXRlcy5pby9zZXJ2aWNlYWNjb3VudC9zZXJ2aWNlLWFjY291bnQubmFtZSI6ImRlZmF1bHQiLCJrdWJlcm5ldGVzLmlvL3NlcnZpY2VhY2NvdW50L3NlcnZpY2UtYWNjb3VudC51aWQiOiI5NDZlMjI5MC0zMDZiLTQzNWYtYjJkMC00MWVhOThlYTA3ZWUiLCJzdWIiOiJzeXN0ZW06c2VydmljZWFjY291bnQ6a3ViZS1zeXN0ZW06ZGVmYXVsdCJ9.yZqS1CdbmoVQPnKVNjc5kbBO23OxfWUAznUNG305KuR-b6Jy9jyK2BwIvvcJDalIT22uKLWbrblQCck_JwsAdAV3egkDBA9L59FLuKjXVtkJoF15aFpfPUx-InLnNUAAtr5Qqnl8ckiV8eQF8MopOp5anGUcrDryMAxsdXV8qcJPJBr0RmSuzwolMcpTbpkA4v-okHWmCRkH1i-RVeG1tAKgZj5PqxrKQz3vg9FnD7ffxp6GVg-fiLeOz5NbEZBKAiR9hm09JTv3LZlPxXZd5FJ3fwizzXTXHyC6U5uJrmeTCtFXURRZqTnu7KVFT7aiM9zoEFHuRSt_u2qjgKUV3g