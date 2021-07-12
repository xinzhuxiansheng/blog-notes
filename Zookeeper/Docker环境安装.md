

## 1. 下载ZooKeep镜像
DockerHub的镜像地址：https://hub.docker.com/_/zookeeper?tab=tags&page=1&ordering=last_updated
```shell
docker pull zookeeper
```

## 2. 启动新容器
```java
docker run -d -p 2181:2181 --name zookeeper370 --restart=always <image id>
```

## 3. sh命令进入容器
```java

```
