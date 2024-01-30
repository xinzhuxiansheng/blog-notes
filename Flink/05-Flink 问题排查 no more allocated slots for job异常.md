## Flink入门篇(2)-no more allocated slots for job异常处理

运行模式: Flink on Native k8s application mode


### 异常描述

```
org.apache.flink.util.FlinkException: TaskExecutor akka.tcp://flink@xx.xx.130.130:6122/user/rpc/taskmanager_0 has no more allocated slots for job 6d876f68244d28d20c7db26a26a9c091. 
	at org.apache.flink.runtime.taskexecutor.TaskExecutor.closeJobManagerConnectionIfNoAllocatedResources(TaskExecutor.java:1936) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.runtime.taskexecutor.TaskExecutor.freeSlotInternal(TaskExecutor.java:1917) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.runtime.taskexecutor.TaskExecutor.timeoutSlot(TaskExecutor.java:1950) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.runtime.taskexecutor.TaskExecutor.access$3200(TaskExecutor.java:183) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.runtime.taskexecutor.TaskExecutor$SlotActionsImpl.lambda$timeoutSlot$1(TaskExecutor.java:2352) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.lambda$handleRunAsync$4(AkkaRpcActor.java:455) ~[flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at org.apache.flink.runtime.concurrent.akka.ClassLoadingUtils.runWithContextClassLoader(ClassLoadingUtils.java:68) ~[flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.handleRunAsync(AkkaRpcActor.java:455) ~[flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.handleRpcMessage(AkkaRpcActor.java:213) ~[flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at org.apache.flink.runtime.rpc.akka.AkkaRpcActor.handleMessage(AkkaRpcActor.java:163) ~[flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.japi.pf.UnitCaseStatement.apply(CaseStatements.scala:24) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.japi.pf.UnitCaseStatement.apply(CaseStatements.scala:20) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at scala.PartialFunction.applyOrElse(PartialFunction.scala:123) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at scala.PartialFunction.applyOrElse$(PartialFunction.scala:122) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.japi.pf.UnitCaseStatement.applyOrElse(CaseStatements.scala:20) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at scala.PartialFunction$OrElse.applyOrElse(PartialFunction.scala:171) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at scala.PartialFunction$OrElse.applyOrElse(PartialFunction.scala:172) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at scala.PartialFunction$OrElse.applyOrElse(PartialFunction.scala:172) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.actor.Actor.aroundReceive(Actor.scala:537) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.actor.Actor.aroundReceive$(Actor.scala:535) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.actor.AbstractActor.aroundReceive(AbstractActor.scala:220) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.actor.ActorCell.receiveMessage(ActorCell.scala:580) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.actor.ActorCell.invoke(ActorCell.scala:548) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.dispatch.Mailbox.processMailbox(Mailbox.scala:270) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.dispatch.Mailbox.run(Mailbox.scala:231) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at akka.dispatch.Mailbox.exec(Mailbox.scala:243) [flink-rpc-akka_a1d94605-8b4d-4260-9480-da1bc2321c32.jar:1.14.4] 
	at java.util.concurrent.ForkJoinTask.doExec(ForkJoinTask.java:289) [?:1.8.0_322] 
	at java.util.concurrent.ForkJoinPool$WorkQueue.runTask(ForkJoinPool.java:1056) [?:1.8.0_322] 
	at java.util.concurrent.ForkJoinPool.runWorker(ForkJoinPool.java:1692) [?:1.8.0_322] 
	at java.util.concurrent.ForkJoinWorkerThread.run(ForkJoinWorkerThread.java:175) [?:1.8.0_322] 

```

### 解决
refer: https://cwiki.apache.org/confluence/display/FLINK/Akka+and+Actors

```
-Dakka.ask.timeout=100s
-Dtaskmanager.slot.timeout=100s
```

