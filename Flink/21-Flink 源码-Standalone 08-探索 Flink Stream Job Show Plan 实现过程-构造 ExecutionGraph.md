# Flink 源码 - Standalone - 探索 Flink Stream Job Show Plan 实现过程 - 构建 ExecutionGraph                

>Flink version: 1.17.2       

## 引言   
在之前的 Blog "Flink 源码 - Standalone - 探索 Flink Stream Job Show Plan 实现过程 - 构建 StreamGraph" 和 "Flink 源码 - Standalone - 探索 Flink Stream Job Show Plan 实现过程 - 构建 JobGraph" 讲解了 Flink Job Main() Stream API 转成 StreamGraph，再转成 JobGraph 流程，此时，不知道你是否跟我一样，有种兴奋感觉，那种离真相越来越近的感觉。      

我们需要引述官网的一段介绍来引入该篇 Blog的主题：   
>###  JobManager Data Structures #
>During job execution, the JobManager keeps track of distributed tasks, decides when to schedule the next task (or set of tasks), and reacts to finished tasks or execution failures.  

>The JobManager receives the JobGraph , which is a representation of the data flow consisting of operators ( JobVertex ) and intermediate results ( IntermediateDataSet ). Each operator has properties, like the parallelism and the code that it executes. In addition, the JobGraph has a set of attached libraries, that are necessary to execute the code of the operators.     

>The JobManager transforms the JobGraph into an ExecutionGraph . The ExecutionGraph is a parallel version of the JobGraph: For each JobVertex, it contains an ExecutionVertex per parallel subtask. An operator with a parallelism of 100 will have one JobVertex and 100 ExecutionVertices. The ExecutionVertex tracks the state of execution of a particular subtask. All ExecutionVertices from one JobVertex are held in an ExecutionJobVertex , which tracks the status of the operator as a whole. Besides the vertices, the ExecutionGraph also contains the IntermediateResult and the IntermediateResultPartition . The former tracks the state of the IntermediateDataSet, the latter tracks the state of each of its partitions.  

(`https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/internals/job_scheduling/#jobmanager-data-structures`)






refer     
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/internals/job_scheduling/             

