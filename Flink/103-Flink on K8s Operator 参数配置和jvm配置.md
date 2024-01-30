## Flink on K8s Operator参数配置和Jvm配置 

1.Flink Kubernetes Operator可配置的参数说明 
https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.6/docs/operations/configuration/    

2.配置Flink Operator参数，修改helm/values.yaml文件，在flink-conf.yaml部分添加或修改配置信息
cd /root/flink-operator/helm    
vim values.yaml     

```
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
```


3.修改Flink Kubernetes Operator JVM启动参数

# ** Set the jvm start up options for webhook and operator
jvmArgs:
  webhook: "-Dlog.file=/opt/flink/log/webhook.log -Xms256m -Xmx256m"
  operator: "-Dlog.file=/opt/flink/log/operator.log -Xms1024m -Xmx1024m"


4.卸载现有的Flink Kubernetes Operator
helm uninstall flink-kubernetes-operator -n flink   

kubectl -n flink delete configmap kube-root-ca.crt; kubectl -n flink delete svc --all; kubectl -n flink delete secret --all; kubectl -n flink delete serviceaccount --all; kubectl -n flink delete role --all; kubectl -n flink delete rolebinding --all;   

5、重新安装Flink Kubernetes Operator    
cd /root/flink-operator/helm
helm install -f values.yaml flink-kubernetes-operator . --namespace flink --create-namespace    

6、登录Dashboard查看Flink Kubernetes Operator启动日志   
https://k8s01:32469/#/login 



