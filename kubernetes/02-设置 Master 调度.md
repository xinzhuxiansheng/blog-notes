## 设置 Master 调度 

默认情况下，出于安全原因，群集不会在master节点上调度pod，如果希望能够在master节点上调度pod，例如，对于用于开发的单机Kubernetes集群，请运行以下命令：      

```shell
#master节点默认打了taints
[root@master ~]# kubectl describe nodes | grep Taints
Taints:             node-role.kubernetes.io/master:NoSchedule

#执行以下命令去掉taints污点
[root@master ~]# kubectl taint nodes master node-role.kubernetes.io/master- 
node/master untainted

#再次查看 taint字段为none
[root@master ~]# kubectl describe nodes | grep Taints
Taints:             <none>

#如果要恢复Master Only状态，执行如下命令：
kubectl taint node k8s-master node-role.kubernetes.io/master=:NoSchedule
```