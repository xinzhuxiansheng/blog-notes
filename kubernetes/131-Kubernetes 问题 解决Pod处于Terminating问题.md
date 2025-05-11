## 解决删除 Namespace 后一直处于Terminating状态 


```shell
# 示例
kubectl delete pod busybox-pod  -n flink --grace-period=0 --force
```