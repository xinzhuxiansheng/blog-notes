# Flink 源码 - Native Kubernetes - 探索如何远程调试 Kubernetes 中 TaskManager 补充  

>Flink version: 1.15.4, JDK version: 11, Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8         

## 引言    
在上一篇 "Flink 源码 - Native Kubernetes - 探索如何远程调试 Kubernetes 中 JobManager 和 TaskManager"（https://mp.weixin.qq.com/s/G6J_W1Yuetg-rGEYa5f7ZQ） 文章中介绍了 `在 Kubernetes中如何远程调试 JobManager 和 TaskManager`， 在介绍 TaskManager 部分，我选择了 Jetbrains Gateway 工具使用 SSH 方式对 TaskManager 源码进行调试， 我这样的选择实属无奈。   

当Job 有多个 TaskManager的情况下并不能像 JobManager 那样，给 Service 设置 `Lables Selector` 定位到某个具体的 TaskManager。下图是 TaskManager 的 Labels：  
![remotedebugonk8s06](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s06.jpg)       

我们也知道 TaskManager 是由 JobManager 提交给 Kubernetes 创建的，那基于以上的痛点，我们可否改造一下 JobManager 在构建 TaskManager YAML 时，添加上名为 `podname` 的 Label。  

我们想使用 Service NodePort 方式 解决 TaskManager 远程调试端口暴露问题，如果是你在思考，你觉得还缺什么？    

## TaskManager Pod Name 就像 Kubernetes StatefulSet 应用的 Pod Name 一样  
Flink Job 中 JobManager Pod Name 是包含随机字符串的，如果我们给 Service 的`Selector` 添加 podname 定位到 JobManager，那作业重新部署后， 该 Service 无法复用。   

但 TaskManager Pod Name 就不同于 JobManager Pod Name，它就像 Kubernetes StatefulSet 应用那样， Pod Name 是由 `${servername}-1`,`${servername}-2`... 这样的结构构成的。所以 Job 重新部署时，TaskManager Pod Name 是可以`提前预期`到。   

下图是 Kubernetes 中的 Flink Job Pod Name 示例图：   
![remotedebugonk8s08](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s08.jpg)  

## 改造 Flink 源码，JobManager 构造 TaskManager YAML 时，添加 podname Label 
这部分的实现在 `KubernetesTaskManagerParameters` 类，它包含 `podName` 属性，那我们就在它的 getLabels() 方法中，将 podname 添加到集合中。 如下图：   
![remotedebugonk8s09](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s09.jpg)   

Flink 代码编译好后，替换 pvc 的lib。 再重新通过 Flink Cli 发布作业，此时 TaskManager YAML 中的 Labels 包含了 `podname`. 如下图：  
![remotedebugonk8s10](http://img.xinzhuxiansheng.com/blogimgs/flink/remotedebugonk8s10.jpg)   

## 为某个 TaskManager Pod 创建 Service  
vim  flink-taskmanager-service-remote-debug-01.yaml 
```bash
apiVersion: v1
kind: Service
metadata:
  name: flink-taskmanager-service-remote-debug-01
  namespace: flink
  labels:
    app: flink-application-test
    type: flink-native-kubernetes
    component: service
spec:
  type: NodePort
  selector:
    app: flink-application-test
    component: taskmanager
    type: flink-native-kubernetes
    podname: flink-application-test-taskmanager-1-1   # 此时 selector 多了 podname 匹配 
  ports:
    - name: remote-debug
      port: 5005
      targetPort: 5005
      protocol: TCP
      nodePort: 30004
```

到这里，我新增的TaskManager 补充内容就介绍完了，我不需要使用 Jetbrains Gateway 调试了。  