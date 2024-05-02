# Flink on Kubernetes - Kubernetes Operator - 监听 Flink Job Status          

>Operator version: 1.8  

## 引言         
在之前的 Blog 介绍过 Operator Job、Native Kubernetes Job 的部署过程，不管通过 `kubectl` 、`Java KubernetesClient`、`./flink run-application` 客户端提交 Job时，以上操作都是异步模式（并不会等待Job 部署结果），但在 `./flink run` 客户端提交 Job时（注意，不是 ./flink run-application）会存在 `--detached` 参数来控制 Client 开启分离模式（不等待返回结果）。                 








refer       
1.
2.https://www.51cto.com/article/777424.html     
