# RockyLinux - Kubernetes 集群搭建 - 添加 worker 节点  

## 添加 http 代理  
```shell
export http_proxy=http://192.168.0.2:7897
export https_proxy=http://192.168.0.2:7897
export no_proxy="localhost,127.0.0.1,example.com,*.example.org,192.168.0.*"
``` 

## 关闭防火墙 
```shell
systemctl stop firewalld.service
systemctl disable firewalld.service
```


## 1. 容器运行时 Docker部署（worker节点）
```shell
wget -O /etc/yum.repos.d/docker-ce.repo https://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo   
yum -y install docker-ce    
systemctl enable --now docker    
```

### 1.1 修改 Docker 配置  
```shell
cat <<EOF | sudo tee /etc/docker/daemon.json
{
  "exec-opts": ["native.cgroupdriver=systemd"],
  "registry-mirrors": ["https://docker.1panel.live"]
}
EOF
```   

```shell  
systemctl restart docker  
```  

## 2. cri-dockerd安装    

```shell
wget https://github.com/Mirantis/cri-dockerd/releases/download/v0.3.17/cri-dockerd-0.3.17.amd64.tgz 
tar -zxf cri-dockerd-0.3.17.amd64.tgz 
cp cri-dockerd/cri-dockerd /usr/bin
```

创建服务配置文件 `cri-docker.service` 和 `cri-docker.socket`      
```shell
cat > /usr/lib/systemd/system/cri-docker.service << EOF
[Unit]
Description=CRI Interface for Docker Application Container Engine
Documentation=https://docs.mirantis.com
After=network-online.target firewalld.service docker.service
Wants=network-online.target
Requires=cri-docker.socket

[Service]
Type=notify
ExecStart=/usr/bin/cri-dockerd --container-runtime-endpoint fd://
ExecReload=/bin/kill -s HUP \$MAINPID
TimeoutSec=0
RestartSec=2
Restart=always

# Note that StartLimit* options were moved from "Service" to "Unit" in systemd 229.
# Both the old, and new location are accepted by systemd 229 and up, so using the old location
# to make them work for either version of systemd.
StartLimitBurst=3

# Note that StartLimitInterval was renamed to StartLimitIntervalSec in systemd 230.
# Both the old, and new name are accepted by systemd 230 and up, so using the old name to make
# this option work for either version of systemd.
StartLimitInterval=60s

# Having non-zero Limit*s causes performance problems due to accounting overhead
# in the kernel. We recommend using cgroups to do container-local accounting.
LimitNOFILE=infinity
LimitNPROC=infinity
LimitCORE=infinity

# Comment TasksMax if your systemd version does not support it.
# Only systemd 226 and above support this option.
TasksMax=infinity
Delegate=yes
KillMode=process

[Install]
WantedBy=multi-user.target
EOF
```

```shell
cat > /usr/lib/systemd/system/cri-docker.socket << EOF
[Unit]
Description=CRI Docker Socket for the API
PartOf=cri-docker.service

[Socket]
ListenStream=%t/cri-dockerd.sock
SocketMode=0660
SocketUser=root
SocketGroup=root

[Install]
WantedBy=sockets.target
EOF
```

启动 cri-docker 并设置开机自启  
```shell
systemctl daemon-reload
systemctl enable --now cri-docker.socket
systemctl enable cri-docker.service
systemctl start cri-docker.socket
systemctl start cri-docker.service
systemctl status cri-docker.socket
systemctl status cri-docker.service
```

## 3. 创建kubelet配置文件（worker节点） 
```shell
mkdir -p  /etc/kubernetes/ssl
```

```shell
cat >  /etc/kubernetes/kubelet.json << "EOF"
{
  "kind": "KubeletConfiguration",
  "apiVersion": "kubelet.config.k8s.io/v1beta1",
  "authentication": {
    "x509": {
      "clientCAFile": "/etc/kubernetes/ssl/ca.pem"
    },
    "webhook": {
      "enabled": true,
      "cacheTTL": "2m0s"
    },
    "anonymous": {
      "enabled": false
    }
  },
  "authorization": {
    "mode": "Webhook",
    "webhook": {
      "cacheAuthorizedTTL": "5m0s",
      "cacheUnauthorizedTTL": "30s"
    }
  },
  "address": "192.168.0.148",
  "port": 10250,
  "readOnlyPort": 10255,
  "cgroupDriver": "systemd",                    
  "hairpinMode": "promiscuous-bridge",
  "serializeImagePulls": false,
  "clusterDomain": "cluster.local.",
  "clusterDNS": ["10.96.0.2"]
}
EOF
```

### 创建kubelet服务配置文件（worker节点）  

```shell
cat > /usr/lib/systemd/system/kubelet.service << "EOF"
[Unit]
Description=Kubernetes Kubelet
Documentation=https://github.com/kubernetes/kubernetes
After=docker.service
Requires=docker.service

[Service]
WorkingDirectory=/var/lib/kubelet
ExecStart=/usr/local/bin/kubelet \
  --bootstrap-kubeconfig=/etc/kubernetes/kubelet-bootstrap.kubeconfig \
  --cert-dir=/etc/kubernetes/ssl \
  --kubeconfig=/etc/kubernetes/kubelet.kubeconfig \
  --config=/etc/kubernetes/kubelet.json \
  --rotate-certificates \
  --container-runtime-endpoint=unix:///run/cri-dockerd.sock \
  --pod-infra-container-image=registry.k8s.io/pause:3.9 \
  --v=2
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF
```

关于容器运行时的说明：
如果使用的是containerd，则--container-runtime-endpoint设置为：unix:///run/containerd/containerd.sock

## 4. kube-proxy 部署 

### 创建kube-proxy服务配置文件（worker节点）  
```shell
cat > /etc/kubernetes/kube-proxy.yaml << "EOF"
apiVersion: kubeproxy.config.k8s.io/v1alpha1
bindAddress: 192.168.0.148
clientConnection:
  kubeconfig: /etc/kubernetes/kube-proxy.kubeconfig
clusterCIDR: 10.244.0.0/16
healthzBindAddress: 192.168.0.148:10256
kind: KubeProxyConfiguration
metricsBindAddress: 192.168.0.148:10249
mode: "ipvs"
EOF
```

### 创建kube-proxy服务启动配置文件（worker节点）
```shell
cat >  /usr/lib/systemd/system/kube-proxy.service << "EOF"
[Unit]
Description=Kubernetes Kube-Proxy Server
Documentation=https://github.com/kubernetes/kubernetes
After=network.target

[Service]
WorkingDirectory=/var/lib/kube-proxy
ExecStart=/usr/local/bin/kube-proxy \
  --config=/etc/kubernetes/kube-proxy.yaml \
  --v=2
Restart=on-failure
RestartSec=5
LimitNOFILE=65536

[Install]
WantedBy=multi-user.target
EOF
```

#### 同步kube-proxy文件到worker节点（master01执行） 
```shell
scp kube-proxy*.pem worker01:/etc/kubernetes/ssl/
scp kube-proxy.kubeconfig worker01:/etc/kubernetes/

scp kube-proxy*.pem worker02:/etc/kubernetes/ssl/
scp kube-proxy.kubeconfig worker02:/etc/kubernetes/

scp kube-proxy*.pem worker03:/etc/kubernetes/ssl/
scp kube-proxy.kubeconfig worker03:/etc/kubernetes/

scp kube-proxy*.pem worker04:/etc/kubernetes/ssl/
scp kube-proxy.kubeconfig worker04:/etc/kubernetes/

scp kube-proxy*.pem worker05:/etc/kubernetes/ssl/
scp kube-proxy.kubeconfig worker05:/etc/kubernetes/

scp kube-proxy*.pem worker06:/etc/kubernetes/ssl/
scp kube-proxy.kubeconfig worker06:/etc/kubernetes/
```

#### kube-proxy服务启动  
```shell
mkdir -p /var/lib/kube-proxy
systemctl daemon-reload
systemctl enable --now kube-proxy

systemctl status kube-proxy
```

### 分布CA证书及kubelet-bootstrap.kubeconfig文件  

```shell
scp kubelet-bootstrap.kubeconfig worker06:/etc/kubernetes
scp ca.pem worker06:/etc/kubernetes/ssl
```

```shell
for i in worker01 worker02;do scp kubelet-bootstrap.kubeconfig $i:/etc/kubernetes;done
```

```shell
for i in k8s-worker01 k8s-worker02;do scp ca.pem $i:/etc/kubernetes/ssl;done
```

#### 创建目录及启动kubelet服务  
```shell
mkdir -p /var/lib/kubelet
```

```shell
systemctl daemon-reload
systemctl enable --now kubelet
systemctl status kubelet
```

```shell
[root@master01 bin]# kubectl get nodes
NAME           STATUS     ROLES    AGE     VERSION
worker01   NotReady   <none>   2m35s   v1.28.0
worker02   NotReady   <none>   2m9s    v1.28.0
```
