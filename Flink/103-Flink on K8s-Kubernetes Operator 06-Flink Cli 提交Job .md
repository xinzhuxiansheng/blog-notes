# Flink on Kubernetes - Kubernetes Operator - Flink Cli 提交 Job    

>Operator version: 1.8，Flink version: 1.17         



./bin/flink run-application -t kubernetes-application \
-Dkubernetes.cluster-id=flink-cli-application-demo \
-Dkubernetes.container.image=flink:1.17 \
-Dkubernetes.jobmanager.cpu=1 -Dkubernetes.taskmanager.cpu=1 \
-Djobmanager.memory.process.size=1024m -Dtaskmanager.memory.process.size=1024m \
your-flink-job.jar