
## 查看pod和pvc的绑定关系

refer： https://blog.csdn.net/Brave_heart4pzj/article/details/124568481

### 安装jq
```shell
yum install -y jq
```

### 查询shell
```shell
kubectl get pods --all-namespaces -o=json | jq -c '.items[] | {name: .metadata.name, namespace: .metadata.namespace, claimName:.spec.volumes[] | select( has ("persistentVolumeClaim") ).persistentVolumeClaim.claimName }'
```

### 过滤
根据以上shell命令，添加 grep 过滤某个pod