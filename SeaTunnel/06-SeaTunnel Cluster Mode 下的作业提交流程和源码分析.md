# SeaTunnel Cluster Mode 下的作业提交提交流程和源码分析         

>Seatunnel version: 2.3.4, Zeta Cluster Mode

## Job 提交结构图 
![zetasubmit01](images/zetasubmit01.png)            

上图中的内容，启动 Seatunnel zeta Cluster Server，只不过，Cluster 目前是单节点，往往我们在开发和测试阶段大多时候用的是单节点，这可简化我们时间成本。 你也可以通过 `curl`工具 请求 zeta clsuter rest api `http://192.168.0.201:5801/hazelcast/rest/cluster` 查看集群 `members`。  

>注意：/hazelcast/rest/cluster 是由 hazelcast 提供的 Rest api，无需做任何处理，想了解更多，可访问 hazelcast官网。       




refer   
1.https://seatunnel.apache.org/docs/2.3.4/connector-v2/sink/Socket     


