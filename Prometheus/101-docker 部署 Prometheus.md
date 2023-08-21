## Docker 部署 Prometheus   

>CentOS7环境    

### 安装 Docker 
参考 `https://docs.docker.com/engine/install/centos/`   

### Docker 安装 
Volumes & bind-mount

Bind-mount your prometheus.yml from the host by running:
```shell
docker run \
    -p 9090:9090 \
    -v /path/to/prometheus.yml:/etc/prometheus/prometheus.yml \
    prom/prometheus
```

Or bind-mount the directory containing prometheus.yml onto /etc/prometheus by running:
```shell
docker run \
    -p 9090:9090 \
    -v /path/to/config:/etc/prometheus \
    prom/prometheus
``` 

refer 
1.https://prometheus.io/docs/prometheus/latest/installation/