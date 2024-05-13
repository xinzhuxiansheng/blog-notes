# Flink on Kubernetes - Kubernetes Operator - Akka Cluster 监听 Flink Kubernetes Operator Job Status 实战             

>Operator version: 1.8, Akka version: 2.7.0  

## 引言         
在之前的 Blog 中，介绍过 "Kubernetes Operator Job、Native Kubernetes Job 的部署过程"，不管通过 `kubectl` 、`Java KubernetesClient`、`./flink run-application` 客户端，哪种方式提交 Job时，执行流程都是异步操作，并不会等待 Job 部署最终结果并返回，但在 `./flink run` 客户端提交 Job时（注意，不是 ./flink run-application）会存在 `--detached` 参数来控制 Client 是否开启分离模式。 接下来，我们开始介绍该篇 Blog 主要的内容:`监听 Flink Kubernetes Operator Job Status`。          

## 获取 Flink Kubernetes Operator Job Status                    















refer       
1.
2.https://www.51cto.com/article/777424.html     
