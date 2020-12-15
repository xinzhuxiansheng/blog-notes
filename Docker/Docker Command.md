# Docker 常用命令行

## Docker image相关
```shell
#查看镜像列表
docker images

#拉取镜像
docker pull <image>

#删除镜像
docker rmi <image id>
```

## Docker container相关
```shell
#查看正在运行的container
docker ps

#查看全部container
docker ps -a

#删除container id
docker rm <container id>

#根据<image id> 创建container
docker run -it --privileged=true  <image id> 

#先启动docker container
docker start/stop <container id>

#进入container内（进入centos）
docker exec -it <containerid> /bin/bash 

#参数
-i                                          启动互式
-t                                          进入终端
-d	                                        后台运行容器，并返回容器ID
--privileged=true                           使用root用户登陆
-v D:\docker-data\exchange\:/data           利用本地磁盘D:\docker-data\exchange路径挂载到容器 /data路径

#eg:  
# step01 创建container id
docker run -dit --privileged=true -v D:\docker-data\exchange\:/data <image id> 
# step02 进入container id
docker exec -it <containerid> /bin/bash
```

