# StreamPark - 探索有限数据源 Job 的状态

>streampark version: 2.1.5         

## 背景  
最近在用 `StreamPark` 管理 Flink on Kubernetes（Native）Application Mode 作业,在这个过程中遇到作业状态不符合预期，若作业 Source 是有限数据源时，通过 StreamPark 启动的作业执行完后，作业状态显示 `FAILED`, 但当作业正常完成后，作业状态预期应该是 `FINISHED`。            

![streamparkjobstatuserror01](http://img.xinzhuxiansheng.com/blogimgs/flink/streamparkjobstatuserror01.jpg)     

## StreamPark 作业状态不符合预期  
注意，我的 streampark version 是 2.1.5。    

首先，我们通过 StreamPark 源码中 `streampark-flink-kubernetes` 模块找到了 作业状态监听的处理逻辑是 `FlinkJobStatusWatcher#doStart()`。 它通过 java 定时线程池触发 `doWatch()` 方法来监听作业状态。 我们出问题的作业状态是已经 `完成` 的作业，当 Flink on Kubernetes时，针对已 `完成`的作业，它从 k8s中已经无法查看了，它也没有留下任何查看的痕迹。 这部分实践逻辑，从 StreamPark 处理已经`完成`的作业中得到验证。    

![streamparkjobstatuserror03](http://img.xinzhuxiansheng.com/blogimgs/flink/streamparkjobstatuserror03.jpg)  

在 `FlinkJobStatusWatcher#inferStateFromK8sEvent()` 方法对 `完成` 的作业会去作业归档路径下查找它的信息来判断它的 `最终` 状态。   

![streamparkjobstatuserror04](http://img.xinzhuxiansheng.com/blogimgs/flink/streamparkjobstatuserror04.jpg)     

### 配置归档参数  
关于 `归档`参数 需要在 StreamPark `Flink Home`下的 `flink-conf.yaml` 配置。（直接修改文件后，再 Sync Conf）       
配置内容：      
```bash
historyserver.web.port: 8082  
historyserver.web.address: 0.0.0.0  
jobmanager.archive.fs.dir: s3://xxx/xxx/xxx/  
historyserver.archive.fs.dir: s3://xxx/xxx/xxx/  
historyserver.archive.fs.refresh-interval: 10000   
historyserver.web.tmpdir: /tmp/flink/history  
historyserver.archive.retained-jobs: 200  
```

![streamparkjobstatuserror05](http://img.xinzhuxiansheng.com/blogimgs/flink/streamparkjobstatuserror05.jpg)  

有了归档参数，我们重新部署作业就可以了。     

### 归档 JSON  
在 `FlinkHistoryArchives#getJobStateFromArchiveFile()` 方法会从 `jobmanager.archive.fs.dir` 参数获取归档路径，再通过 Flink 归档 API `FsJobArchivist.getArchivedJsons(archivePath)` 方法读取当前作业归档后的 JSON  

```java
def getJobStateFromArchiveFile(trackId: TrackId): String = Try {
require(trackId.jobId != null, "[StreamPark] getJobStateFromArchiveFile: JobId cannot be null.")
val archiveDir = trackId.properties.getProperty(JobManagerOptions.ARCHIVE_DIR.key)
if (archiveDir == null) {
    FAILED_STATE
} else {
    val archivePath = new Path(archiveDir, trackId.jobId)
    FsJobArchivist.getArchivedJsons(archivePath) match {
    case r if r.isEmpty => FAILED_STATE
    case r =>
        r.foreach {
        a =>
            if (a.getPath == s"/jobs/${trackId.jobId}/exceptions") {
            Try(parse(a.getJson)) match {
                case Success(ok) =>
                val log = (ok \ "root-exception").extractOpt[String].orNull
                if (log != null) {
                    val path = KubernetesDeploymentHelper.getJobErrorLog(trackId.jobId)
                    val file = new File(path)
                    Files.asCharSink(file, Charsets.UTF_8).write(log)
                    println(" error path: " + path)
                }
                case _ =>
            }
            } else if (a.getPath == "/jobs/overview") {
            Try(parse(a.getJson)) match {
                case Success(ok) =>
                ok \ "jobs" match {
                    case JNothing | JNull =>
                    case JArray(arr) =>
                    arr.foreach(
                        x => {
                        val jid = (x \ "jid").extractOpt[String].orNull
                        if (jid == trackId.jobId) {
                            return (x \ "state").extractOpt[String].orNull
                        }
                        })
                    case _ =>
                }
                case Failure(_) =>
            }
            }
        }
        FAILED_STATE
    }
}
}.getOrElse(FAILED_STATE)
```

下面是归档的 JSON 列表，file name 是以 作业 job id 命名的。   
![streamparkjobstatuserror06](http://img.xinzhuxiansheng.com/blogimgs/flink/streamparkjobstatuserror06.jpg)   

我们来看一个归档 JSON 的内容：          
```json
{
    "archive": [
        {
            "path": "/jobs/overview",
            "json": "{\"jobs\":[{\"jid\":\"3a93aa1064b1eeb1febf594ba08d992f\",\"name\":\"wordCountBatch\",\"state\":\"FINISHED\",\"start-time\":1742984931532,\"end-time\":1742984938167,\"duration\":6635,\"last-modification\":1742984938167,\"tasks\":{\"total\":2,\"created\":0,\"scheduled\":0,\"deploying\":0,\"running\":0,\"finished\":2,\"canceling\":0,\"canceled\":0,\"failed\":0,\"reconciling\":0,\"initializing\":0}}]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/config",
            "json": "{\"jid\":\"3a93aa1064b1eeb1febf594ba08d992f\",\"name\":\"wordCountBatch\",\"execution-config\":{\"execution-mode\":\"PIPELINED\",\"restart-strategy\":\"Cluster level default restart strategy\",\"job-parallelism\":1,\"object-reuse-mode\":false,\"user-config\":{}}}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/checkpoints/config",
            "json": "{\"mode\":\"at_least_once\",\"interval\":9223372036854775807,\"timeout\":600000,\"min_pause\":0,\"max_concurrent\":1,\"externalization\":{\"enabled\":false,\"delete_on_cancellation\":true},\"state_backend\":\"BatchExecutionStateBackend\",\"checkpoint_storage\":\"BatchExecutionCheckpointStorage\",\"unaligned_checkpoints\":false,\"tolerable_failed_checkpoints\":0,\"aligned_checkpoint_timeout\":0,\"checkpoints_after_tasks_finish\":true}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/checkpoints",
            "json": "{\"counts\":{\"restored\":0,\"total\":0,\"in_progress\":0,\"completed\":0,\"failed\":0},\"summary\":{\"checkpointed_size\":{\"min\":0,\"max\":0,\"avg\":0,\"p50\":\"NaN\",\"p90\":\"NaN\",\"p95\":\"NaN\",\"p99\":\"NaN\",\"p999\":\"NaN\"},\"state_size\":{\"min\":0,\"max\":0,\"avg\":0,\"p50\":\"NaN\",\"p90\":\"NaN\",\"p95\":\"NaN\",\"p99\":\"NaN\",\"p999\":\"NaN\"},\"end_to_end_duration\":{\"min\":0,\"max\":0,\"avg\":0,\"p50\":\"NaN\",\"p90\":\"NaN\",\"p95\":\"NaN\",\"p99\":\"NaN\",\"p999\":\"NaN\"},\"alignment_buffered\":{\"min\":0,\"max\":0,\"avg\":0,\"p50\":0.0,\"p90\":0.0,\"p95\":0.0,\"p99\":0.0,\"p999\":0.0},\"processed_data\":{\"min\":0,\"max\":0,\"avg\":0,\"p50\":\"NaN\",\"p90\":\"NaN\",\"p95\":\"NaN\",\"p99\":\"NaN\",\"p999\":\"NaN\"},\"persisted_data\":{\"min\":0,\"max\":0,\"avg\":0,\"p50\":\"NaN\",\"p90\":\"NaN\",\"p95\":\"NaN\",\"p99\":\"NaN\",\"p999\":\"NaN\"}},\"latest\":{\"completed\":null,\"savepoint\":null,\"failed\":null,\"restored\":null},\"history\":[]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/plan",
            "json": "{\"plan\":{\"jid\":\"3a93aa1064b1eeb1febf594ba08d992f\",\"name\":\"wordCountBatch\",\"type\":\"BATCH\",\"nodes\":[{\"id\":\"90bea66de1c231edf33913ecd54406c1\",\"parallelism\":1,\"operator\":\"\",\"operator_strategy\":\"\",\"description\":\"Keyed Aggregation<br/>+- Sink: Print to Std. Out<br/>\",\"inputs\":[{\"num\":0,\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"ship_strategy\":\"HASH\",\"exchange\":\"blocking\"}],\"optimizer_properties\":{}},{\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"parallelism\":1,\"operator\":\"\",\"operator_strategy\":\"\",\"description\":\"Source: Collection Source<br/>+- Flat Map<br/>\",\"optimizer_properties\":{}}]}}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/exceptions",
            "json": "{\"root-exception\":null,\"timestamp\":null,\"all-exceptions\":[],\"truncated\":false,\"exceptionHistory\":{\"entries\":[],\"truncated\":false}}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f",
            "json": "{\"jid\":\"3a93aa1064b1eeb1febf594ba08d992f\",\"name\":\"wordCountBatch\",\"isStoppable\":false,\"state\":\"FINISHED\",\"start-time\":1742984931532,\"end-time\":1742984938167,\"duration\":6635,\"maxParallelism\":-1,\"now\":1742984938186,\"timestamps\":{\"CREATED\":1742984931637,\"RUNNING\":1742984931741,\"RESTARTING\":0,\"CANCELED\":0,\"FINISHED\":1742984938167,\"FAILED\":0,\"SUSPENDED\":0,\"INITIALIZING\":1742984931532,\"FAILING\":0,\"CANCELLING\":0,\"RECONCILING\":0},\"vertices\":[{\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"name\":\"Source: Collection Source -> Flat Map\",\"maxParallelism\":128,\"parallelism\":1,\"status\":\"FINISHED\",\"start-time\":1742984937067,\"end-time\":1742984937947,\"duration\":880,\"tasks\":{\"RECONCILING\":0,\"RUNNING\":0,\"FINISHED\":1,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":0,\"DEPLOYING\":0,\"SCHEDULED\":0,\"INITIALIZING\":0,\"FAILED\":0},\"metrics\":{\"read-bytes\":0,\"read-bytes-complete\":true,\"write-bytes\":13846,\"write-bytes-complete\":true,\"read-records\":0,\"read-records-complete\":true,\"write-records\":864,\"write-records-complete\":true}},{\"id\":\"90bea66de1c231edf33913ecd54406c1\",\"name\":\"Keyed Aggregation -> Sink: Print to Std. Out\",\"maxParallelism\":128,\"parallelism\":1,\"status\":\"FINISHED\",\"start-time\":1742984937951,\"end-time\":1742984938166,\"duration\":215,\"tasks\":{\"RECONCILING\":0,\"RUNNING\":0,\"FINISHED\":1,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":0,\"DEPLOYING\":0,\"SCHEDULED\":0,\"INITIALIZING\":0,\"FAILED\":0},\"metrics\":{\"read-bytes\":13842,\"read-bytes-complete\":true,\"write-bytes\":0,\"write-bytes-complete\":true,\"read-records\":864,\"read-records-complete\":true,\"write-records\":0,\"write-records-complete\":true}}],\"status-counts\":{\"RECONCILING\":0,\"RUNNING\":0,\"FINISHED\":2,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":0,\"DEPLOYING\":0,\"SCHEDULED\":0,\"INITIALIZING\":0,\"FAILED\":0},\"plan\":{\"jid\":\"3a93aa1064b1eeb1febf594ba08d992f\",\"name\":\"wordCountBatch\",\"type\":\"BATCH\",\"nodes\":[{\"id\":\"90bea66de1c231edf33913ecd54406c1\",\"parallelism\":1,\"operator\":\"\",\"operator_strategy\":\"\",\"description\":\"Keyed Aggregation<br/>+- Sink: Print to Std. Out<br/>\",\"inputs\":[{\"num\":0,\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"ship_strategy\":\"HASH\",\"exchange\":\"blocking\"}],\"optimizer_properties\":{}},{\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"parallelism\":1,\"operator\":\"\",\"operator_strategy\":\"\",\"description\":\"Source: Collection Source<br/>+- Flat Map<br/>\",\"optimizer_properties\":{}}]}}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/accumulators",
            "json": "{\"job-accumulators\":[],\"user-task-accumulators\":[],\"serialized-user-task-accumulators\":{}}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/cbc357ccb763df2852fee8c4fc7d55f2/subtasktimes",
            "json": "{\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"name\":\"Source: Collection Source -> Flat Map\",\"now\":1742984938195,\"subtasks\":[{\"subtask\":0,\"host\":\"xx.xxx.80.32\",\"duration\":6203,\"timestamps\":{\"RECONCILING\":0,\"RUNNING\":1742984937874,\"FINISHED\":1742984937947,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":1742984931649,\"DEPLOYING\":1742984937067,\"SCHEDULED\":1742984931744,\"INITIALIZING\":1742984937765,\"FAILED\":0}}]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/90bea66de1c231edf33913ecd54406c1/subtasktimes",
            "json": "{\"id\":\"90bea66de1c231edf33913ecd54406c1\",\"name\":\"Keyed Aggregation -> Sink: Print to Std. Out\",\"now\":1742984938197,\"subtasks\":[{\"subtask\":0,\"host\":\"xx.xxx.80.32\",\"duration\":216,\"timestamps\":{\"RECONCILING\":0,\"RUNNING\":1742984938080,\"FINISHED\":1742984938166,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":1742984931649,\"DEPLOYING\":1742984937951,\"SCHEDULED\":1742984937950,\"INITIALIZING\":1742984937971,\"FAILED\":0}}]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/cbc357ccb763df2852fee8c4fc7d55f2/subtasks/0/attempts/0",
            "json": "{\"subtask\":0,\"status\":\"FINISHED\",\"attempt\":0,\"host\":\"xx.xxx.80.32\",\"start-time\":1742984937067,\"end-time\":1742984937947,\"duration\":880,\"metrics\":{\"read-bytes\":0,\"read-bytes-complete\":true,\"write-bytes\":13846,\"write-bytes-complete\":true,\"read-records\":0,\"read-records-complete\":true,\"write-records\":864,\"write-records-complete\":true},\"taskmanager-id\":\"yzhou-word-count-taskmanager-1-1\",\"start_time\":1742984937067}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/90bea66de1c231edf33913ecd54406c1/subtasks/0/attempts/0",
            "json": "{\"subtask\":0,\"status\":\"FINISHED\",\"attempt\":0,\"host\":\"xx.xxx.80.32\",\"start-time\":1742984937951,\"end-time\":1742984938166,\"duration\":215,\"metrics\":{\"read-bytes\":13842,\"read-bytes-complete\":true,\"write-bytes\":0,\"write-bytes-complete\":true,\"read-records\":864,\"read-records-complete\":true,\"write-records\":0,\"write-records-complete\":true},\"taskmanager-id\":\"yzhou-word-count-taskmanager-1-1\",\"start_time\":1742984937951}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/cbc357ccb763df2852fee8c4fc7d55f2/subtasks/0/attempts/0/accumulators",
            "json": "{\"subtask\":0,\"attempt\":0,\"id\":\"32aa90671cfbf8748b69c9ddd275642e\",\"user-accumulators\":[]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/90bea66de1c231edf33913ecd54406c1/subtasks/0/attempts/0/accumulators",
            "json": "{\"subtask\":0,\"attempt\":0,\"id\":\"9ca5b48edd9f4d671cfb8945e5b4f762\",\"user-accumulators\":[]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/cbc357ccb763df2852fee8c4fc7d55f2/taskmanagers",
            "json": "{\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"name\":\"Source: Collection Source -> Flat Map\",\"now\":1742984938200,\"taskmanagers\":[{\"host\":\"xx.xxx.80.32:37571\",\"status\":\"FINISHED\",\"start-time\":1742984937067,\"end-time\":1742984937947,\"duration\":880,\"metrics\":{\"read-bytes\":0,\"read-bytes-complete\":true,\"write-bytes\":13846,\"write-bytes-complete\":true,\"read-records\":0,\"read-records-complete\":true,\"write-records\":864,\"write-records-complete\":true},\"status-counts\":{\"RECONCILING\":0,\"RUNNING\":0,\"FINISHED\":1,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":0,\"DEPLOYING\":0,\"SCHEDULED\":0,\"INITIALIZING\":0,\"FAILED\":0},\"taskmanager-id\":\"yzhou-word-count-taskmanager-1-1\"}]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/90bea66de1c231edf33913ecd54406c1/taskmanagers",
            "json": "{\"id\":\"90bea66de1c231edf33913ecd54406c1\",\"name\":\"Keyed Aggregation -> Sink: Print to Std. Out\",\"now\":1742984938202,\"taskmanagers\":[{\"host\":\"xx.xxx.80.32:37571\",\"status\":\"FINISHED\",\"start-time\":1742984937951,\"end-time\":1742984938166,\"duration\":215,\"metrics\":{\"read-bytes\":13842,\"read-bytes-complete\":true,\"write-bytes\":0,\"write-bytes-complete\":true,\"read-records\":864,\"read-records-complete\":true,\"write-records\":0,\"write-records-complete\":true},\"status-counts\":{\"RECONCILING\":0,\"RUNNING\":0,\"FINISHED\":1,\"CANCELING\":0,\"CANCELED\":0,\"CREATED\":0,\"DEPLOYING\":0,\"SCHEDULED\":0,\"INITIALIZING\":0,\"FAILED\":0},\"taskmanager-id\":\"yzhou-word-count-taskmanager-1-1\"}]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/cbc357ccb763df2852fee8c4fc7d55f2",
            "json": "{\"id\":\"cbc357ccb763df2852fee8c4fc7d55f2\",\"name\":\"Source: Collection Source -> Flat Map\",\"parallelism\":1,\"maxParallelism\":128,\"now\":1742984938202,\"subtasks\":[{\"subtask\":0,\"status\":\"FINISHED\",\"attempt\":0,\"host\":\"xx.xxx.80.32\",\"start-time\":1742984937067,\"end-time\":1742984937947,\"duration\":880,\"metrics\":{\"read-bytes\":0,\"read-bytes-complete\":true,\"write-bytes\":13846,\"write-bytes-complete\":true,\"read-records\":0,\"read-records-complete\":true,\"write-records\":864,\"write-records-complete\":true},\"taskmanager-id\":\"yzhou-word-count-taskmanager-1-1\",\"start_time\":1742984937067}]}"
        },
        {
            "path": "/jobs/3a93aa1064b1eeb1febf594ba08d992f/vertices/90bea66de1c231edf33913ecd54406c1",
            "json": "{\"id\":\"90bea66de1c231edf33913ecd54406c1\",\"name\":\"Keyed Aggregation -> Sink: Print to Std. Out\",\"parallelism\":1,\"maxParallelism\":128,\"now\":1742984938203,\"subtasks\":[{\"subtask\":0,\"status\":\"FINISHED\",\"attempt\":0,\"host\":\"xx.xxx.80.32\",\"start-time\":1742984937951,\"end-time\":1742984938166,\"duration\":215,\"metrics\":{\"read-bytes\":13842,\"read-bytes-complete\":true,\"write-bytes\":0,\"write-bytes-complete\":true,\"read-records\":864,\"read-records-complete\":true,\"write-records\":0,\"write-records-complete\":true},\"taskmanager-id\":\"yzhou-word-count-taskmanager-1-1\",\"start_time\":1742984937951}]}"
        }
    ]
}
```   

StreamPark 基于上面的归档 JSON 判断作业最终的状态。 首先根据 `/jobs/${trackId.jobId}/exceptions` path 判断作业是否包含 `root-exception` 信息，若存在则作业状态标记为 `FAILED`。 若上面条件不成立，则查询 `/jobs/overview` path 查看 Job 的 state。  

`知晓了上面的处理,对排查问题多了一些依据，但并没有解决我的问题`。 我们接着往下看 ...        

### Scala Try{}  
`FlinkHistoryArchives#getJobStateFromArchiveFile()` 方法中的 Try{...}.getOrElse(FAILED_STATE) 语句，首先尝试执行 `{...}` 中的代码，如果成功返回其结果，如果抛出异常，就返回 `FAILED_STATE`。    

这就是我的问题所在，当该方法发生异常时，它默认返回 `FAILED_STATE`, 并不会打印异常栈信息，隐藏了问题。 我虽然在 flink-conf.yaml 中配置了 归档参数，但是我并没有提前放置 归档存储的 jar 和 认证 file ，所以导致 `FsJobArchivist.getArchivedJsons(archivePath)` 方法会发生异常，导致作业状态默认返回 `FAILED_STATE`。   

![streamparkjobstatuserror07](http://img.xinzhuxiansheng.com/blogimgs/flink/streamparkjobstatuserror07.jpg)

针对作业状态监听，我想你估计跟我想的一样，即使发生异常，也不希望它的状态返回 `FAILED_STATE`, 如果抛出异常，或者标记 `唯一` 状态，我们也好排查问题。   

如果我们要解决它，可以减小 Try{} 的作用域，或者 在发生异常的方法外添加 try{} catch{}，手动在 catch 里面抛出异常栈信息。      

到这里，我的问题解决啦。 !!!       
