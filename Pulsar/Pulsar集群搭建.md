

>提示
除了最极端的情况，对于绝大多数的用例来说，单机群安装的Pulsar就能够满足要求了。 如果是创业公司或单个团队想体验下Pulsar，我们推荐使用单集群。 如果你需要使用多集群的 Pulsar 实例 请看这个指南here。

如果要在部署的 Pulsar 集群中使用所有内置的 Pulsar IO连接器。你必须先下载apache-pulsar-io-connectors安装包，然后将apache-pulsar-io-connectors安装到每台 broker 中 Pulsar 安装路径的connectors目录下。如果需要用独立的 function worker 运行Pulsar Functions，则也需要将包安装在 worker 节点的对应目录。

如果要在部署的集群使用分层存储特性，你必须先下载apache-pulsar-offloaders安装包，然后将apache-pulsar-offloaders安装到每台 broker 中 Pulsar 安装路径的offloaders 目录下。 了解该特性的详细配置，请参考分层存储指南。




# Apache Pulsar集群搭建

* Pulsar IO连接器
* 分层存储特性


部署机器(YZ)：
xx.xx.4.231
xx.xx.4.232
xx.xx.4.233