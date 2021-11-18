## Flink on Native Kubernetes Metrics监控

> 若没有特殊说明,Flink的默认版本是1.14.0

### 如何将Flink作业的Metrics上报？
参考：https://nightlies.apache.org/flink/flink-docs-release-1.14/zh/docs/deployment/metric_reporters/
这里对官网文档做出一些补充：  
从doc可知，Flink 支持 JMX、Graphite、InfluxDB、Prometheus、PrometheusPushGateway、StatsD、Datadog、Slf4j 方式进行Metrics数据上报，各自配置不同，请详细查看官网文档!。  

博主选择，团队中常用方法`使用PrometheusPushGateway方式`上报, 以下是具体的配置： 
```
metrics.reporter.promgateway.class: org.apache.flink.metrics.prometheus.PrometheusPushGatewayReporter
metrics.reporter.promgateway.host: localhost
metrics.reporter.promgateway.port: 9091
metrics.reporter.promgateway.jobName: myJob
metrics.reporter.promgateway.randomJobNameSuffix: true
metrics.reporter.promgateway.deleteOnShutdown: false
metrics.reporter.promgateway.groupingKey: k1=v1;k2=v2
metrics.reporter.promgateway.interval: 60 SECONDS
```

### 提交测试Flink作业

```
./bin/flink run-application -p 4 -t kubernetes-application \
 -Dkubernetes.namespace=yzhou \
 -Dkubernetes.cluster-id=flink-yzhoutest-cluster02 \
 -Dkubernetes.service-account=flink \
 -Dtaskmanager.memory.process.size=4096m \
 -Dkubernetes.taskmanager.cpu=2 \
 -Dtaskmanager.numberOfTaskSlots=4 \
 -Dkubernetes.container.image=镜像地址\yzhouflinkperf:v1.3.4 \
 local:///opt/flink/usrlib/flink-perf-1.0-SNAPSHOT.jar \
 --bootstrapServers bootstrapServers \
 --topic yzhoutp01 \
 --singleTaskQPS 1000 \
 --recordSize 1024
```



### Grafana配置监控图

