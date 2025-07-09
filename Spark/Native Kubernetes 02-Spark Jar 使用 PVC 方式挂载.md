# Spark - Native Kubernetes - Spark Jar 使用 PVC 方式挂载   

>spark version: 3.5.5 

## 创建 PVC 
```yaml
#  Spark 作业jar 持久化存储pvc
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: spark355-jar-pvc  # jar pvc名称
  namespace: spark
spec:
  storageClassName: nfs-storage   #sc名称
  accessModes:
    - ReadWriteMany   
  resources:
    requests:
      storage: 1Gi    #存储容量，根据实际需要更改
```

将 jar 通过 busybox + kubectl cp 上传到 pvc 中。 

## 配置参数  
在命令中添加以下参数
```bash
 --conf spark.kubernetes.driver.volumes.persistentVolumeClaim.jars-pvc.mount.path=/opt/spark/appJars \
 --conf spark.kubernetes.driver.volumes.persistentVolumeClaim.jars-pvc.options.claimName=spark355-jar-pvc \
 --conf spark.kubernetes.executor.volumes.persistentVolumeClaim.jars-pvc.mount.path=/opt/spark/appJars \
 --conf spark.kubernetes.executor.volumes.persistentVolumeClaim.jars-pvc.options.claimName=spark355-jar-pvc \
```


`完整示例`  
```bash
/root/yzhou/spark/spark-3.5.5-bin-hadoop3/bin/spark-submit \
 --name yzhoutestjob \
 --verbose \
 --master k8s://https://192.168.0.137:6443 \
 --deploy-mode cluster \
 --conf spark.network.timeout=300 \
 --conf spark.executor.instances=3 \
 --conf spark.driver.cores=1 \
 --conf spark.executor.cores=1 \
 --conf spark.driver.memory=1024m \
 --conf spark.executor.memory=1024m \
 --conf spark.kubernetes.namespace=spark \
 --conf spark.kubernetes.container.image.pullPolicy=IfNotPresent \
 --conf spark.kubernetes.authenticate.driver.serviceAccountName=spark-service-account \
 --conf spark.kubernetes.authenticate.executor.serviceAccountName=spark-service-account \
 --conf spark.driver.extraJavaOptions="-Dio.netty.tryReflectionSetAccessible=true" \
 --conf spark.executor.extraJavaOptions="-Dio.netty.tryReflectionSetAccessible=true" \
 --conf spark.kubernetes.container.image=192.168.0.134:5000/apache/spark:3.5.5 \
 --conf spark.kubernetes.driver.volumes.persistentVolumeClaim.jars-pvc.mount.path=/opt/spark/appJars \
 --conf spark.kubernetes.driver.volumes.persistentVolumeClaim.jars-pvc.options.claimName=spark355-jar-pvc \
 --conf spark.kubernetes.executor.volumes.persistentVolumeClaim.jars-pvc.mount.path=/opt/spark/appJars \
 --conf spark.kubernetes.executor.volumes.persistentVolumeClaim.jars-pvc.options.claimName=spark355-jar-pvc \
 --class com.yzhou.spark355.BasicWordCount \
 local:///opt/spark/appJars/spark355-1.0-SNAPSHOT-jar-with-dependencies.jar \
 file:///opt/spark/appJars/wc_input.txt  
```
