# Flink on Kubernetes - Kubernetes集群搭建 - 安装 Ingress  

## 安装 Ingress 
Ingress是一个k8s的资源类型，用于实现用域名的方式访问k8s内部应用，它内部运行的是Nginx。简单来说，它是一个代理，可以根据配置转发请求到指定的服务上。 

### 下载 Ingress 安装包 
访问 https://github.com/kubernetes/ingress-nginx, 了解 ingress 与 Kubernetes 版本对应关系，因本人 K8s version 是 1.23.16， 所以下载路径为： https://github.com/kubernetes/ingress-nginx/releases/tag/controller-v1.4.0 。  


>注意 deploy.yaml文件，在 `ingress-nginx-controller-v1.4.0\deploy\static\provider\cloud` 路径下。   

### 替换镜像地址  

```bash 
[root@k8s01 cloud]# cat deploy.yaml |grep image 
        image: registry.k8s.io/ingress-nginx/controller:v1.4.0@sha256:34ee929b111ffc7aa426ffd409af44da48e5a0eea1eb2207994d9e0c0882d143
        imagePullPolicy: IfNotPresent
        image: registry.k8s.io/ingress-nginx/kube-webhook-certgen:v20220916-gd32f8c343@sha256:39c5b2e3310dc4264d638ad28d9d1d96c4cbb2b2dcfb52368fe4e3c63f61e10f
        imagePullPolicy: IfNotPresent
        image: registry.k8s.io/ingress-nginx/kube-webhook-certgen:v20220916-gd32f8c343@sha256:39c5b2e3310dc4264d638ad28d9d1d96c4cbb2b2dcfb52368fe4e3c63f61e10f
        imagePullPolicy: IfNotPresent
[root@k8s01 cloud]# 
``` 
ingress 相关在 http://registry.k8s.io 域名中，由于国内的网络问题，拉取不到该仓库的镜像。 幸好 https://dockerproxy.com/docs提供了镜像代理，按照文档的说明，我们需要把registry.k8s.io地址改为k8s.dockerproxy.com即可。    

```shell
:%s/registry.k8s.io/k8s.dockerproxy.com/g 
```

### 安装 deploy.yaml 
```bash
[root@k8s01 cloud]# kubectl apply -f deploy.yaml 
namespace/ingress-nginx created
serviceaccount/ingress-nginx created
serviceaccount/ingress-nginx-admission created
role.rbac.authorization.k8s.io/ingress-nginx created
role.rbac.authorization.k8s.io/ingress-nginx-admission created
clusterrole.rbac.authorization.k8s.io/ingress-nginx created
clusterrole.rbac.authorization.k8s.io/ingress-nginx-admission created
rolebinding.rbac.authorization.k8s.io/ingress-nginx created
rolebinding.rbac.authorization.k8s.io/ingress-nginx-admission created
clusterrolebinding.rbac.authorization.k8s.io/ingress-nginx created
clusterrolebinding.rbac.authorization.k8s.io/ingress-nginx-admission created
configmap/ingress-nginx-controller created
service/ingress-nginx-controller created
service/ingress-nginx-controller-admission created
deployment.apps/ingress-nginx-controller created
job.batch/ingress-nginx-admission-create created
job.batch/ingress-nginx-admission-patch created
ingressclass.networking.k8s.io/nginx created
validatingwebhookconfiguration.admissionregistration.k8s.io/ingress-nginx-admission created
[root@k8s01 cloud]# 
```

### 查看结果  
```bash
[root@k8s01 cloud]# kubectl get pods -n ingress-nginx -o wide
NAME                                        READY   STATUS      RESTARTS   AGE    IP                NODE    NOMINATED NODE   READINESS GATES
ingress-nginx-admission-create-k5m4h        0/1     Completed   0          9m9s   192.166.235.129   k8s03   <none>           <none>
ingress-nginx-admission-patch-h7npv         0/1     Completed   2          9m9s   192.166.77.1      k8s04   <none>           <none>
ingress-nginx-controller-65c995b75d-rhpmk   1/1     Running     0          9m9s   192.166.89.129    k8s06   <none>           <none>
[root@k8s01 cloud]# 
```


