## Kubernetes 证书过期  

### 现象  
```shell
[root@kube-xx ~]# kubectl get po
Unable to connect to the server: x509: certificate has expired or is not yet valid: current time 2024-01-12T17:00:52+08:00 is after 2024-01-12T08:08:39Z
[root@kube-xx ~]#
```