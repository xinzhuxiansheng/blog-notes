# RockyLinux - Kubernetes 集群搭建 - 部署 Metrics Server   

```yaml
  template:
    metadata:
      labels:
        k8s-app: metrics-server
    spec:
      hostNetwork: true  # 添加该参数
      containers:
      - args:
        - --cert-dir=/tmp
        - --secure-port=4443  # 记得修改端口，因为它与 kubelet 的端口冲突  
        - --kubelet-preferred-address-types=InternalIP,ExternalIP,Hostname
          #- --kubelet-preferred-address-types=InternalIP
        - --kubelet-use-node-status-port
        - --metric-resolution=15s
        - --kubelet-insecure-tls  # 添加该参数  
        image: registry.k8s.io/metrics-server/metrics-server:v0.7.2
        imagePullPolicy: IfNotPresent
        livenessProbe:
          failureThreshold: 3

```