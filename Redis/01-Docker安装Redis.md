## Docker安装Redis

```shell
# 1.查看redis镜像仓库
访问 https://hub.docker.com/_/redis?tab=tags

# 2.获取redis镜像
docker pull redis:latest

# 3.查看本地镜像
docker images

# 4.运行容器
docker run -itd --name redis-test -p 6379:6379 redis

# 5.登录redis pod
docker exec -it redis-test /bin/bash
```

