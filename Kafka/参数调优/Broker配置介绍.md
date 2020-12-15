

## Broker Configs
kafka的官网: http://kafka.apache.org

在Broker的参数介绍的表格中 ，有"DYNAMIC UPDATE MODE" title的列：
read-only: 表示只读，修改参数必须重启才能生效
per-broker: 表示当前broker，修改参数针对某个broker生效，不需要重启
cluster-wide: 表示集群范围，修改参数对针对集群的broker节点都生效，不需要重启