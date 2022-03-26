
## 创建Session集群

```shell
./kubernetes-session.sh \
-Dkubernetes.cluster-id=flink-session01 \
-Dkubernetes.context=ts-flink-prd-cluster@ts-flink-prd \
-Dkubernetes.namespace=yzhou \
-Dkubernetes.jobmanager.service-account=flink \
-Dkubernetes.rest-service.exposed.type=NodePort \
-Dkubernetes.container.image.pull-policy=Always \
-Djobmanager.memory.process.size=2048m \
-Dkubernetes.taskmanager.cpu=4 \
-Dtaskmanager.numberOfTaskSlots=4 \
-Dresourcemanager.taskmanager-timeout=3600000
```


