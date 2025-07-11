# Flink 源码 - Native Kubernetes Session - 部署 Seesion 模式集群 

>Flink version: 1.18.1, Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8         

##  部署 Native Kubernetes Session 

```bash
./kubernetes-session.sh \
-Dkubernetes.cluster-id=flink-native-session-15 \
-Dkubernetes.namespace=flink \
-Dkubernetes.service-account=flink-service-account \
-Dkubernetes.pod-template-file.jobmanager=./pod-template.yaml \
-Dkubernetes.pod-template-file.taskmanager=./pod-template.yaml \
-Dkubernetes.rest-service.exposed.type=Nodeport  
```

创建 flink-native-session-15-lib-pvc.yaml 
```yaml
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-native-session-15-lib-pvc
  namespace: flink
spec:
  storageClassName: nfs-storage  
  accessModes:
    - ReadWriteMany 
  resources:
    requests:
      storage: 1Gi 
```


使用 pod-template.yaml 配置 pvc 
```yaml
---
apiVersion: v1
kind: Pod
spec:
  containers:
    - name: flink-main-container
      volumeMounts:
        - mountPath: /opt/flink/lib
          name: flink-native-session-15-lib-pvc

  volumes:
    - name: flink-native-session-15-lib-pvc
      persistentVolumeClaim:
        claimName: flink-native-session-15-lib-pvc
```


## 删除  
两种删除方式： 
1.直接通过 `kubectl delete deployment/flink-native-seesion-15` 删除即可。         
2.以下脚本  
```bash
$ echo 'stop' | ./bin/kubernetes-session.sh \
    -Dkubernetes.cluster-id=flink-native-seesion-15 \
    -Dexecution.attached=true
```





refer:      
1.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/resource-providers/native_kubernetes/


