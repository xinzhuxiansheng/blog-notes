## Flink on K8s Log 持久化  

1.编写日志 flink-log-pvc.yaml
vim flink-log-pvc.yaml  
```yaml
#Flink 日志 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: flink-log-pvc  # 日志 pvc名称
  namespace: flink
spec:
  storageClassName: nfs-storage   #sc名称，更改为实际的sc名称
  accessModes:
    - ReadWriteMany   #采用ReadWriteMany的访问模式
  resources:
    requests:
      storage: 1Gi    #存储容量，根据实际需要更改
```


2.创建日志pvc
kubectl apply -f flink-log-pvc.yaml 

3.查看pvc   
kubectl get pvc -n flink    

4.编写作业application-deployment-with-log.yaml
vim application-deployment-with-log.yaml
```yaml
# Flink Application集群
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: application-deployment-with-log
spec:
  image: flink:1.13.6
  flinkVersion: v1_13
  imagePullPolicy: IfNotPresent   # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
  serviceAccount: flink
  jobManager:
    replicas: 1
    resource:
      memory: "1024m"
      cpu: 1
  taskManager:
    replicas: 1
    resource:
      memory: "1024m"
      cpu: 1
  podTemplate:
    spec:
      containers:
        - name: flink-main-container
          volumeMounts:
            - name: flink-jar  # 挂载nfs上的jar
              mountPath: /opt/flink/jar
            - name: flink-log  # 挂载log
              mountPath: /opt/flink/log
      volumes:
        - name: flink-jar
          persistentVolumeClaim:
            claimName: flink-jar-pvc
        - name: flink-log
          persistentVolumeClaim:
            claimName: flink-log-pvc
  job:
    jarURI: local:///opt/flink/jar/flink-learn-1.0-SNAPSHOT.jar
    entryClass: com.yzhou.job.StreamWordCount
    args:
    parallelism: 1
    upgradeMode: stateless
```


6、提交作业
kubectl apply -f application-deployment-with-log.yaml   

7、查看作业Pod  
kubectl get all -n flink    

8、网页查看
http://flink.k8s.io:32469/flink/application-deployment-with-log/#/overview  