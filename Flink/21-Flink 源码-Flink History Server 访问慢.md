# Flink 源码 - Flink History - 探索 Flink History Server 查询已完成的作业 “慢” 问题      

>Flink version: 1.16.0 , Flink on Kubernetes   

## 背景     
身边的小伙伴陆续反馈，他们通过 `Flink History Server` 查看已完成的 Flink Job 信息时（例如 Exception Message），出现需要等待十几分钟(预估时间)才能看到，例如 12:00 作业发生异常，Job 状态变成 `FAILED`, 大概需要等到 12:18 才能在 `Flink History Server` 查看到。 对于他们想了解 Flink Job 运行时发生了哪些异常 Message 来说，这十几分钟等待是太漫长的。   

![fixflinkhistor01](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor01.jpg)   

他们大多数都会重新启动下 Flink Job，再通过 `Kubectl logs`实时查看 Flink Job Log 信息，若稍不在意，可能还会多次启动 Job，因为这部分工作靠的是`手速`。（目前我们还没有采集 Flink Log）      

>若你对 Flink History Server  还不了解，可以参考我的公众号文章 `https://mp.weixin.qq.com/s/reUzjh3luMO5q4-OYx8F5w`   

## 排查  
>注意：Flink History Server 核心配置如下：  
```bash
historyserver.archive.fs.dir: s3://flink-16-state/completed-jobs/
historyserver.archive.fs.refresh-interval: 10000
historyserver.archive.refresh-handlerthreadnum: 100
historyserver.web.tmpdir: /data/flink/history
historyserver.archive.retained-jobs: 200
```

### 不放过打印的 LOG   
得知问题后,首先将 Flink History Server 的 Log 级别动态调整为 `DEBUG`（若还不清楚如何调整，可以参考我的公众号文章 `https://mp.weixin.qq.com/s/DH81Xto6l6pm8NP7sbkxYA`）。这样便于通过 Log 来定位代码。  

我定位到了 `HistoryServerArchiveFetcher#fetchArchives()`，如下所示, 顿时觉得通过 `historyserver.sh`脚本定位 `HistoryServer`入口类 `不香了` ：）  
```hash
org.apache.flink.runtime.webmonitor.history.HistoryServerArchiveFetcher [] - Starting archive fetching.
```

## 分析 fetchArchives() 方法  
HistoryServer 会根据 `historyserver.archive.fs.refresh-interval` 参数，创建一个定时线程池，然后每间隔一段时间去执行 `fetchArchives()` 方法。它会先读取 `historyserver.archive.fs.dir` 参数配置的路径地址下的 JobManager 的归档 JSON 文件列表。  
![fixflinkhistor03](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor03.jpg)   

再通过 for 循环遍历 JSON 文件列表，针对已经解析过的，会放入 `cachedArchivesPerRefreshDirectory` 缓存中，避免重复解析。       

![fixflinkhistor02](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor02.jpg)        
  
整个归档 JSON 文件解析是由 `HistoryServerArchiveFetcher#processArchive()` 方法处理的，它会调用 `FsJobArchivist#getArchivedJsons()`方法解析得到多个 JSON 数据，这些数据会通过 for 循环轮询写入到文件中，被当作 Flink History Server 接口数据返回给前端。下面以 `a2accd2a99f639f00be04b937d3486de` JobId 为示例, 其 JobManager 存储在 S3 的归档 JSON 文件大小为 `27.4 MiB` 如下所示， 而它会被拆解出 `41611` 个接口数据文件， 这些文件主要分布在 `checkpoints`，`jobmanager`，`vertices` 文件夹下。         

`jobs/a2accd2a99f639f00be04b937d3486de 目录`    
![fixflinkhistor06](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor06.jpg)    

`jobmanager 归档 json 文件`  
![fixflinkhistor04](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor04.jpg)  

>建议大家可以看下 `historyserver.web.tmpdir` 参数配置路径下的文件，以及直接通过浏览器直接访问 json 文件路径试试, 可以直观感受 json file 对应的就是 接口返回值。       

processArchive() 调用 for 循环写入json 数据到文件中，并且每次都调用 flush() 方法，保证每次都立即刷新到磁盘中。     
```java
private void processArchive(String jobID, Path jobArchive) throws IOException {
    for (ArchivedJson archive : FsJobArchivist.getArchivedJsons(jobArchive)) {
        
        // 省略部分代码

        Files.createFile(target.toPath());
        try (FileWriter fw = new FileWriter(target)) {
            fw.write(json);
            fw.flush();
        }
    }
}
```

`包含 processArchive() 方法流程图`   
![fixflinkhistor07](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor07.jpg)  

`jobs/a2accd2a99f639f00be04b937d3486de 目录下的文件数`   
![fixflinkhistor05](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor05.jpg)    

### 小结  
分析到这里，我认为你应该可以意识到以下几点：  
1.在 `historyserver.web.tmpdir` 配置的路径下的 Jobs 目录下分别对应的 每个 JobID 的接口返回数据文件。    
2.在 JobId a2accd2a99f639f00be04b937d3486de 示例中，子文件有 41611 个，若通过 for循环遍历写，这必然会偏慢。     
3.json 文件的归类，例如，存放在 checkpoints 文件夹，jobmanager 文件夹，vertices 文件夹。    
4.vertices 目录下的文件多取决于算子复杂度（拓扑结构复杂、并行度多），可以推导出，算子多的 Flink Job 解析 JSON 文件写入到 磁盘中会更耗时。      

当把首次解析的 json 文件处理完后，fetchArchives() 会调用 updateJobOverview() 方法对 每个 `JobID.json` 进行整体写入到 `overview.json` 文件。 这一步大家应该不难理解，我们访问 `Completed Jobs`菜单查看的 Job 列表，它调用的是 `jobs/overview` 接口，此时就完成一个 JobID 整个解析流程，如下图所示：        

![fixflinkhistor08](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor08.jpg)  

**fetchArchives()** 
```java

        // 省略部分代码

                    LOG.info("Processing archive {}.", jobArchivePath);
                    try {
                        processArchive(jobID, jobArchivePath);
                        events.add(new ArchiveEvent(jobID, ArchiveEventType.CREATED));
                        cachedArchivesPerRefreshDirectory.get(refreshDir).add(jobID);
                        LOG.info("Processing archive {} finished.", jobArchivePath);
                    } catch (IOException e) {
                        LOG.error(
                                "Failure while fetching/processing job archive for job {}.",
                                jobID,
                                e);
                        deleteJobFiles(jobID);
                    }
                }
            }
        }

        if (jobsToRemove.values().stream().flatMap(Set::stream).findAny().isPresent()
                && processExpiredArchiveDeletion) {
            events.addAll(cleanupExpiredJobs(jobsToRemove));
        }
        if (!archivesBeyondSizeLimit.isEmpty() && processBeyondLimitArchiveDeletion) {
            events.addAll(cleanupJobsBeyondSizeLimit(archivesBeyondSizeLimit));
        }
        if (!events.isEmpty()) {
            updateJobOverview(webOverviewDir, webDir);    // 进行整合 
        }
        events.forEach(jobArchiveEventListener::accept);
        LOG.debug("Finished archive fetching.");
    } catch (Exception e) {
        LOG.error("Critical failure while fetching/processing job archives.", e);
    }
}
```

updateJobOverview() 方法中的形参，`webOverviewDir`,`webDir` 在 HistoryServerArchiveFetcher 类中是有定义的：   
```java
this.webJobDir = new File(webDir, "jobs");
Files.createDirectories(webJobDir.toPath());
this.webOverviewDir = new File(webDir, "overviews");
```

**updateJobOverview()**   
```java
private static void updateJobOverview(File webOverviewDir, File webDir) {
    long startTime = System.currentTimeMillis();
    LOG.info("updateJobOverview webOverviewDir: {}, webDir: {}", webOverviewDir.getPath(), webDir.getPath());
    try (JsonGenerator gen =
            jacksonFactory.createGenerator(
                    HistoryServer.createOrGetFile(webDir, JobsOverviewHeaders.URL))) {
        File[] overviews = new File(webOverviewDir.getPath()).listFiles();
        LOG.info("updateJobOverview overviews size:{}",overviews.length);
        if (overviews != null) {
            Collection<JobDetails> allJobs = new ArrayList<>(overviews.length);
            for (File overview : overviews) {
                LOG.info("updateJobOverview overview: {}", overview.getPath());
                MultipleJobsDetails subJobs =
                        mapper.readValue(overview, MultipleJobsDetails.class);
                allJobs.addAll(subJobs.getJobs());
            }
            mapper.writeValue(gen, new MultipleJobsDetails(allJobs));
        }
    } catch (IOException ioe) {
        LOG.error("Failed to update job overview.", ioe);
    }
    LOG.info("updateJobOverview cost time: {}", System.currentTimeMillis() - startTime);
}
```

`完整流程`      
![fixflinkhistor09](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor09.jpg)  

## 优化   
针对查询已完成的作业`慢`问题的思考逻辑如下：（为了方便介绍，我仍然拿上面 JobID=a2accd2a99f639f00be04b937d3486de 为示例 ）  

1.`processArchive()` 方法使用 for 循环遍历写入磁盘，若某个 Job 解析后要写入磁盘文件较多时，则偏慢，那是否可以考虑多线程 ？     
2.场景分析，打开 `Flink History Server` 为了看什么？ 我这边的场景主要是为了看 Exception,若要满足这点，我至少需要做2步：第一步，我需要在`已完成列表`中要看到我的作业,它对应的是 `http://192.168.0.143:30002/jobs/overview.json`; 第二步，进入作业详情，可看到 Exception,它对应的接口地址 `http://192.168.0.143:30002/jobs/a2accd2a99f639f00be04b937d3486de/exceptions.json`。

下面是优化后的逻辑图：  
![fixflinkhistor10](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor10.jpg)       
我将需要写入的JSON 接口数据文件，区分两类，一种是影响主业务，由主线程处理，`jobs/overview.json`，`jobs/JobID/exceptions.json`；另一种不着急看的数据，通过消息队列的方式先存储起来 `checkpoints 文件夹`，`jobmanager 文件夹`,`vertices 文件夹`，后面交由其他线程来处理，根据示例中文件个数占比来看，这是一个非常大的提升。  

>下面展示基于 `懒加载`的思路来实现它。   

### 优化后的代码目录     
![fixflinkhistor11](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor11.jpg)   

### 1.接入 Disruptor 组件 
在 flink-runtime-web 模块 pom.xml 添加 `disruptor` 依赖   
```xml
<dependency>
    <groupId>com.lmax</groupId>
    <artifactId>disruptor</artifactId>
    <version>3.4.2</version>
</dependency>
```

创建 `ArchivedJsonDisruptorManager`,`ArchivedJsonEventProducer`,`ArchivedJsonEventHandler`,`ArchivedJsonEventWrapper`。    

HistoryServerArchiveFetcher 构造方法中 创建 ArchivedJsonDisruptorManager，此时就已经完成 Disruptor 组件的接入了，它会在构造方法中创建 Producer 和 Handler，并且使用 handleEventsWithWorkerPool() 保证 Handler 共同处理队列中的数据。ArchivedJsonEventWrapper 将 jobArchive,jobID,archivedJSON 包装在一个对象中，因为它们是队列一个子项的数据。    

`关系图`
![fixflinkhistor12](http://img.xinzhuxiansheng.com/blogimgs/flink/fixflinkhistor12.jpg)  

**ArchivedJsonDisruptorManager**  
```java
public class ArchivedJsonDisruptorManager {
    Disruptor<ArchivedJsonEventWrapper> jsonDisruptor;
    RingBuffer<ArchivedJsonEventWrapper> ringBuffer;
    ArchivedJsonEventProducer eventProducer;
    File webDir;
    File webOverviewDir;

    public ArchivedJsonDisruptorManager(File webDir,File webOverviewDir) {
        this.webDir = webDir;
        this.webOverviewDir = webOverviewDir;

        jsonDisruptor = new Disruptor<>(
                ArchivedJsonEventWrapper::new,
                1024 * 1024,
                Executors.defaultThreadFactory(),
                ProducerType.SINGLE,
                new YieldingWaitStrategy()
        );
        initProducer();
        initHandler();
        jsonDisruptor.start();
    }


    public ArchivedJsonEventProducer getEventProducer() {
        return eventProducer;
    }


    private void initProducer(){
        ringBuffer = jsonDisruptor.getRingBuffer();
        eventProducer = new ArchivedJsonEventProducer(ringBuffer);
    }

    private void initHandler(){
        ArchivedJsonEventHandler handler01 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler02 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler03 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler04 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler05 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler06 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler07 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler08 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler09 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler10 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler11 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler12 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler13 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler14 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler15 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler16 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler17 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler18 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler19 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler20 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler21 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler22 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler23 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler24 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler25 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler26 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler27 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler28 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler29 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        ArchivedJsonEventHandler handler30 = new ArchivedJsonEventHandler(webDir,webOverviewDir);
        jsonDisruptor.handleEventsWithWorkerPool(
                handler01,
                handler02,
                handler03,
                handler04,
                handler05,
                handler06,
                handler07,
                handler08,
                handler09,
                handler10,
                handler11,
                handler12,
                handler13,
                handler14,
                handler15,
                handler16,
                handler17,
                handler18,
                handler19,
                handler20,
                handler21,
                handler22,
                handler23,
                handler24,
                handler25,
                handler26,
                handler27,
                handler28,
                handler29,
                handler30
        );}

}
```

**ArchivedJsonEventProducer**  
```java
public class ArchivedJsonEventProducer {
    private final RingBuffer<ArchivedJsonEventWrapper> ringBuffer;

    public ArchivedJsonEventProducer(RingBuffer<ArchivedJsonEventWrapper> ringBuffer) {
        this.ringBuffer = ringBuffer;
    }

    public void onData(Path jobArchive, String jobID, ArchivedJson data) {
        long sequence = ringBuffer.next();
        try {
            ArchivedJsonEventWrapper archivedJsonEventWrapper = ringBuffer.get(sequence);
            archivedJsonEventWrapper.setJobArchive(jobArchive);
            archivedJsonEventWrapper.setJobID(jobID);
            archivedJsonEventWrapper.setData(data);
        } finally {
            ringBuffer.publish(sequence);
        }
    }
}
```

**ArchivedJsonEventHandler**
```java
public class ArchivedJsonEventHandler implements  WorkHandler<ArchivedJsonEventWrapper> {
    private static final Logger LOG = LoggerFactory.getLogger(ArchivedJsonEventHandler.class);

    private final File webDir;
    private final File webOverviewDir;

    public ArchivedJsonEventHandler(File webDir, File webOverviewDir) {
        this.webDir = webDir;
        this.webOverviewDir = webOverviewDir;
    }

    @Override
    public void onEvent(ArchivedJsonEventWrapper event) {
        long st = System.currentTimeMillis();
        ArchivedJson archive = event.getData();
        Path jobArchive = event.getJobArchive();
        String jobID = event.getJobID();
        try{
            String path = archive.getPath();
            String json = archive.getJson();

            File target;
            if (path.equals(JobsOverviewHeaders.URL)) {
                target = new File(webOverviewDir, jobID + JSON_FILE_ENDING);
            } else if (path.equals("/joboverview")) { // legacy path
                LOG.info("ArchivedJsonEventHandler Migrating legacy archive {}", jobArchive);
                json = convertLegacyJobOverview(json);
                target = new File(webOverviewDir, jobID + JSON_FILE_ENDING);
            } else {
                // this implicitly writes into webJobDir
                target = new File(webDir, path + JSON_FILE_ENDING);
            }

            java.nio.file.Path parent = target.getParentFile().toPath();

            try {
                if (Files.exists(parent) && Files.isDirectory(parent)) {
                    // 忽略
                }else{
                    Files.createDirectories(parent);
                }
            } catch (FileAlreadyExistsException ignored) {
                // 如果目录已经存在，忽略异常
            }
            long opSt =  System.currentTimeMillis();
            java.nio.file.Path targetPath = target.toPath();

            // We overwrite existing files since this may be another attempt
            // at fetching this archive.
            // Existing files may be incomplete/corrupt.
            Files.deleteIfExists(targetPath);

            Files.createFile(targetPath);
            long writeSt =  System.currentTimeMillis();
            try (FileWriter fw = new FileWriter(target)) {
                fw.write(json);
                fw.flush();
            }
            
            LOG.info("ArchivedJsonEventHandler write file job size {}, jobId {}, archive {}, write cost {} ms.",json.length(), jobID, archive.getPath(), System.currentTimeMillis() - writeSt);
            LOG.info("ArchivedJsonEventHandler operator file jobId {}, archive {}, cost {} ms.", jobID, archive.getPath(), System.currentTimeMillis() - opSt);
        }catch (Exception e){
            LOG.error("ArchivedJsonEventHandler jobId {}, Failed to process archive {}.",jobID, jobArchive, e);
        }finally {
            LOG.info("ArchivedJsonEventHandler onEvent jobId {}, archive {}, cost {} ms.", jobID, archive.getPath(), System.currentTimeMillis() - st);
        }
    }
}
```

**ArchivedJsonEventWrapper**
```java
public class ArchivedJsonEventWrapper {
    private Path jobArchive;
    private String jobID;
    private ArchivedJson data;

    public ArchivedJsonEventWrapper() {
    }

    public ArchivedJsonEventWrapper(Path jobArchive, String jobID, ArchivedJson data) {
        this.jobArchive = jobArchive;
        this.jobID = jobID;
        this.data = data;
    }

    public ArchivedJson getData() {
        return data;
    }

    public void setData(ArchivedJson data) {
        this.data = data;
    }

    public Path getJobArchive() {
        return jobArchive;
    }

    public void setJobArchive(Path jobArchive) {
        this.jobArchive = jobArchive;
    }

    public String getJobID() {
        return jobID;
    }

    public void setJobID(String jobID) {
        this.jobID = jobID;
    }
}
```

### 2.processArchive 过滤要写入文件 path 
通过 `filterAndSendDisruptor()` 方法判断是否发送到 Disruptor 队列中去。 

>filterAndSendDisruptor() 是过滤的核心点，目前我只过滤了 `vertices` 文件夹下数据放入队列中。因为它的占比可达到 90% 以上。  

```java
private void processArchive(String jobID, Path jobArchive) throws IOException {
    long startTime = System.currentTimeMillis();
    Collection<ArchivedJson> archivedJsonCollection = FsJobArchivist.getArchivedJsons(jobArchive);
    if (archivedJsonCollection == null || archivedJsonCollection.isEmpty()){
        return;
    }
    LOG.info("processArchive archivedJsonCollection jobID: {}, size: {}", jobID,archivedJsonCollection.size());
    long step02 = 0;
    long step03 = 0;
    for (ArchivedJson archive : archivedJsonCollection) {
        try{
            String path = archive.getPath();
            String json = archive.getJson();

            long cc =  System.currentTimeMillis();
            if(filterAndSendDisruptor(jobArchive,jobID,archive)){
                continue;
            }
            step02 += System.currentTimeMillis() - cc;

            File target;
            if (path.equals(JobsOverviewHeaders.URL)) {
                target = new File(webOverviewDir, jobID + JSON_FILE_ENDING);
            } else if (path.equals("/joboverview")) { // legacy path
                LOG.info("Migrating legacy archive {}", jobArchive);
                json = convertLegacyJobOverview(json);
                target = new File(webOverviewDir, jobID + JSON_FILE_ENDING);
            } else {
                // this implicitly writes into webJobDir
                target = new File(webDir, path + JSON_FILE_ENDING);
            }

            java.nio.file.Path parent = target.getParentFile().toPath();

            try {
                if (Files.exists(parent) && Files.isDirectory(parent)) {
                    // 忽略
                }else{
                    Files.createDirectories(parent);
                }
            } catch (FileAlreadyExistsException ignored) {
                // 如果目录已经存在，忽略异常
            }
            long opSt =  System.currentTimeMillis();
            java.nio.file.Path targetPath = target.toPath();

            // We overwrite existing files since this may be another attempt
            // at fetching this archive.
            // Existing files may be incomplete/corrupt.
            long bb = System.currentTimeMillis();
            Files.deleteIfExists(targetPath);

            Files.createFile(targetPath);
//                long writeSt =  System.currentTimeMillis();
            try (FileWriter fw = new FileWriter(target)) {
                fw.write(json);
                fw.flush();
            }
            step03 += System.currentTimeMillis() - bb;

//                LOG.info("processArchive write file job size {}, jobId {}, archive {}, write cost {} ms.",json.length(), jobID, archive.getPath(), System.currentTimeMillis() - writeSt);
            LOG.info("processArchive operator file jobId {}, archive {}, cost {} ms.", jobID, archive.getPath(), System.currentTimeMillis() - opSt);
        }catch (Exception e){
            LOG.error("processArchive jobId {}, Failed to process archive {}.",jobID, jobArchive, e);
        }
    }
    LOG.info("processArchive jobId {}, archive {}, cost {} ms, step02: {}, step03: {}.", jobID, jobArchive, System.currentTimeMillis() - startTime,step02,step03);
}

    private boolean filterAndSendDisruptor(Path jobArchive,String jobID,ArchivedJson archivedJson){
        boolean filterResult = false;
        String path = archivedJson.getPath();
        if(path.contains("vertices")){
            filterResult = true;
            archivedJsonDisruptorManager.getEventProducer().onData(jobArchive,jobID,archivedJson);
        }
        return filterResult;
    }
```

### 3.添加 history server jvm 参数 
使用多线程，适当增加 jvm 堆大小，避免出现 OOM  

```
env.java.opts.historyserver: -Duser.timezone=GMT+08 -XX:+UseG1GC -Xms4g -Xmx4g
```


## 总结  
此次优化的核心思想是`懒加载`,我要的是什么，对于不着急的事晚点也没关系。在这个版本之前，我的另一个版本是直接在 processArchive()方法中的 for 增加个线程池来实现并行写入文件。从实践角度来看，并行确实增加了 file 写入磁盘的速度，但并不会发生质的提升，因为慢 十几分钟与 慢 二，三分钟是一个性质，若是线上问题，更希望的是立马看到 Exception Message，再分析，修复最后部署新任务。       

目前该版本优化已经跑了几天了，是符合期望的，后面要是有啥异常信息也可再继续讨论。     

