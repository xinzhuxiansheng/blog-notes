
## 查看命令空间
```shell
kubectl get namespaces
```

## 创建命名空间
```shell
kubectl create namespace [namespace]
```

## 删除命名空间
```shell
kubectl delete namespace [namespace]
```

## 查看日志
```shell
kubectl logs [podname] -n [namespace]
```


## 查看pods多个资源信息
```shell
kubectl -n [namespace] describe pods [podname]
```


## 扩展查看(ip)
```
 kubectl -n [namespace] get all -o wide
```

## 删除deploymet



## ephemeral-storage 临时存储
