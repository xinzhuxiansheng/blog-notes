# Kubernetes Operator - 部署 Operator

>该篇是基于 “Kubernetes Operator - 开发 Operator 案例1” 中实战案例代码部署。	

## 编写 Dockerfile 	
```yaml

```
## 生成ingress-manager.yml			
```shell
# 查看 yaml
kubectl create deployment ingress-manager --image yzhou/ingress-manager:1.0.0 --dry-run=client -o yaml 

# 生成 yaml 存储到 ingress-manager.yml
kubectl create deployment ingress-manager --image yzhou/ingress-manager:1.0.0 --dry-run=client -o yaml > manifests/ingress-manager.yml
```

## 打包镜像		
```shell
docker build -t yzhou/ingress-manager:1.0.0 .			
```

## 将镜像导入 kind	
注意 --name 参数，它指向的是 kind的 集群名称， 可通过`kind get clusters` 获取。		
```bash
kind load docker-image yzhou/ingress-manager:1.0.0 --name yzhou-k8s-dev
```

>注意: 此时还不能直接部署 ingress-manager.yml， 因为还未授权，下面是安装后的异常信息：(包含大量的 forbidden 字样的 log异常描述)			
```bash
# 查看 log
kubectl logs ingress-manager-8649f789c9-xmc4t 	

E0404 03:50:18.248716       1 reflector.go:138] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: Failed to watch *v1.Ingress: failed to list *v1.Ingress: ingresses.networking.k8s.io is forbidden: User "system:serviceaccount:default:default" cannot list resource "ingresses" in API group "networking.k8s.io" at the cluster scope
W0404 03:50:29.179797       1 reflector.go:324] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: failed to list *v1.Service: services is forbidden: User "system:serviceaccount:default:default" cannot list resource "services" in API group "" at the cluster scope
E0404 03:50:29.179912       1 reflector.go:138] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: Failed to watch *v1.Service: failed to list *v1.Service: services is forbidden: User "system:serviceaccount:default:default" cannot list resource "services" in API group "" at the cluster scope
W0404 03:51:08.981876       1 reflector.go:324] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: failed to list *v1.Service: services is forbidden: User "system:serviceaccount:default:default" cannot list resource "services" in API group "" at the cluster scope
E0404 03:51:08.982079       1 reflector.go:138] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: Failed to watch *v1.Service: failed to list *v1.Service: services is forbidden: User "system:serviceaccount:default:default" cannot list resource "services" in API group "" at the cluster scope
W0404 03:51:10.567449       1 reflector.go:324] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: failed to list *v1.Ingress: ingresses.networking.k8s.io is forbidden: User "system:serviceaccount:default:default" cannot list resource "ingresses" in API group "networking.k8s.io" at the cluster scope
E0404 03:51:10.567532       1 reflector.go:138] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: Failed to watch *v1.Ingress: failed to list *v1.Ingress: ingresses.networking.k8s.io is forbidden: User "system:serviceaccount:default:default" cannot list resource "ingresses" in API group "networking.k8s.io" at the cluster scope
W0404 03:51:45.865465       1 reflector.go:324] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: failed to list *v1.Service: services is forbidden: User "system:serviceaccount:default:default" cannot list resource "services" in API group "" at the cluster scope
E0404 03:51:45.865530       1 reflector.go:138] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: Failed to watch *v1.Service: failed to list *v1.Service: services is forbidden: User "system:serviceaccount:default:default" cannot list resource "services" in API group "" at the cluster scope
W0404 03:51:59.687673       1 reflector.go:324] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: failed to list *v1.Ingress: ingresses.networking.k8s.io is forbidden: User "system:serviceaccount:default:default" cannot list resource "ingresses" in API group "networking.k8s.io" at the cluster scope
E0404 03:51:59.687805       1 reflector.go:138] pkg/mod/k8s.io/client-go@v0.23.3/tools/cache/reflector.go:167: Failed to watch *v1.Ingress: failed to list *v1.Ingress: ingresses.networking.k8s.io is forbidden: User "system:serviceaccount:default:default" cannot list resource "ingresses" in API group "networking.k8s.io" at the cluster scope    
```


## 创建 ServiceAccount	
```shell
kubectl create serviceaccount  ingress-manager-sa --dry-run=client -o yaml > manifests/ingress-manager-sa.yml     
```

需修改 `ingress-manager.yml`, 修改内容如下：	
```yaml
spec:
  replicas: 1
  selector:
    matchLabels:
      app: ingress-manager
  strategy: {}
  template:
    metadata:
      creationTimestamp: null
      labels:
        app: ingress-manager
    spec:
      serviceAccountName: ingress-manager-sa # 添加 serviceAccountName 
      containers:
      - image: yzhou/ingress-manager:1.0.0
        name: ingress-manager
```

## 创建 Role，分配权限 
```shell
kubectl create role ingress-manager-role --resource=ingress,service --verb list,watch,create,update,delete --dry-run=client -o yaml > manifests/ingress-manager-role.yml      
```	

需注意，ingress-manager-role.yml 对 service 的资源权限定义较宽泛，只需 list，watch 权限即可，所以，还需编辑 `ingress-manager-role.yml`，删除 service资源下多余的权限配置			
```yaml
rules:
- apiGroups:
  - ""
  resources:
  - services
  verbs:
  - list
  - watch
  - create # 删除
  - update # 删除
  - delete # 删除
- apiGroups:
  - networking.k8s.io
  resources:
  - ingresses
  verbs:

```

将 Role 绑定在 ServiceAccount 上：	
```shell
kubectl create rolebinding ingress-manager-rb --role ingress-manager-role --serviceaccount default:ingress-manager-sa --dry-run=client -o yaml > manifests/ingress-manager-role-binding.yml      
```

基于以上操作后，可以看到 manifests/ 目录下的文件结构：		
```shell
➜  manifests git:(main) ✗ tree .
.
├── ingress-manager-role-binding.yml
├── ingress-manager-role.yml
├── ingress-manager-sa.yml
└── ingress-manager.yml
```

## 执行 manifests下的yaml	
```shell
kubectl apply -f manifests 
```	

output log: 	
```bash 
➜  11 git:(main) ✗ kubectl apply -f manifests
rolebinding.rbac.authorization.k8s.io/ingress-manager-rb created
role.rbac.authorization.k8s.io/ingress-manager-role created
serviceaccount/ingress-manager-sa created
deployment.apps/ingress-manager configured   
```		

 **验证**	
 检查 ingress-manager 的yaml 是否符合预期：	
 ```yaml
kubectl get deployment ingress-manager  -o yaml		
```

output log:		
```bash
      restartPolicy: Always
      schedulerName: default-scheduler
      securityContext: {}
      serviceAccount: ingress-manager-sa
      serviceAccountName: ingress-manager-sa
      terminationGracePeriodSeconds: 30
```	

serviceAccount 已生效。		

## 查看 ingress-manager Pod logs 存在异常	
看到 pod logs 存在异常，则说明上面的操作，并没有达到我们预想的期望值，原因是 上面在创建 ServiceAccount,Role 是基于 default namespace 下，但 ingress-manager Operator 并没有限制监听某个命名空间下，所以，我们还需接着上面的操作修改权限。			

### 修改 ingress-manager-role.yml	
修改 kind ，内容如下：	
```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
......
```

### 修改 ingress-manager-role-binding.yml	
修改 kind (需修改2处) , 内容如下：	
```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
......
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: ingress-manager-role
......
```

### 生效	
```yaml
kubectl apply -f manifests 
```

查看日志：	
kubectl logs -f ingress-manager-645fb557dc-s29gx 

## 测试 Operator	
根据上一篇Blog 测试的 ingress 示例，先删除 ingress，确保标记`ingress/http:true`为Serivce，无对应的 ingress，  观察 ingress 是否会自动创建 。		


refer	
1.https://www.bilibili.com/video/BV1hS4y1m7CT/?spm_id_from=333.880.my_history.page.click&vd_source=89b3f4bd088c6355f40a00df74cf8ffd			



