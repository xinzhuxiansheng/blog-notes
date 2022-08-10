##  Failed to rollback to checkpoint/savepoint异常

### 异常描述：	
```
2022-08-11 00:05:00,905 INFO  org.apache.flink.runtime.dispatcher.StandaloneDispatcher     [] - Job 0e33d5018ce6d8911117001823466c59 reached terminal state FAILED. 
org.apache.flink.runtime.client.JobInitializationException: Could not start the JobMaster. 
	at org.apache.flink.runtime.jobmaster.DefaultJobMasterServiceProcess.lambda$new$0(DefaultJobMasterServiceProcess.java:97) 
	at java.util.concurrent.CompletableFuture.uniWhenComplete(CompletableFuture.java:774) 
	at java.util.concurrent.CompletableFuture$UniWhenComplete.tryFire(CompletableFuture.java:750) 
	at java.util.concurrent.CompletableFuture.postComplete(CompletableFuture.java:488) 
	at java.util.concurrent.CompletableFuture$AsyncSupply.run(CompletableFuture.java:1609) 
	at java.util.concurrent.ThreadPoolExecutor.runWorker(ThreadPoolExecutor.java:1149) 
	at java.util.concurrent.ThreadPoolExecutor$Worker.run(ThreadPoolExecutor.java:624) 
	at java.lang.Thread.run(Thread.java:750) 
Caused by: java.util.concurrent.CompletionException: java.lang.IllegalStateException: Failed to rollback to checkpoint/savepoint s3://fmflink-savepoint/fm-rm7o0vsnqj/savepoint-256b32-a2dc1e4c22ad. Cannot map checkpoint/savepoint state for operator 6cdc5bb954874d922eaee11a8e7b5dd5 to the new program, because the operator is not available in the new program. If you want to allow to skip this, you can set the --allowNonRestoredState option on the CLI. 
	at java.util.concurrent.CompletableFuture.encodeThrowable(CompletableFuture.java:273) 
	at java.util.concurrent.CompletableFuture.completeThrowable(CompletableFuture.java:280) 
	at java.util.concurrent.CompletableFuture$AsyncSupply.run(CompletableFuture.java:1606) 
	... 3 more 
Caused by: java.lang.IllegalStateException: Failed to rollback to checkpoint/savepoint s3://fmflink-savepoint/fm-rm7o0vsnqj/savepoint-256b32-a2dc1e4c22ad. Cannot map checkpoint/savepoint state for operator 6cdc5bb954874d922eaee11a8e7b5dd5 to the new program, because the operator is not available in the new program. If you want to allow to skip this, you can set the --allowNonRestoredState option on the CLI. 
	at org.apache.flink.runtime.checkpoint.Checkpoints.throwNonRestoredStateException(Checkpoints.java:230) 
	at org.apache.flink.runtime.checkpoint.Checkpoints.loadAndValidateCheckpoint(Checkpoints.java:188) 
	at org.apache.flink.runtime.checkpoint.CheckpointCoordinator.restoreSavepoint(CheckpointCoordinator.java:1648) 
	at org.apache.flink.runtime.scheduler.DefaultExecutionGraphFactory.tryRestoreExecutionGraphFromSavepoint(DefaultExecutionGraphFactory.java:163) 
	at org.apache.flink.runtime.scheduler.DefaultExecutionGraphFactory.createAndRestoreExecutionGraph(DefaultExecutionGraphFactory.java:138) 
	at org.apache.flink.runtime.scheduler.SchedulerBase.createAndRestoreExecutionGraph(SchedulerBase.java:335) 
	at org.apache.flink.runtime.scheduler.SchedulerBase.<init>(SchedulerBase.java:191) 
	at org.apache.flink.runtime.scheduler.DefaultScheduler.<init>(DefaultScheduler.java:140) 
	at org.apache.flink.runtime.scheduler.DefaultSchedulerFactory.createInstance(DefaultSchedulerFactory.java:134) 
	at org.apache.flink.runtime.jobmaster.DefaultSlotPoolServiceSchedulerFactory.createScheduler(DefaultSlotPoolServiceSchedulerFactory.java:110) 
	at org.apache.flink.runtime.jobmaster.JobMaster.createScheduler(JobMaster.java:346) 
	at org.apache.flink.runtime.jobmaster.JobMaster.<init>(JobMaster.java:323) 
	at org.apache.flink.runtime.jobmaster.factories.DefaultJobMasterServiceFactory.internalCreateJobMasterService(DefaultJobMasterServiceFactory.java:106) 
	at org.apache.flink.runtime.jobmaster.factories.DefaultJobMasterServiceFactory.lambda$createJobMasterService$0(DefaultJobMasterServiceFactory.java:94) 
	at org.apache.flink.util.function.FunctionUtils.lambda$uncheckedSupplier$4(FunctionUtils.java:112) 
	at java.util.concurrent.CompletableFuture$AsyncSupply.run(CompletableFuture.java:1604) 
	... 3 more 
2022-08-11 00:05:01,458 INFO  org.apache.flink.runtime.jobmanager.DefaultJobGraphStore     [] - Removed job graph 0e33d5018ce6d8911117001823466c59 from KubernetesStateHandleStore{configMapName='fm-rm7o0vsnqj-dispatcher-leader'}. 
2022-08-11 00:05:01,467 INFO  org.apache.flink.runtime.leaderelection.DefaultLeaderElectionService [] - Stopping DefaultLeaderElectionService. 
2022-08-11 00:05:01,467 INFO  org.apache.flink.kubernetes.highavailability.KubernetesLeaderElectionDriver [] - Closing KubernetesLeaderElectionDriver{configMapName='fm-rm7o0vsnqj-0e33d5018ce6d8911117001823466c59-jobmanager-leader'}. 
2022-08-11 00:05:01,467 INFO  org.apache.flink.kubernetes.kubeclient.resources.KubernetesConfigMapSharedInformer [] - Stopped to watch for ark-middleware-flink-0/fm-rm7o0vsnqj-0e33d5018ce6d8911117001823466c59-jobmanager-leader, watching id:785cba2b-79e0-4135-94f2-c3bb1cabd7b4 
2022-08-11 00:05:01,552 INFO  org.apache.flink.kubernetes.highavailability.KubernetesHaServices [] - Clean up the high availability data for job 0e33d5018ce6d8911117001823466c59. 
2022-08-11 00:05:01,583 INFO  org.apache.flink.kubernetes.highavailability.KubernetesHaServices [] - Finished cleaning up the high availability data for job 0e33d5018ce6d8911117001823466c59. 
2022-08-11 00:05:02,616 WARN  org.apache.flink.client.deployment.application.ApplicationDispatcherBootstrap [] - Application failed unexpectedly: 
java.util.concurrent.CompletionException: org.apache.flink.client.deployment.application.ApplicationExecutionException: Could not execute application. 
	at java.util.concurrent.CompletableFuture.encodeThrowable(CompletableFuture.java:292) ~[?:1.8.0_322] 
	at java.util.concurrent.CompletableFuture.completeThrowable(CompletableFuture.java:308) ~[?:1.8.0_322] 
	at java.util.concurrent.CompletableFuture.uniCompose(CompletableFuture.java:957) ~[?:1.8.0_322] 
	at java.util.concurrent.CompletableFuture$UniCompose.tryFire(CompletableFuture.java:940) ~[?:1.8.0_322] 
	at java.util.concurrent.CompletableFuture.postComplete(CompletableFuture.java:488) ~[?:1.8.0_322] 
	at java.util.concurrent.CompletableFuture.completeExceptionally(CompletableFuture.java:1990) ~[?:1.8.0_322] 
	at org.apache.flink.client.deployment.application.ApplicationDispatcherBootstrap.runApplicationEntryPoint(ApplicationDispatcherBootstrap.java:287) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.deployment.application.ApplicationDispatcherBootstrap.lambda$runApplicationAsync$2(ApplicationDispatcherBootstrap.java:224) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at java.util.concurrent.Executors$RunnableAdapter.call(Executors.java:511) ~[?:1.8.0_322] 
	at java.util.concurrent.FutureTask.run(FutureTask.java:266) ~[?:1.8.0_322] 
	at org.apache.flink.runtime.concurrent.akka.ActorSystemScheduledExecutorAdapter$ScheduledFutureTask.run(ActorSystemScheduledExecutorAdapter.java:171) ~[flink-rpc-akka_c6831bf2-86e4-458b-9297-62c17068a50d.jar:1.14.4] 
	at org.apache.flink.runtime.concurrent.akka.ClassLoadingUtils.runWithContextClassLoader(ClassLoadingUtils.java:68) ~[flink-rpc-akka_c6831bf2-86e4-458b-9297-62c17068a50d.jar:1.14.4] 
	at org.apache.flink.runtime.concurrent.akka.ClassLoadingUtils.lambda$withContextClassLoader$0(ClassLoadingUtils.java:41) ~[flink-rpc-akka_c6831bf2-86e4-458b-9297-62c17068a50d.jar:1.14.4] 
	at akka.dispatch.TaskInvocation.run(AbstractDispatcher.scala:49) [flink-rpc-akka_c6831bf2-86e4-458b-9297-62c17068a50d.jar:1.14.4] 
	at akka.dispatch.ForkJoinExecutorConfigurator$AkkaForkJoinTask.exec(ForkJoinExecutorConfigurator.scala:48) [flink-rpc-akka_c6831bf2-86e4-458b-9297-62c17068a50d.jar:1.14.4] 
	at java.util.concurrent.ForkJoinTask.doExec(ForkJoinTask.java:289) [?:1.8.0_322] 
	at java.util.concurrent.ForkJoinPool$WorkQueue.runTask(ForkJoinPool.java:1056) [?:1.8.0_322] 
	at java.util.concurrent.ForkJoinPool.runWorker(ForkJoinPool.java:1692) [?:1.8.0_322] 
	at java.util.concurrent.ForkJoinWorkerThread.run(ForkJoinWorkerThread.java:175) [?:1.8.0_322] 
Caused by: org.apache.flink.client.deployment.application.ApplicationExecutionException: Could not execute application. 
	... 13 more 
Caused by: org.apache.flink.client.program.ProgramInvocationException: The main method caused an error: Failed to execute job 'Mysql Binlog Job'. 
	at org.apache.flink.client.program.PackagedProgram.callMainMethod(PackagedProgram.java:372) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.program.PackagedProgram.invokeInteractiveModeForExecution(PackagedProgram.java:222) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.ClientUtils.executeProgram(ClientUtils.java:114) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.deployment.application.ApplicationDispatcherBootstrap.runApplicationEntryPoint(ApplicationDispatcherBootstrap.java:261) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	... 12 more 
Caused by: org.apache.flink.util.FlinkException: Failed to execute job 'Mysql Binlog Job'. 
	at org.apache.flink.streaming.api.environment.StreamExecutionEnvironment.executeAsync(StreamExecutionEnvironment.java:2055) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.program.StreamContextEnvironment.executeAsync(StreamContextEnvironment.java:137) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.program.StreamContextEnvironment.execute(StreamContextEnvironment.java:76) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.streaming.api.environment.StreamExecutionEnvironment.execute(StreamExecutionEnvironment.java:1916) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at com.flink.dts.job.cdc.mysql.Mysql2MysqlDataStream.main(Mysql2MysqlDataStream.java:138) ~[?:?] 
	at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method) ~[?:1.8.0_322] 
	at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62) ~[?:1.8.0_322] 
	at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43) ~[?:1.8.0_322] 
	at java.lang.reflect.Method.invoke(Method.java:498) ~[?:1.8.0_322] 
	at org.apache.flink.client.program.PackagedProgram.callMainMethod(PackagedProgram.java:355) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.program.PackagedProgram.invokeInteractiveModeForExecution(PackagedProgram.java:222) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.ClientUtils.executeProgram(ClientUtils.java:114) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	at org.apache.flink.client.deployment.application.ApplicationDispatcherBootstrap.runApplicationEntryPoint(ApplicationDispatcherBootstrap.java:261) ~[flink-dist_2.11-1.14.4.jar:1.14.4] 
	... 12 more 

2022-08-11 00:05:02,637 INFO  org.apache.flink.runtime.entrypoint.ClusterEntrypoint        [] - Shutting KubernetesApplicationClusterEntrypoint down with application status UNKNOWN. Diagnostics Cluster entrypoint has been closed externally.. 
2022-08-11 00:05:02,649 INFO  org.apache.flink.runtime.blob.BlobServer                     [] - Stopped BLOB server at 0.0.0.0:6124 
2022-08-11 00:05:02,704 INFO  org.apache.flink.runtime.jobmaster.MiniDispatcherRestEndpoint [] - Shutting down rest endpoint. 
2022-08-11 00:05:04,610 INFO  akka.remote.RemoteActorRefProvider$RemotingTerminator        [] - Shutting down remote daemon. 
2022-08-11 00:05:04,610 INFO  akka.remote.RemoteActorRefProvider$RemotingTerminator        [] - Shutting down remote daemon. 
2022-08-11 00:05:04,616 INFO  akka.remote.RemoteActorRefProvider$RemotingTerminator        [] - Remote daemon shut down; proceeding with flushing remote transports. 
2022-08-11 00:05:04,616 INFO  akka.remote.RemoteActorRefProvider$RemotingTerminator        [] - Remote daemon shut down; proceeding with flushing remote transports. 
2022-08-11 00:05:04,825 INFO  akka.remote.RemoteActorRefProvider$RemotingTerminator        [] - Remoting shut down. 
2022-08-11 00:05:04,825 INFO  akka.remote.RemoteActorRefProvider$RemotingTerminator        [] - Remoting shut down. 

```

### 解决

refer:
1.https://blog.csdn.net/congcong68/article/details/122182308
2.https://nightlies.apache.org/flink/flink-docs-release-1.14/zh/docs/ops/state/savepoints/#%e4%bb%8e-savepoint-%e6%81%a2%e5%a4%8d

```
从 Savepoint 恢复
$ bin/flink run -s :savepointPath [:runArgs]	
这将提交作业并指定要从中恢复的 Savepoint 。 你可以给出 Savepoint 目录或 _metadata 文件的路径。	

跳过无法映射的状态恢复 	
默认情况下，resume 操作将尝试将 Savepoint 的所有状态映射回你要还原的程序。 如果删除了运算符，则可以通过 --allowNonRestoredState（short：-n）选项跳过无法映射到新程序的状态：	

$ bin/flink run -s :savepointPath -n [:runArgs]
```