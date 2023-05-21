## 编译安装Redis

### 编译安装
```shell
# 1.准备gcc环境
yum install gcc gcc-c++

# 2.下载及解压
wget https://download.redis.io/redis-stable.tar.gz
tar -xzvf redis-stable.tar.gz

# 3.编译安装
make
make install

# 4.启动 Redis
redis-server redis.conf
```

### 参数配置
```shell
# 1.设置已守护进程启动
daemonize yes

```


### Q&A

1. zmalloc.h:50:31: fatal error: jemalloc/jemalloc.h: No such file or directory

**A:** 执行make时，添加MALLOC参数，如下：   
```shell
make MALLOC=libc
```


refer
1.https://redis.io/docs/getting-started/installation/install-redis-from-source/#compiling-redis