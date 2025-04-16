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
      - DOCKER_REGISTRY_URL=http://docker-registry:5000/v2
      - ENABLE_DELETE_IMAGES=true
      - SECRET_KEY_BASE=81277d52e70d468e31e08b1191dd7f8d21b6b55373d7eaada4fcb9a65fd6385ec58d3c1d19bd4297390c1bd08354767052702a4b9fd293da735e35879b79b10e
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

## 测试 

```shell
docker pull nginx  

docker tag nginx 192.168.0.134:5000/test:v1

docker push 192.168.0.134:5000/test:v1  
```

访问 `192.168.0.134:8080` 浏览 WEB。   





## 部署 docker registry 
```shell
docker run -d \
  -p 5000:5000 \
  --restart always \
  --name registry-new \
  -v /root/dockerregistry/data:/var/lib/registry \
  registry:2
```

## 部署 docker registry web  
```shell
docker run \
--name registry-browser \
-p 8080:8080 \
--restart=always \ 
--link registry \
-e DOCKER_REGISTRY_URL=http://registry:5000/v2 \
-d klausmeyer/docker-registry-browser  
```

refer   
1.https://www.cnblogs.com/netcore3/p/16982828.html      
2.https://github.com/klausmeyer/docker-registry-browser      