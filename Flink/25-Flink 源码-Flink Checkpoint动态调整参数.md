# Flink 源码 - Flink Checkpoint 动态调整参数        

## 引言     
Flink作为有状态的流式计算引擎，周期性的 Checkpoint 至关重要。Checkpoint的 周期不宜设置过长或过短，针对不同的任务要区别对待。甚至针对同一个任务，在不同场景下 Checkpoint 过程也会因为超时或反压等原因导致失败。 下面先来看一下传统checkpoint调优所面临的问题:            


### Flink Checkpoint 调优痛点
Flink Checkpoint 速率、频率、超时时间参数等直接影响了任务的健康度。当 flink 任务重启时，会因消息积压导致任务反压，任务反压反过来会促使 Checkpoint 变慢甚至是超时。如此一来，仿佛进入了一个恶性循环。            
* 静态调整：flink 任务的 checkpoint 相关参数，必须在任务运行前提前设置好，运行时是没办法动态调整的       
* 影响数据时效：重启任务调整checkpoint，必然带来消息处理的延迟，对于实时性要求非常高的场景，影响很大      
* 加剧反压：重启任务后，会带来数据消费的滞后性，如果任务本身checkpoint耗时比较长，还会因为反压与同时做checkpoint带来性能进一步的恶化      






refer       
1.https://gitee.com/fire-framework/fire/blob/master/docs/highlight/checkpoint.md#https://gitee.com/link?target=https%3A%2F%2Fgithub.com%2FZTO-Express%2Ffire        
