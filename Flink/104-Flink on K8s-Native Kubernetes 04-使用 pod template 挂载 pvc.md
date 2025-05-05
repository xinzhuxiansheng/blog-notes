# Flink on Kubernetes - Native Kubernetes - 使用 POD Template 挂载 PVC  

>Flink version: 1.17, Kubernetes version: 1.30.8       

## jobmanager-pod-template.yaml  
```shell
apiVersion: v1
kind: Pod
metadata:
  name: jobmanager-pod-template
spec:
  initContainers:
    - name: artifacts-fetcher
      image: 192.168.0.134:5000/busybox:1.37.0
      # Use wget or other tools to get user jars from remote storage
      command: [ 'wget', 'http://192.168.0.201:886/flink-jar-1-15-4-1.0-SNAPSHOT.jar', '-O', '/flink-artifact/app.jar' ]
      volumeMounts:
        - mountPath: /flink-artifact
          name: flink-artifact
  containers:
    # Do not change the main container name
    - name: flink-main-container
      resources:
        requests:
          ephemeral-storage: 2048Mi
        limits:
          ephemeral-storage: 2048Mi
      volumeMounts:
        - mountPath: /opt/flink/artifacts
          name: flink-artifact
        # 新增的 volumeMounts
        - mountPath: /opt/flink/lib
          name: flink-15-lib
        - mountPath: /opt/flink/plugins
          name: flink-15-plugins
        - mountPath: /opt/flink/log
          name: flink-15-log
  volumes:
    - name: flink-artifact
      emptyDir: { }
    # 新增的 PVC volumes
    - name: flink-15-lib
      persistentVolumeClaim:
        claimName: flink-15-lib-pvc
    - name: flink-15-plugins
      persistentVolumeClaim:
        claimName: flink-15-plugins-pvc
    - name: flink-15-log
      persistentVolumeClaim:
        claimName: flink-15-log-pvc
``` 

## taskmanager-pod-template.yaml 
```yaml
apiVersion: v1
kind: Pod
metadata:
  name: taskmanager-pod-template
spec:
  initContainers:
    - name: artifacts-fetcher
      image: 192.168.0.134:5000/busybox:1.37.0
      # Use wget or other tools to get user jars from remote storage
      command: [ 'wget', 'http://192.168.0.201:886/flink-jar-1-15-4-1.0-SNAPSHOT.jar', '-O', '/flink-artifact/app.jar' ]
      volumeMounts:
        - mountPath: /flink-artifact
          name: flink-artifact
  containers:
    # Do not change the main container name
    - name: flink-main-container
      resources:
        requests:
          ephemeral-storage: 2048Mi
        limits:
          ephemeral-storage: 2048Mi
      volumeMounts:
        - mountPath: /opt/flink/artifacts
          name: flink-artifact
        # 新增的 volumeMounts
        - mountPath: /opt/flink/lib
          name: flink-15-lib
        - mountPath: /opt/flink/plugins
          name: flink-15-plugins
        - mountPath: /opt/flink/log
          name: flink-15-log
  volumes:
    - name: flink-artifact
      emptyDir: { }
    # 新增的 PVC volumes
    - name: flink-15-lib
      persistentVolumeClaim:
        claimName: flink-15-lib-pvc
    - name: flink-15-plugins
      persistentVolumeClaim:
        claimName: flink-15-plugins-pvc
    - name: flink-15-log
      persistentVolumeClaim:
        claimName: flink-15-log-pvc
```


```shell
/root/yzhou/flink/flink-1.15.4/bin/flink  run-application \
    --target kubernetes-application \
    -Dkubernetes.namespace=flink \
    -Dkubernetes.service-account=flink-service-account \
    -Dkubernetes.cluster-id=flink-application-test \
    -Dkubernetes.pod-template-file.jobmanager=./jobmanager-pod-template.yaml \
    -Dkubernetes.pod-template-file.taskmanager=./taskmanager-pod-template.yaml \
    -Dclassloader.resolve-order=parent-first \
    -Dkubernetes.rest-service.exposed.type=NodePort \
    -c com.yzhou.flink.example.MySQL2Console \
    local:///opt/flink/artifacts/app.jar
```