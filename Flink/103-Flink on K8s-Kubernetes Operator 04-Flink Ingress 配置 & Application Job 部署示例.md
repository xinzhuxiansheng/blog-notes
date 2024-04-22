# Flink on Kubernetes - Kubernetes Operator - Flink Ingress 配置 & Application Job 部署示例  

>Operator version: 1.8, Kubernetes version: 1.23.16 ，MetalLB version: v0.10.2

## 介绍 
在上一篇《Flink on K8s - Kubernetes Operator - yaml 创建 Session 集群》 Blog 中介绍 Seesion 集群部署及Job 创建， 在Job 创建过程中，会自动创建 Type 为 ClusterIP的 service，可这个无法在集群外部访问， 所以我们创建 Type 为 NodePort的 service 来访问 Flink Web UI。     

NodePort service 会在所有节点（虚拟机）上开放一个特定端口，任何发送到该端口的流量都被转发到对应的 service。但在企业生产环境中，几乎是不被允许的，机器端口对外开放增加了服务的安全性，`这点 Flink Kubernetes Operator 已经帮我们想到了`, 可访问`https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/ingress/` 了解更多细节。   

![flinkingress02](http://img.xinzhuxiansheng.com/blogimgs/flink/flinkingress02.png)  

Flink Kubernetes Operator 的CR 定义了 Ingress，内容如下： 
```yaml
metadata:
  namespace: default
  name: advanced-ingress
spec:
  image: flink:1.17
  flinkVersion: v1_17
  ingress:
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
```

将 ingress参数添加后，当提交 yaml后，会根据 Job Name 创建 1个 Ingress，同时将 Job Rest Service 与 Ingress 进行绑定。  在后续的访问 Flink Web UI过程中，不需要每次都创建给Job 创建 Type为 NdePort 的 Service。 可通过 `http://flink.k8s.io/{namespace}/{job name}` 访问，示例： flink.k8s.io/flink/basic-application-deployment-only-ingress/ 。   

>接下来，我们通过 部署一个 Application Job来演示。      

## 部署 Application Job   
`vim basic-application-deployment-only-ingress.yaml`    
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-application-deployment-only-ingress
spec:
  image: flink:1.17
  flinkVersion: v1_17
  ingress: 
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
  serviceAccount: flink
  jobManager:
    resource:
      memory: "2048m"
      cpu: 1
  taskManager:
    resource:
      memory: "2048m"
      cpu: 1
  job:
    jarURI: local:///opt/flink/examples/streaming/StateMachineExample.jar
    parallelism: 2
    upgradeMode: stateless
```

```shell
kubectl -n flink apply -f basic-application-deployment-only-ingress.yaml    
```

查看 Flink Job POD：       
```bash 
flink           basic-application-deployment-only-ingress-7cf4c4bf4d-vs7d4  ●  1/1   Running           0 192.166.115
flink           basic-application-deployment-only-ingress-taskmanager-1-1   ●  1/1   Running           0 192.166.235 
``` 

通过`kubectl get svc -n ingress-nginx` 命令，了解到 ingress-nginx-controller 的访问 Type 是 LoadBalancer， 还不能暴露 IP地址。      
```bash
[root@k8s01 job_yaml]# kubectl get svc -n ingress-nginx   
NAME                                 TYPE           CLUSTER-IP     EXTERNAL-IP   PORT(S)                      AGE
ingress-nginx-controller             LoadBalancer   10.96.36.74    <pending>     80:32717/TCP,443:30075/TCP   21d
ingress-nginx-controller-admission   ClusterIP      10.96.244.12   <none>        443/TCP
```

![flinkingress04](http://img.xinzhuxiansheng.com/blogimgs/flink/flinkingress04.png)     

根据梳理的网络结构图， 我们还需对 Kubernetes 环境安装 MetalLB插件。        

## 安装 MetalLB   
本次安装 Metallb, 选择`Layer 2 模式配置`      

>必须注意： 非常建议使用 v0.10.2 版本 

### 使用 yaml 安装  
1.安装 metallb    
```shell
kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.10.2/manifests/namespace.yaml
kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.10.2/manifests/metallb.yaml
```

2.使用 Layer2 模式, 配置IP池          
>需注意 addresses 参数，IP段范围 与 搭建的Kubernetes集群节点的IP段范围保持一致，例如，Kubernetes 集群节点从k8s01-k8s06，而IP是192.168.0.140~192.168.0.145。     
`vim metallb.ip.yaml`   
```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  namespace: metallb-system
  name: config
data:
  config: |
    address-pools:
    - name: default
      protocol: layer2
      addresses:
      - 192.168.0.140-192.168.0.145
```

```shell
kubectl apply -f metallb.ip.yaml      
```

可以查看 Metallb相关 pod 信息     
```bash
[root@k8s01 metallb-v0.10.2]# kubectl get pod -n metallb-system 
NAME                         READY   STATUS    RESTARTS   AGE
controller-f54fbc6f9-bwrn8   1/1     Running   0          52m
speaker-27nch                1/1     Running   0          52m
speaker-2tqbr                1/1     Running   0          52m
speaker-bwp96                1/1     Running   0          52m
speaker-z5xf4                1/1     Running   0          52m
speaker-z78nm                1/1     Running   0          52m
speaker-zwxqp                1/1     Running   0          52m  
```

>访问 https://metallb.universe.tf/installation/ 可自行了解更多的安装步骤。       

3.验证 metallb 是否安装成功   
vim metallb-test.yaml     
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: nginx-metallb-test
spec:
  selector:
    matchLabels:
      app: nginx-metallb-test
  template:
    metadata:
      labels:
        app: nginx-metallb-test
    spec:
      containers:
      - name: nginx
        image: nginx:1.8
        ports:
        - name: http
          containerPort: 80

---
apiVersion: v1
kind: Service
metadata:
  name: nginx-service
spec:
  ports:
  - name: http
    port: 80
    protocol: TCP
    targetPort: 80
  selector:
    app: nginx-metallb-test
  type: LoadBalancer
```

使用`kubectl get svc nginx` ，查看 svc 是否分配`EXTERNAL-IP`      
```bash 
[root@k8s01 metallb-v0.10.2]# kubectl get svc  
NAME            TYPE           CLUSTER-IP    EXTERNAL-IP     PORT(S)        AGE
kubernetes      ClusterIP      10.96.0.1     <none>          443/TCP        22d
nginx-service   LoadBalancer   10.96.45.27   192.168.0.141   80:32026/TCP   2m40s
```
* 此时在虚机中，访问对应的 EXTERNAL-IP        
```shell
[root@k8s01 k8s_yaml]# curl 192.168.165.7  
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
html { color-scheme: light dark; }
body { width: 35em; margin: 0 auto;
font-family: Tahoma, Verdana, Arial, sans-serif; }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
``` 

* 可在宿主机的浏览器访问 192.168.0.141    
![flinkingress01](http://img.xinzhuxiansheng.com/blogimgs/flink/flinkingress01.png)      

表示，metallb 安装 OK。       


## 宿主机访问 Flink Web UI    
1.查看 ingress-nginx-controller 分配的 EXTERNAL-IP       
2.在宿主机中添加`ip flink.k8s.io` 域名映射        
3.访问 `http://flink.k8s.io/flink/basic-application-deployment-only-ingress` 地址，查看 Flink Web UI。        
![flinkingress03](http://img.xinzhuxiansheng.com/blogimgs/flink/flinkingress03.png)      


>通过 Ingress，管控 Flink Job Web UI的访问地址，是非常符合实际生产要素的。    

refer     
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/ingress/          
2.https://metallb.universe.tf/installation/               
3.https://blog.cnscud.com/k8s/2021/09/17/k8s-metalb.html        
