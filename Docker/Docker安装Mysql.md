# Docker安装MySQL9

### 下载MySQL9
```
docker pull mysql:9.3.0
```
下载完毕，检查镜像：
```
docker images
```

### 运行MySQL9

创建目录
```
mkdir /Volumes/mysql9.3.0/log -p
mkdir /Volumes/mysql9.3.0/data -p
mkdir /Volumes/mysql9.3.0/conf -p
mkdir /Volumes/mysql9.3.0/mysql-files -p
```

使用如下命令来启动mysql容器：
```
docker run -p 5506:3306 --name mysql9-imooc \
-v /Volumes/mysql9.3.0/log:/var/log/mysql \
-v /Volumes/mysql9.3.0/data:/var/lib/mysql \
-v /Volumes/mysql9.3.0/conf:/etc/mysql/conf.d \
-v /Volumes/mysql9.3.0/mysql-files:/var/lib/mysql-files \
-e MYSQL_ROOT_PASSWORD=root \
-d mysql:9.3.0 \
--character-set-server=utf8mb4 --collation-server=utf8mb4_unicode_ci
```

其他命令：
```
docker start mysql
docker stop mysql
docker restart mysql
```