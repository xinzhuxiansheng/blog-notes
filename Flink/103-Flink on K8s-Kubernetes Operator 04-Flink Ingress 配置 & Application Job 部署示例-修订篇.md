# Flink on Kubernetes - Kubernetes Operator - Flink Ingress 配置 & Application Job 部署示例 - 修订篇      

## 引言 
在 上一篇“Flink on Kubernetes - Kubernetes Operator - Flink Ingress 配置 & Application Job 部署示例” Blog 中，介绍了部署 M


```shell
Warning  FailedCreatePodSandBox  48s               kubelet            Failed to create pod sandbox: rpc error: code = U
nknown desc = [failed to set up sandbox container "d384665c69292bad5f9ebd10d0cab13005cc66d6656051ab8313273f90672445" netw
ork for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to set up pod "flink-kubernetes-operato
r-59878dff7-d6cnw_flink" network: error getting ClusterInformation: Get "https://10.96.0.1:443/apis/crd.projectcalico.org
/v1/clusterinformations/default": net/http: TLS handshake timeout, failed to clean up sandbox container "d384665c69292bad
5f9ebd10d0cab13005cc66d6656051ab8313273f90672445" network for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlu
gin cni failed to teardown pod "flink-kubernetes-operator-59878dff7-d6cnw_flink" network: error getting ClusterInformatio
n: Get "https://10.96.0.1:443/apis/crd.projectcalico.org/v1/clusterinformations/default": net/http: TLS handshake timeout
]                                                                                                                        
  Warning  FailedCreatePodSandBox  37s               kubelet            Failed to create pod sandbox: rpc error: code = U
nknown desc = failed to set up sandbox container "bea1875d098ca1455dc3e1c8a0f6d91bbd35023118aaeab7347558a3896c3038" netwo
rk for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to set up pod "flink-kubernetes-operator
-59878dff7-d6cnw_flink" network: error getting ClusterInformation: Get "https://10.96.0.1:443/apis/crd.projectcalico.org/
v1/clusterinformations/default": net/http: TLS handshake timeout                                                         
  Normal   SandboxChanged          3s (x5 over 69s)  kubelet            Pod sandbox changed, it will be killed and re-cre
ated.                                                                                                                    
  Warning  FailedCreatePodSandBox  3s                kubelet            Failed to create pod sandbox: rpc error: code = U
nknown desc = [failed to set up sandbox container "f0025c162c332318263440d433ad6d0e8d17ee587131b8c1e03279f254dfd17b" netw
ork for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to set up pod "flink-kubernetes-operato
r-59878dff7-d6cnw_flink" network: Get "https://10.96.0.1:443/apis/crd.projectcalico.org/v1/ippools?limit=500": net/http: 
TLS handshake timeout, failed to clean up sandbox container "f0025c162c332318263440d433ad6d0e8d17ee587131b8c1e03279f254df
d17b" network for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to teardown pod "flink-kubern
etes-operator-59878dff7-d6cnw_flink" network: error getting ClusterInformation: Get "https://10.96.0.1:443/apis/crd.proje
ctcalico.org/v1/clusterinformations/default": net/http: TLS handshake timeout]
```
