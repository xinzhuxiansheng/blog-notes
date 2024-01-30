## Flink on K8s Operator高可用部署  

1.查看当前Flink Kubernetes Operator的实例数量     
kubectl get po -n flink 

2.配置Flink Kubernetes Operator高可用，修改replicas大于等于2，修改flink-conf.yaml添加高可用配置     
cd /root/flink-operator/helm    

>重点配置 replicas, 以及部分配置项  
```
    kubernetes.operator.leader-election.enabled: true
    kubernetes.operator.leader-election.lease-name: flink-operator-lease
``` 

vim values.yaml   

```yaml
image:
  repository: ghcr.io/apache/flink-kubernetes-operator
  pullPolicy: IfNotPresent
  tag: "8e60e9b"
  # If image digest is set then it takes precedence and the image tag will be ignored
  # digest: ""

imagePullSecrets: []

# ** Replicas must be 1 unless operator leader election is configured
replicas: 2

defaultConfiguration:
  # If set to true, creates ConfigMaps/VolumeMounts. If set to false, no configuration will be created.
  # All below fields will be ignored if create is set to false.
  create: true
  # If set to true,
  #      (1) loads the built-in default configuration
  #      (2) appends the below flink-conf and logging configuration overrides
  # If set to false, loads just the overrides as in (2).
  # This option has not effect, if create is equal to false.
  append: true
  flink-conf.yaml: |+
    # Flink Config Overrides
    kubernetes.operator.metrics.reporter.slf4j.factory.class: org.apache.flink.metrics.slf4j.Slf4jReporterFactory
    kubernetes.operator.metrics.reporter.slf4j.interval: 5 MINUTE

    kubernetes.operator.flink.client.timeout: 30 s
    kubernetes.operator.reconcile.interval: 60 s
    kubernetes.operator.reconcile.parallelism: 30
    kubernetes.operator.observer.progress-check.interval: 5 s
    kubernetes.operator.leader-election.enabled: true
    kubernetes.operator.leader-election.lease-name: flink-operator-lease
```

3.卸载现有的Flink Kubernetes Operator 
helm uninstall flink-kubernetes-operator -n flink

kubectl -n flink delete configmap kube-root-ca.crt; kubectl -n flink delete svc --all; kubectl -n flink delete secret --all; kubectl -n flink delete serviceaccount --all; kubectl -n flink delete role --all; kubectl -n flink delete rolebinding --all;

4.重新安装Flink Kubernetes Operator 
cd /root/flink-operator/helm
helm install -f values.yaml flink-kubernetes-operator . --namespace flink --create-namespace

5.查看Flink Kubernetes Operator的实例数量 
kubectl get all -n flink

6.登录Dashboard查看Flink Kubernetes Operator启动日志  
https://k8s01:32469/#/login   
