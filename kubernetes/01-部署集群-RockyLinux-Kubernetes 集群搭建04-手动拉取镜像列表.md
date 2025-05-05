# RockyLinux - Kubernetes 集群搭建 - 手动拉取镜像列表   

## Docker 镜像  

`registry.k8s.io/pause:3.10`  
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/pause:3.10
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/pause:3.10 registry.k8s.io/pause:3.10
```

`docker.io/calico/pod2daemon-flexvol:v3.27.2`
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/pod2daemon-flexvol:v3.27.2
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/pod2daemon-flexvol:v3.27.2 docker.io/calico/pod2daemon-flexvol:v3.27.2
```

`docker.io/calico/cni:v3.27.2` 
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/cni:v3.27.2
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/cni:v3.27.2 docker.io/calico/cni:v3.27.2
```  

`registry.k8s.io/ingress-nginx/controller:v1.12.1`  
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/ingress-nginx/controller:v1.12.1  
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/ingress-nginx/controller:v1.12.1 registry.k8s.io/ingress-nginx/controller:v1.12.1
```

`registry.k8s.io/ingress-nginx/kube-webhook-certgen:v1.5.2` 
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/ingress-nginx/kube-webhook-certgen:v1.5.2  
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/ingress-nginx/kube-webhook-certgen:v1.5.2 registry.k8s.io/ingress-nginx/kube-webhook-certgen:v1.5.2
```

`docker.io/calico/typha:v3.27.2`
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/typha:v3.27.2 
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/typha:v3.27.2 docker.io/calico/typha:v3.27.2
```

`docker.io/calico/node:v3.27.2`  
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/node:v3.27.2
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/node:v3.27.2 docker.io/calico/node:v3.27.2
```

`docker.io/calico/node-driver-registrar:v3.27.2` 
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/node-driver-registrar:v3.27.2 
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/node-driver-registrar:v3.27.2 docker.io/calico/node-driver-registrar:v3.27.2
```

`docker.io/calico/csi:v3.27.2` 
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/csi:v3.27.2  
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/docker.io/calico/csi:v3.27.2 docker.io/calico/csi:v3.27.2
```

`registry.k8s.io/metrics-server/metrics-server:v0.7.2` 
```shell
docker pull swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/metrics-server/metrics-server:v0.7.2
docker tag swr.cn-north-4.myhuaweicloud.com/ddn-k8s/registry.k8s.io/metrics-server/metrics-server:v0.7.2 registry.k8s.io/metrics-server/metrics-server:v0.7.2
```


refer 
1.https://docker.aityp.com/   
