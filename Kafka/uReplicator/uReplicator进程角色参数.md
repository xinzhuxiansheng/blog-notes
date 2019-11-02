**正文**

## 参数

### Manager Configurations
Name 	Default 	Example 	Description
config 			Clusters info file. Should contain all source/destination cluster's zk/broker info. Format is: kafka.cluster.zkStr.cluster1=zk1,zk2/cluster1 kafka.cluster.servers.cluster1=broker1:9092,broker2:9092

srcClusters 			List of clusters that can be source

destClusters 			List of clusters that can be destination

enableRebalance 			If manager finds a new worker in a route when a worker in the route is down for some time

zookeeper 			Helix zookeeper path. Should be same as zookeeper in controller

managerPort 			Port for manager rest endpoint

deployment 			Deployment name, will be used in metrics

env 			env has two parts seperated by '.': first part is the data center and second par is deployment name (i.e., aggregation-dc1), it will be used in metrics.

instanceId 			Manager instance name or ID

controllerPort 			Port for controller rest endpoint

graphiteHost 			Graphite host

graphitePort 			Graphite port

metricsPrefix 			Graphite metrics prefix

workloadRefreshPeriodInSeconds 		300 	Interval to refresh workload

initMaxNumPartitionsPerRoute 		1500 	Max number of partition in a route when adding new topics

maxNumPartitionsPerRoute 		2000 	If topics in a route is expanded and total number of partitions exceeds this number, manager will move the topics with largest partition number to a smaller route

initMaxNumWorkersPerRoute 		10 	Number of workers when creating a route

maxNumWorkersPerRoute 		50 	Max number of workers in a route 