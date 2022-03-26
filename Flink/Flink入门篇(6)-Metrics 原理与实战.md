
## Metrics原理与实战

### Metric Reporters配置
请参考 https://nightlies.apache.org/flink/flink-docs-release-1.14/docs/deployment/metric_reporters/

在`flink-conf.yaml`添加以下配置:

```
metrics.reporter.promgateway.class: org.apache.flink.metrics.prometheus.PrometheusPushGatewayReporter
metrics.reporter.promgateway.host: xxx-gateway.xxxxx.com
metrics.reporter.promgateway.port: 80
#metrics.reporter.promgateway.jobName: flinkonk8s_test
metrics.reporter.promgateway.randomJobNameSuffix: false
metrics.reporter.promgateway.deleteOnShutdown: false
metrics.reporter.promgateway.groupingKey: _token=xxxxxxxx;_step=60
metrics.reporter.promgateway.interval: 60
```

特别注意：`metrics.reporter.promgateway.jobName`配置项需要在作业启动时添加该参数，添加形式为: 

```
-Dmetrics.reporter.promgateway.jobName=xxxxx



### Metrics自定义
请参考 https://nightlies.apache.org/flink/flink-docs-release-1.14/docs/ops/metrics/

### Connector Metrics介绍
请参考 https://nightlies.apache.org/flink/flink-docs-release-1.14/docs/connectors/datastream/overview/