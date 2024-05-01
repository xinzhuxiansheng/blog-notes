# Flink on Kubernetes - Native Kubernetes - Flink Cli 提交 Application Job   

>Flink version: 1.17, Kubernetes version: 1.23.16       

## 引言  
基于上一篇 Blog "Flink on Kubernetes - Native Kubernetes - 基础环境配置"的环境配置， 接下来，我们来探讨 Native Kubernetes Job 部署（Application Mode）          

## 定义 jobmanager-pod 模板  
Native Kubernetes Job 提供了 `-Dkubernetes.pod-template-file.jobmanager` 配置项，可使用它定义一些通用行为，例如，通过添加 `initContainers`，可执行 Pod 主容器启动之前的准备工作，如设置配置文件、下载必要的数据等。           

为了演示 initContainers 阶段，下载 StateMachineExample.jar, 我们需提前将 该jar 上传到 机器上，这里特别标明：StateMachineExample.jar 在 flink 安装包目录下 `examples/streaming/StateMachineExample.jar`;         

![nativek8senv02](images/nativek8senv02.png)   

initContainers 容器使用的是 `busybox:latest`镜像，它的目的是使用最小化工具镜像，提高部署效率， 当然 command 使用 wget 下载 StateMachineExample.jar。那 busybox镜像也需支持该命令，否则无法下载。

>注意：YAML 中的 wget 地址需是 jar真实地址                  

initContainers 与 containers 使用了卷挂载（volumeMounts）来共享数据和状态。这里是具体的匹配方式：   

**共享卷：**        
initContainers 中的 artifacts-fetcher 容器使用 volumeMounts 将一个命名为 flink-artifact 的卷挂载到路径 /flink-artifact 上。该容器负责从远程位置下载一个 JAR 文件到这个挂载点。          

在主 containers 中，同样的 flink-artifact 卷被挂载到路径 /opt/flink/artifacts 上。这意味着主容器可以访问 initContainer 下载的 JAR 文件，因为两者都挂载了同一个卷，但是挂载点路径不同。                  

**操作流程:**               
initContainer 运行，下载文件到 /flink-artifact。        
下载完成后，initContainer 完成，主容器启动。            
主容器通过挂载点 /opt/flink/artifacts 访问到同一个卷，从而可以使用 initContainer 下载的文件。       

所以，后续的 job.jar 的路径是 `local:///opt/flink/artifacts/StateMachineExample.jar`;       

**jobmanager-pod-template.yaml 示例如下**        
```yaml     
apiVersion: v1
kind: Pod
metadata:
  name: jobmanager-pod-template
spec:
  initContainers:
    - name: artifacts-fetcher
      image: busybox:latest
      # Use wget or other tools to get user jars from remote storage
      command: [ 'wget', 'http://192.168.0.201:886/StateMachineExample.jar', '-O', '/flink-artifact/StateMachineExample.jar' ]
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
        - mountPath: /opt/flink/volumes/hostpath
          name: flink-volume-hostpath
        - mountPath: /opt/flink/artifacts
          name: flink-artifact
        - mountPath: /opt/flink/log
          name: flink-logs
      # Use sidecar container to push logs to remote storage or do some other debugging things
    # - name: sidecar-log-collector
    #   image: sidecar-log-collector:latest
    #   command: [ 'command-to-upload', '/remote/path/of/flink-logs/' ]
    #   volumeMounts:
    #     - mountPath: /flink-logs
    #       name: flink-logs
  volumes:
    - name: flink-volume-hostpath
      hostPath:
        path: /tmp
        type: Directory
    - name: flink-artifact
      emptyDir: { }
    - name: flink-logs
      emptyDir: { }
```



## Flink Cli 提交 Job   

### 目录结构  
```bash
[root@k8s01 native]# ll
total 8
-rw-r--r-- 1 root root 1417 May  1 21:49 jobmanager-pod-template.yaml
-rw-r--r-- 1 root root  484 May  1 21:54 submit-application-job.sh
```

### Flink Cli 提交
```shell
vim submit-application-job.sh   

# 内容如下：    
/root/yzhou/flink/flink1172/flink-1.17.2/bin/flink  run-application \
    --target kubernetes-application \
    -Dkubernetes.namespace=flink-native \
    -Dkubernetes.service-account=flink-native-sa \
    -Dkubernetes.cluster-id=flink-application-job01 \
    -Dkubernetes.container.image=flink:1.17 \
    -Dkubernetes.pod-template-file.jobmanager=./jobmanager-pod-template.yaml \
    -Dclassloader.resolve-order=parent-first \
    local:///opt/flink/artifacts/StateMachineExample.jar
```

### 查看 Job 部署情况     
```bash
[root@k8s01 native]# kubectl get all -n flink-native
NAME                                           READY   STATUS    RESTARTS   AGE
pod/flink-application-job01-7dbd847c56-vkk7v   1/1     Running   0          33m
pod/flink-application-job01-taskmanager-1-1    1/1     Running   0          32m

NAME                                   TYPE        CLUSTER-IP     EXTERNAL-IP   PORT(S)             AGE
service/flink-application-job01        ClusterIP   None           <none>        6123/TCP,6124/TCP   33m
service/flink-application-job01-rest   ClusterIP   10.96.72.127   <none>        8081/TCP            33m

NAME                                      READY   UP-TO-DATE   AVAILABLE   AGE
deployment.apps/flink-application-job01   1/1     1            1           33m

NAME                                                 DESIRED   CURRENT   READY   AGE
replicaset.apps/flink-application-job01-7dbd847c56   1         1         1       33m
[root@k8s01 native]#
```

缺少 ingress， 所以目前无法访问 Flink Web UI， 配置 ingress 

### 配置 ingress    
vim flink-application-job01-ingress.yaml  
```yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /$2
  name: flink-application-job01
  namespace: flink-native
spec:
  ingressClassName: nginx
  rules:
  - host: flink.k8s.io
    http:
      paths:
      - backend:
          service:
            name: flink-application-job01-rest # 指向 job的 rest service 
            port:
              number: 8081
        path: /flink-native/flink-application-job01(/|$)(.*) # 路径是 flink-native, 此处与 Operator Job 区分开
        pathType: ImplementationSpecific
```

```shell
kubectl apply -f flink-application-job01-ingress.yaml   
```

## 最后一步         
浏览器访问： http://flink.k8s.io:32717/flink-native/flink-application-job01/ (还得重点提醒大家，URL 一定是以"/"结尾)   

以上就完成 Flink Cli 提交 Application Job 示例。        


>注意： 
目前，我们尝试过 Operator Job 和 Native Kubernetes Job 部署，可以观察下两者区别... 后面我们也会做一些对比介绍。             

