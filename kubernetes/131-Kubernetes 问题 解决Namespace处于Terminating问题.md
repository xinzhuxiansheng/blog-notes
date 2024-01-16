## 解决删除 Namespace 后一直处于Terminating状态 

### 背景    
Namespace删除后，一直显示`Terminating`状态，是因为kubelet 阻塞，有其他的资源在使用该namespace，比如CRD等，尝试重启kubelet，再删除该namespace 也不好使。 
![namespaceterminating01](http://img.xinzhuxiansheng.com/blogimgs/kubernetes/namespaceterminating01.png)    
    
在尝试以下命令强制删除也不好使：    
```shell
kubectl delete ns <terminating-namespace> --force --grace-period=0  
```

### 解决
1. 运行以下命令以查看处于“Terminating”状态的namespace： 
```shell
kubectl get namespaces
``` 

2. 选择一个Terminating namespace，并查看namespace 中的finalizer。运行以下命令： 
```shell
kubectl get namespace <terminating-namespace> -o yaml   
```

得到类似信息如下，重点是：  
```shell
spec:
  finalizers:
  - kubernetes
```

```shell
apiVersion: v1
kind: Namespace
metadata:
  creationTimestamp: "2019-11-20T15:18:06Z"
  deletionTimestamp: "2020-01-16T02:50:02Z"
  name: <terminating-namespace>
  resourceVersion: "3249493"
  selfLink: /api/v1/namespaces/knative-eventing
  uid: f300ea38-c8c2-4653-b432-b66103e412db
spec:
  finalizers:
  - kubernetes
status:
  phase: Terminating
```

3. 导出json格式到tmp.json:  
```shell
kubectl get namespace <terminating-namespace> -o json >tmp.json
```

4. 编辑tmp.josn，删除finalizers 字段的值：   
```
  "spec": {
    "finalizers": []
  },
```

5. 开启 proxy : 
```shell
kubectl proxy   
```

6. 打开新的terminal 窗口，运行以下命令：    
```shell
curl -k -H "Content-Type: application/json" -X PUT --data-binary @tmp.json http://127.0.0.1:8001/api/v1/namespaces/<terminating-namespace>/finalize
```

7. 检查该namespace 是否被删除： 
```shell
kubectl get namespaces
```

完成以上步骤，可删除处于 Terminating状态的 namespace。  


refer   
1.https://blog.csdn.net/ANXIN997483092/article/details/104233494