# RockyLinux - Kubernetes 集群搭建 - 搭建 docker registry 镜像仓库 & 可视化工具 registry-browser

## docker-compose.yaml 配置

>需要根据 `openssl rand -hex 64` 生成字符串，将其覆盖到 `SECRET_KEY_BASE` 参数值   

```bash
version: "3.8"
services:
  docker-registry:
    image: registry:2
    container_name: docker-registry
    volumes:
      - ./registry-storage:/var/lib/registry
      - ./config.yml:/etc/docker/registry/config.yml
    environment:
      - TZ=Asia/Shanghai
    ports:
      - 5000:5000
    restart: unless-stopped
    logging:
      options:
        max-size: "10m"
  docker-registry-browser:
    image: klausmeyer/docker-registry-browser:latest
    container_name: docker-registry-browser
    environment:
      - TZ=Asia/Shanghai
      - DOCKER_REGISTRY_URL=http://192.168.0.134:5000/v2
      - ENABLE_DELETE_IMAGES=true
      - SECRET_KEY_BASE=58f0203dca74818c270ba572f113cfc39397b99d3bfa96e3040e7808e76a32c272dfa95c66a6fe2d9c3a6845280184b8bd974ebfdd6a08c0009eb0a2dad6c051
    ports:
      - 8080:8080
    restart: unless-stopped
    logging:
      options:
        max-size: "10m"
```

## config.yml 配置     
```bash
version: 0.1
log:
  fields:
    service: registry
storage:
  cache:
    blobdescriptor: inmemory
  filesystem:
    rootdirectory: /var/lib/registry
  delete:
    enabled: true
http:
  addr: :5000
  headers:
    X-Content-Type-Options: [nosniff]
health:
  storagedriver:
    enabled: true
    interval: 10s
    threshold: 3
```

## 当前 Docker 配置参数   
```shell
vim /etc/docker/daemon.json  

# 添加以下内容 
"insecure-registries":["IP:5000"]
```

## 测试 
```shell
docker pull flink:1.17.2-java11 
docker tag flink:1.17.2-java11 192.168.0.134:5000/flink:1.17.2-java11
docker push 192.168.0.134:5000/flink:1.17.2-java11
```

访问 `192.168.0.134:8080` 浏览 WEB。   

## 配置自动化脚本    
创建 `push_images.sh` 脚本，将上面测试的 shell 命令放在脚本中， 将 `flink:1.17.2-java11` 做成参数，后面只需执行 `./push_images.sh flink:1.17.2-java11` 实现自动化上报。  

**push_images.sh**
```shell
#!/bin/bash

# 检查是否传入了镜像标签
if [ -z "$1" ]; then
  echo "用法: $0 <image_tag>"
  echo "示例: $0 nginx:v1.8"
  exit 1
fi

IMAGE_TAG=$1
REGISTRY=192.168.0.134:5000

# 拉取镜像
docker pull $IMAGE_TAG

# 打标签
docker tag $IMAGE_TAG $REGISTRY/$IMAGE_TAG

# 推送到私有仓库
docker push $REGISTRY/$IMAGE_TAG
```

refer   
1.https://www.cnblogs.com/netcore3/p/16982828.html      
2.https://github.com/klausmeyer/docker-registry-browser      