## Flink CDC 2.4 原理分析   

>flink-cdc-connectors version: 2.4.0, 随着版本迭代,FLIP-27仅可做参考，避免实际代码处理与该FLIP有较大不同。  

### Flink CDC 演进      
该篇幅不过多阐述，本人也是直接上手flink cdc 2.x,没有 1.x 实战经验，所以下面算是摘抄其他的内容：     
Flink CDC 1.x的底层是基于debezium来实现的，通过捕获变更数据（Change Data Capture）来实现从不同的数据源采集数据, 以MySQL CDC为例，`Flink CDC Connectors`项目设置`server-id`参数模拟自身是MySQL集群中的 **Slave**节点。从而读取Master Binlog数据。但在支持 全量+增量 模式下，为了保证 snapshot 阶段数据的一致性，通过对表加锁来实现，全局锁可能导致数据库被夯住，特别在表特别大的时候，仅在单并发处理下（1.x 只支持单并发），那加锁的时间出现不可评估， 通过其他Blog了解到 1.x 全量读取阶段不支持 checkpoint，那么 fail后只能重新读取。           
    
大伙可参考 `https://cwiki.apache.org/confluence/display/FLINK/FLIP-27%3A+Refactor+Source+Interface` 中的**Motivation** 章节，以下是ChatGPT直翻内容：    
```
**动机**    

此FLIP旨在解决当前流处理源接口（SourceFunction）的若干问题/缺陷，并同时统一批处理和流处理API之间的源接口。我们想要解决的缺陷或问题点包括：  

1. 当前分别为批处理和流处理执行实现了不同的源。 

2. "工作发现"（例如：切片、分区等）的逻辑和实际的"读取"数据逻辑在SourceFunction接口和DataStream API中是交织在一起的，导致了像Kafka和Kinesis源这样的复杂实现。   

3. 分区/碎片/切片在接口中并不明确。这使得以与源无关的方式实现某些功能变得困难，例如事件时间对齐、每个分区的水印、动态切片分配、工作窃取。例如，Kafka和Kinesis消费者都支持每个分区的水印，但在Flink 1.8.1中，只有Kinesis消费者支持事件时间对齐（选择性地从切片中读取，以确保我们在事件时间中均匀前进）。     

4. 检查点锁定由源函数"拥有"。实现必须确保在锁定下进行元素发射和状态更新。Flink没有办法优化它如何处理这个锁。            
    - 该锁并不是一个公平锁。在锁争用下，某些线程可能无法获得锁（例如检查点线程）。          
    - 这也妨碍了对操作员采用无锁的actor/邮箱式线程模型。            

5. 没有公共的构建模块，这意味着每个源自己都实现了一个复杂的线程模型。这使得实现和测试新的源变得困难，并增加了对现有源的贡献的门槛。         
```

>注意：`snapshot`阶段是指**未同步前DB阶段**     


Flink CDC 2.x 为了解决以上痛点，借鉴 Netflix DBLog 的无锁算法，并基于 FLIP-27 实战， 以达成以下目标：       
1.无锁  
2.全量阶段支持 水平扩展
3.全量阶段支持 checkpoint (断点续传)   

### Separating Work Discovery from Reading (工作节点发现从读取中分离出来)

源有两个主要组件：  
**1.SplitEnumerator**：发现并分配切片（文件、分区等）。    
**2.Reader**：从切片中实际读取数据。    

SplitEnumerator与旧的批处理源接口的功能相似，它创建并分配切片。它只运行一次，不并行运行（但未来如果需要，可以考虑并行化）。     
它可能运行在JobManager上或在TaskManager上的一个任务中（参见下文“在哪里运行Enumerator”）。   
**示例**：

- 在**文件源**中，SplitEnumerator列出所有文件（可能将它们细分为块/范围）。      
- 对于**Kafka源**，SplitEnumerator查找源应该从中读取的所有Kafka分区。   

Reader从分配的切片中读取数据。读取器包含了当前源接口的大部分功能。  
一些读取器可能会一个接一个地读取一系列有界的切片，而另一些可能会并行地读取多个（无界）切片。  

Enumerator和Reader之间的这种分离允许不同的枚举策略与切片读取器进行混合和匹配。例如，当前的Kafka连接器有不同的分区发现策略，这些策略与代码的其他部分交织在一起。有了新的接口，我们只需要一个切片读取器实现，并且可以有几个切片Enumerator用于不同的分区发现策略。     

通过这两个组件封装核心功能，主要的Source接口本身只是一个用于创建切片Enumerator和读取器的工厂。   

![flinkcdc2analyse01](images/flinkcdc2analyse01.png)    


下面，我们通过 MySQL CDC案例来分析Master-Worker的构成，这里特意提到MySQL是因为不同SQL所创建的接口实现类也不同。        

```sql
CREATE TABLE `yzhou_source_yzhou_test01`
(
    `id`      INT    NOT NULL COMMENT '',
    `name`    STRING NOT NULL COMMENT '',
    `address` STRING COMMENT '',
    PRIMARY KEY (id) NOT ENFORCED
)
    WITH
        (
        'connector' = 'mysql-cdc',
        'hostname' = '127.0.0.1',
        'port' = '3306',
        'username' = 'root',
        'password' = '12345678',
        'database-name' = 'yzhou_test',
        'table-name' = 'users',
        'server-id' = '5401',
        'scan.startup.mode' = 'initial'
        );

select * from yzhou_source_yzhou_test01;
```

#### Master 
![flinkcdc2analyse02](images/flinkcdc2analyse02.png) 

Flink 运行时会根据SQL的with的SPI参数来构造相关的实现类，可查看 SourceCoordinator 的构造方法中涉及变量 source (MySqlSource), 执行start() 会调用 `MySqlSource#createEnumerator()` 创建 MySqlSourceEnumerator, 根据上面介绍 `SplitEnumerator`主要负责`发现并分配切片`，所以不难发现 MySqlSourceEnumerator 的构造过程中，会根据`startupMode`参数来创建 不同的`MySqlSplitAssigner`接口实现类，当 startupMode=INITIAL 创建的是`MySqlHybridSplitAssigner`,反之是`MySqlBinlogSplitAssigner`。 

我想这里通过接口编程思维，已经不太难了解 `SplitEnumerator` + `MySqlSplitAssigner` 所具备的大致功能。 接下来我们除了分配分片，那SplitEnumerator又是如何处理Work 发现的。     

在`MySqlSourceEnumerator#getRegisteredReader()`方法中可得知，`readers`是从`SourceCoordinatorContext<SplitT> context` 对象下的 `ConcurrentMap<Integer, ReaderInfo> registeredReaders`集合中得到。 
```java
private int[] getRegisteredReader() {
    return this.context.registeredReaders().keySet().stream()
            .mapToInt(Integer::intValue)
            .toArray();
}
``` 

根据 `registeredReaders`的put()方法可知，只有在 `SourceCoordinator#handleReaderRegistrationEvent()`方法中调用了put()，而 xxxEvent()方法主要负责网络请求，同理，Worker中的Reader在启动时会发起 registration请求。        


#### Worker
![flinkcdc2analyse03](images/flinkcdc2analyse03.png)    

Reader注册的逻辑想对比较简单，我们直接看 `SourceOperator#open()`方法中，通过调用`registerReader()` 完成Reader register。        
```java
// Register the reader to the coordinator.
registerReader();
```

通过对Master-Worker结构的梳理，与 FLIP-27 class 关系基本一致。          

![flinkcdc2analyse04](images/flinkcdc2analyse04.png)        


### SplitEnumerator的分配切片（Chunks）   
该篇Blog开头提到 Flink CDC 1.x针对 `全量阶段`只能单并发运行，那么该章节主要来介绍Flink CDC 2.x对这块了哪些优化。        

在**Separating Work Discovery from Reading** 章节中介绍 `SplitEnumerator`与 `Reader`构建 Master-Worker结构，Flink CDC 2.x 对 snapshot阶段的数据按照一定规则进行分割，然后将任务拆分后，分散给`Reader`，让它们来分段读取。这真的是妥妥的**分布式调度中的分片任务**。     

![flinkcdc2analyse05](images/flinkcdc2analyse05.png)    

### 分割数据   

![flinkcdc2analyse06](images/flinkcdc2analyse06.png)    
`MySqlSnapshotSplitAssigner#open()`  
```java
@Override
public void open() {
    // 创建jdbc conn
    chunkSplitter.open();
    // 根据 sql 查找拆分表
    discoveryCaptureTables();
    // 发现新表
    captureNewlyAddedTables();
    // 启动异步拆分表
    startAsynchronouslySplit();
}
```
open()方法中，重点看 `startAsynchronouslySplit()`方法的执行, 忽略checkpoint相关的逻辑，来看 `MySqlSnapshotSplitAssigner#splitTable()`方法逻辑。     

```java
private void splitChunksForRemainingTables() {
    try {
        // restore from a checkpoint and start to split the table from the previous
        // checkpoint
        if (chunkSplitter.hasNextChunk()) {
            LOG.info(
                    "Start splitting remaining chunks for table {}",
                    chunkSplitter.getCurrentSplittingTableId());
            splitTable(chunkSplitter.getCurrentSplittingTableId());
        }

        // split the remaining tables
        for (TableId nextTable : remainingTables) {
            splitTable(nextTable);
        }
    } catch (Throwable e) {
        synchronized (lock) {
            if (uncaughtSplitterException == null) {
                uncaughtSplitterException = e;
            } else {
                uncaughtSplitterException.addSuppressed(e);
            }
            // Release the potential waiting getNext() call
            lock.notify();
        }
    }
}
```

---

`MySqlSnapshotSplitAssigner#splitTable()`,在do...while循环中使用了`synchronized (lock)`,可以确定是 `MySqlSourceEnumerator` 串行拆分 Table。 

```java
private void splitTable(TableId nextTable) {
    LOG.info("Start splitting table {} into chunks...", nextTable);
    long start = System.currentTimeMillis();
    int chunkNum = 0;
    boolean hasRecordSchema = false;
    // split the given table into chunks (snapshot splits)
    do {
        synchronized (lock) {
            List<MySqlSnapshotSplit> splits;
            try {
                splits = chunkSplitter.splitChunks(partition, nextTable);
            } catch (Exception e) {
                throw new IllegalStateException(
                        "Error when splitting chunks for " + nextTable, e);
            }

            if (!hasRecordSchema && !splits.isEmpty()) {
                hasRecordSchema = true;
                final Map<TableId, TableChanges.TableChange> tableSchema = new HashMap<>();
                tableSchema.putAll(splits.iterator().next().getTableSchemas());
                tableSchemas.putAll(tableSchema);
            }
            final List<MySqlSchemalessSnapshotSplit> schemaLessSnapshotSplits =
                    splits.stream()
                            .map(MySqlSnapshotSplit::toSchemalessSnapshotSplit)
                            .collect(Collectors.toList());
            chunkNum += splits.size();
            remainingSplits.addAll(schemaLessSnapshotSplits);
            if (!chunkSplitter.hasNextChunk()) {
                remainingTables.remove(nextTable);
            }
            lock.notify();
        }
    } while (chunkSplitter.hasNextChunk());
    long end = System.currentTimeMillis();
    LOG.info(
            "Split table {} into {} chunks, time cost: {}ms.",
            nextTable,
            chunkNum,
            end - start);
}
```

`MySqlChunkSplitter#splitChunks()`, 
```java
@Override
public List<MySqlSnapshotSplit> splitChunks(MySqlPartition partition, TableId tableId)
        throws Exception {
    if (!hasNextChunk()) {
        analyzeTable(partition, tableId);
        Optional<List<MySqlSnapshotSplit>> evenlySplitChunks =
                trySplitAllEvenlySizedChunks(partition, tableId);
        if (evenlySplitChunks.isPresent()) {
            return evenlySplitChunks.get();
        } else {
            synchronized (lock) {
                this.currentSplittingTableId = tableId;
                this.nextChunkStart = ChunkSplitterState.ChunkBound.START_BOUND;
                this.nextChunkId = 0;
                return Collections.singletonList(
                        splitOneUnevenlySizedChunk(partition, tableId));
            }
        }
    } else {
        Preconditions.checkState(
                currentSplittingTableId.equals(tableId),
                "Can not split a new table before the previous table splitting finish.");
        if (currentSplittingTable == null) {
            analyzeTable(partition, currentSplittingTableId);
        }
        synchronized (lock) {
            return Collections.singletonList(splitOneUnevenlySizedChunk(partition, tableId));
        }
    }
}
``` 

---

`MySqlChunkSplitter#analyzeTable()`                   
![flinkcdc2analyse10](images/flinkcdc2analyse10.png)    

currentSplittingTable是表的schema信息，     

splitColumn是chunk拆分列， 需特别 若有主键表，则无需指定 `scan.incremental.snapshot.chunk.key-column`,仅支持从主键中选择一列（下标为0的列），在无主键表，该`scan.incremental.snapshot.chunk.key-column`字段必填，并且选择的列必须是非空类型（NOT NULL）。        

splitTyp拆分类型是 ROW,         

minMaxOfSplitColumn通过 SplitColumn，查询最小，最大值SELECT MIN(%s), MAX(%s) FROM %s, 示例sql： 
```sql
SELECT MIN(`id`), MAX(`id`) FROM `yzhou_test`.`users`;
```
如果主键是字符串，那么按照`字典顺序`排序，

approximateRowCnt是获取行数，示例sql：      
```sql
SHOW TABLE STATUS LIKE 'users'; 
```

![flinkcdc2analyse08](images/flinkcdc2analyse08.png)    

```java
/** Analyze the meta information for given table. */
private void analyzeTable(MySqlPartition partition, TableId tableId) {
    try {
        currentSplittingTable =
                mySqlSchema.getTableSchema(partition, jdbcConnection, tableId).getTable();
        splitColumn =
                ChunkUtils.getChunkKeyColumn(
                        currentSplittingTable, sourceConfig.getChunkKeyColumns());
        splitType = ChunkUtils.getChunkKeyColumnType(splitColumn);
        minMaxOfSplitColumn = queryMinMax(jdbcConnection, tableId, splitColumn.name());
        approximateRowCnt = queryApproximateRowCnt(jdbcConnection, tableId);
    } catch (Exception e) {
        throw new RuntimeException("Fail to analyze table in chunk splitter.", e);
    }
}
```

`MySqlChunkSplitter#trySplitAllEvenlySizedChunks()` 
![flinkcdc2analyse09](images/flinkcdc2analyse09.png)    
chunkSize等于`scan.incremental.snapshot.chunk.size`，默认大小是8096,这里的单位是条数。          

示例，若A表有3数据，将 scan.incremental.snapshot.chunk.size 设置为2 ，我们看 chunks的分割列表情况 ：    
![flinkcdc2analyse11](images/flinkcdc2analyse11.png)        

```java
private Optional<List<MySqlSnapshotSplit>> trySplitAllEvenlySizedChunks(
        MySqlPartition partition, TableId tableId) {
    LOG.debug("Try evenly splitting table {} into chunks", tableId);
    final Object min = minMaxOfSplitColumn[0];
    final Object max = minMaxOfSplitColumn[1];
    if (min == null || max == null || min.equals(max)) {
        // empty table, or only one row, return full table scan as a chunk
        return Optional.of(
                generateSplits(
                        partition, tableId, Collections.singletonList(ChunkRange.all())));
    }

    final int chunkSize = sourceConfig.getSplitSize();
    final int dynamicChunkSize =
            getDynamicChunkSize(tableId, splitColumn, min, max, chunkSize, approximateRowCnt);
    if (dynamicChunkSize != -1) {
        LOG.debug("finish evenly splitting table {} into chunks", tableId);
        List<ChunkRange> chunks =
                splitEvenlySizedChunks(
                        tableId, min, max, approximateRowCnt, chunkSize, dynamicChunkSize);
        return Optional.of(generateSplits(partition, tableId, chunks));
    } else {
        LOG.debug("beginning unevenly splitting table {} into chunks", tableId);
        return Optional.empty();
    }
}
```

---

`MySqlChunkSplitter#getDynamicChunkSize()`, 
isEvenlySplitColumn()方法会判断 splitColumn 是否是均匀分布，目前仅支持数值类型为均匀分布：BIGINT，INTEGER，DECIMAL。 若是其他则是非均匀分布，直接返回 -1, 此处需明确一点，并不都是数值类型都是均匀分布，还需判断分布因子的范围区间是否在`distributionFactorUpper` 和 `distributionFactorLower` 之间。   
distributionFactorUpper 取决于 `chunk-key.even-distribution.factor.upper-bound` 参数  
distributionFactorLower 取决于 `chunk-key.even-distribution.factor.lower-bound` 参数      
distributionFactor 分布式因子，会根据 公式计算 分布式因子 factor = (max - min + 1) / rowCount，若表的行数是空时，则 返回 Double.MAX_VALUE       
dataIsEvenlyDistributed 判断 分布因子 distributionFactor 是否在`distributionFactorUpper` 和 `distributionFactorLower` 之间      
如果 分布因子代表的是均匀分布，那么最小值取整处理，chunkSize 至少为 1。         

>注意，dynamicChunkSize 不要与 MySqlChunkSplitter#trySplitAllEvenlySizedChunks()的 chunkSize 混淆，后续会在拆分表时用到。   

```java
private int getDynamicChunkSize(
        TableId tableId,
        Column splitColumn,
        Object min,
        Object max,
        int chunkSize,
        long approximateRowCnt) {
    if (!isEvenlySplitColumn(splitColumn)) {
        return -1;
    }
    final double distributionFactorUpper = sourceConfig.getDistributionFactorUpper();
    final double distributionFactorLower = sourceConfig.getDistributionFactorLower();
    double distributionFactor =
            calculateDistributionFactor(tableId, min, max, approximateRowCnt);
    boolean dataIsEvenlyDistributed =
            doubleCompare(distributionFactor, distributionFactorLower) >= 0
                    && doubleCompare(distributionFactor, distributionFactorUpper) <= 0;
    LOG.info(
            "The actual distribution factor for table {} is {}, the lower bound of evenly distribution factor is {}, the upper bound of evenly distribution factor is {}",
            tableId,
            distributionFactor,
            distributionFactorLower,
            distributionFactorUpper);
    if (dataIsEvenlyDistributed) {
        // the minimum dynamic chunk size is at least 1
        return Math.max((int) (distributionFactor * chunkSize), 1);
    }
    return -1;
}
```

---

`MySqlChunkSplitter#splitEvenlySizedChunks()` ，该方法主要负责是 根据一定间距规则，拆分表，返回表的区间集合。       
**if (approximateRowCnt <= chunkSize)** 如果默认的chunkSize >= 表的条数，则将整张表当作一个chunk 返回 
**while (ObjectUtils.compare(chunkEnd, max) <= 0)** 以 dynamicChunkSize 为区间间隔，从第一个 区间 [min,min+dynamicChunkSize]  开始 一直 chunkEnd > max， 当while不成立的时候，chunkStart是条件成立的最后1个,再处于余数即可。              

```java
@VisibleForTesting
public List<ChunkRange> splitEvenlySizedChunks(
        TableId tableId,
        Object min,
        Object max,
        long approximateRowCnt,
        int chunkSize,
        int dynamicChunkSize) {
    LOG.info(
            "Use evenly-sized chunk optimization for table {}, the approximate row count is {}, the chunk size is {}, the dynamic chunk size is {}",
            tableId,
            approximateRowCnt,
            chunkSize,
            dynamicChunkSize);
    if (approximateRowCnt <= chunkSize) {
        // there is no more than one chunk, return full table as a chunk
        return Collections.singletonList(ChunkRange.all());
    }

    final List<ChunkRange> splits = new ArrayList<>();
    Object chunkStart = null;
    Object chunkEnd = ObjectUtils.plus(min, dynamicChunkSize);
    while (ObjectUtils.compare(chunkEnd, max) <= 0) {
        splits.add(ChunkRange.of(chunkStart, chunkEnd));
        chunkStart = chunkEnd;
        try {
            chunkEnd = ObjectUtils.plus(chunkEnd, dynamicChunkSize);
        } catch (ArithmeticException e) {
            // Stop chunk split to avoid dead loop when number overflows.
            break;
        }
    }
    // add the ending split
    splits.add(ChunkRange.of(chunkStart, null));
    return splits;
}
```

---

现在回到 `MySqlChunkSplitter#splitChunks()` 通过上面的 `trySplitAllEvenlySizedChunks()` 可知道 split 的规则，若是均匀分布则直接返回分割段集合`List<MySqlSnapshotSplit>` ,若是非均匀分布，那又如何处理呢？       

```java
// splitChunks()的部分片段, 处理非均匀分布场景  
synchronized (lock) {
    this.currentSplittingTableId = tableId;
    this.nextChunkStart = ChunkSplitterState.ChunkBound.START_BOUND;
    this.nextChunkId = 0;
    return Collections.singletonList(
            splitOneUnevenlySizedChunk(partition, tableId));
}
``` 

---

`MySqlChunkSplitter#splitOneUnevenlySizedChunk()` 正如它的方法名字那样，每次只分割1个chunk， 它主要负责处理非均匀分布，我们来了解在它的处理逻辑，   
![flinkcdc2analyse12](images/flinkcdc2analyse12.png)

1.首先 获取下一个 chunkEnd，它的sql示例： SELECT MAX(`name`) FROM (SELECT `name` FROM `yzhou_test`.`users` WHERE `name` >= ? ORDER BY `name` ASC LIMIT 8096) AS T，     
2.如果 如果chunkStart = 下一个chunkEnd，则表示 当前Chunk已分配结束，接下来使用 SELECT MIN(%s) FROM %s WHERE %s > ?， 将下一段Chunk的min 作为 chunkstart 再执行 上面的sql。      
这里特别注意， 查询下一次的chunkEnd Max(`name`) 里面总是包含 limit chunksize， 所以 SELECT MIN(%s) FROM %s WHERE %s > ? 求 最小值，能跳到下一个 limit 分页。    
3.如果 下一个chunkEnd >=max,表示超过边界值 则 返回 null,    
4.返回 chunk    

因为查询下一个 chunkEnd的SQL 总是包含Limit chunksize，所以 在非均匀分布的情况下，是使用limit 来保障数据均匀分布。       

>但这里有个issue，请大家mark一下 ：https://github.com/ververica/flink-cdc-connectors/issues/2489 。  后续再讨论 MySQL不同编码在的排序方式    

```java
/** Generates one snapshot split (chunk) for the give table path. */
private MySqlSnapshotSplit splitOneUnevenlySizedChunk(MySqlPartition partition, TableId tableId)
        throws SQLException {
    final int chunkSize = sourceConfig.getSplitSize();
    final Object chunkStartVal = nextChunkStart.getValue();
    LOG.info(
            "Use unevenly-sized chunks for table {}, the chunk size is {} from {}",
            tableId,
            chunkSize,
            nextChunkStart == ChunkSplitterState.ChunkBound.START_BOUND
                    ? "null"
                    : chunkStartVal.toString());
    // we start from [null, min + chunk_size) and avoid [null, min)
    Object chunkEnd =
            nextChunkEnd(
                    jdbcConnection,
                    nextChunkStart == ChunkSplitterState.ChunkBound.START_BOUND
                            ? minMaxOfSplitColumn[0]
                            : chunkStartVal,
                    tableId,
                    splitColumn.name(),
                    minMaxOfSplitColumn[1],
                    chunkSize);
    // may sleep a while to avoid DDOS on MySQL server
    maySleep(nextChunkId, tableId);
    if (chunkEnd != null && ObjectUtils.compare(chunkEnd, minMaxOfSplitColumn[1]) <= 0) {
        nextChunkStart = ChunkSplitterState.ChunkBound.middleOf(chunkEnd);
        return createSnapshotSplit(
                jdbcConnection,
                partition,
                tableId,
                nextChunkId++,
                splitType,
                chunkStartVal,
                chunkEnd);
    } else {
        currentSplittingTableId = null;
        nextChunkStart = ChunkSplitterState.ChunkBound.END_BOUND;
        return createSnapshotSplit(
                jdbcConnection,
                partition,
                tableId,
                nextChunkId++,
                splitType,
                chunkStartVal,
                null);
    }
}
```

---

在介绍 splitOneUnevenlySizedChunk() 提到 它每次只拉取1个chunk，与均匀分布不同，均匀分布是直接返回 List<Chunk>集合，那非均匀分布分割table又是 如何停止 ？        

我们现在回到 `MySqlSnapshotSplitAssigner#splitTable()` 里面用do while(hasNextChunk())来遍历处理，返回chunk集合， 可这里分两种split， 均匀分布和非均匀， 其中针对 均匀分布 直接遍历1次即可完成chunk集群， 而对 非均匀分布 是每次遍历只分割1个chunk， 我们来看 hasNextChunk()的判断逻辑：             
```
@Override
public boolean hasNextChunk() {
    return currentSplittingTableId != null;
}
```

![flinkcdc2analyse13](images/flinkcdc2analyse13.png)    

我们再把思绪拉回到 `splitChunks()`方法，当我们根据splitType得到是非均匀分布时，才会调用以下代码：       
```java
// splitChunks()的部分片段, 处理非均匀分布场景  
synchronized (lock) {
    this.currentSplittingTableId = tableId;
    this.nextChunkStart = ChunkSplitterState.ChunkBound.START_BOUND;
    this.nextChunkId = 0;
    return Collections.singletonList(
            splitOneUnevenlySizedChunk(partition, tableId));
```

所以 当对非均匀分布的table进行分割时，会增加一个字段进行标识 this.currentSplittingTableId = tableId; 所以 当每次分割结束时，判断是否结束，若结束重新将 currentSplittingTableId = null； 你可回头再看 splitOneUnevenlySizedChunk() 但达到边界值，又重新赋值后再返回：        
```java
// splitOneUnevenlySizedChunk 片段 
currentSplittingTableId = null;
nextChunkStart = ChunkSplitterState.ChunkBound.END_BOUND;
return createSnapshotSplit(
        jdbcConnection,
        partition,
        tableId,
        nextChunkId++,
        splitType,
        chunkStartVal,
        null);
```

以上 就完成了对Table的分割，将chunk集合存储到 `MySqlSnapshotSplitAssigner.List<MySqlSchemalessSnapshotSplit> remainingSplits;`      



### 分配任务       
上面分析，已经帮我们完成2个环节 
1.获取已注册的Readers
2.获取待分配的Chunks    

此处暂时略， 后续再补充     





### Reader 处理数据 
>该章节主要讲解 Reader处理全量+增量的逻辑         

`MySqlSource`是 flink-connector-mysql-cdc的核心类，在`Separating Work Discovery from Reading`章节中创建`MySqlSourceEnumerator`的时候提到它 ，`MySqlSourceReader`(work)调用 `MySqlSource#createReader()`方法创建的。 下面我们看下**Reader处理入口**的方法调用逻辑：  

![flinkcdc2Reader01](images/flinkcdc2Reader01.png)  

从任务队列中领取SplitTask任务，调用线程run()执行 `MySqlSplitReader#fetch()`方法， 这里大家先忽略，SplitTask多线程模型，我自己也没深入了解，至少这不影响分析数据读取。 fetch()会调用 `pollSplitRecords()`, 该方法非常重要。  
我们还是拿上面的sql案例，来阅读源码：   
```sql
CREATE TABLE `yzhou_source_yzhou_test01`
(
    `id`      INT    NOT NULL COMMENT '',
    `name`    STRING NOT NULL COMMENT '',
    `address` STRING COMMENT '',
    PRIMARY KEY (id) NOT ENFORCED
)
    WITH
        (
        'connector' = 'mysql-cdc',
        'hostname' = '127.0.0.1',
        'port' = '3306',
        'username' = 'root',
        'password' = '12345678',
        'database-name' = 'yzhou_test',
        'table-name' = 'users',
        'server-id' = '5401',
        'scan.startup.mode' = 'initial'
        );

select * from yzhou_source_yzhou_test01;
```

`MySqlSplitReader#pollSplitRecords()`   
```java
private MySqlRecords pollSplitRecords() throws InterruptedException {
    Iterator<SourceRecords> dataIt;
    if (currentReader == null) {
        // (1) Reads binlog split firstly and then read snapshot split
        if (binlogSplits.size() > 0) {
            // the binlog split may come from:
            // (a) the initial binlog split
            // (b) added back binlog-split in newly added table process
            MySqlSplit nextSplit = binlogSplits.poll();
            currentSplitId = nextSplit.splitId();
            currentReader = getBinlogSplitReader();
            currentReader.submitSplit(nextSplit);
        } else if (snapshotSplits.size() > 0) {
            MySqlSplit nextSplit = snapshotSplits.poll();
            currentSplitId = nextSplit.splitId();
            currentReader = getSnapshotSplitReader();
            currentReader.submitSplit(nextSplit);
        } else {
            LOG.info("No available split to read.");
        }
        dataIt = currentReader.pollSplitRecords();
        return dataIt == null ? finishedSplit() : forRecords(dataIt);
    } else if (currentReader instanceof SnapshotSplitReader) {
        // (2) try to switch to binlog split reading util current snapshot split finished
        dataIt = currentReader.pollSplitRecords();
        if (dataIt != null) {
            // first fetch data of snapshot split, return and emit the records of snapshot split
            MySqlRecords records;
            if (context.isHasAssignedBinlogSplit()) {
                records = forNewAddedTableFinishedSplit(currentSplitId, dataIt);
                closeSnapshotReader();
                closeBinlogReader();
            } else {
                records = forRecords(dataIt);
                MySqlSplit nextSplit = snapshotSplits.poll();
                if (nextSplit != null) {
                    currentSplitId = nextSplit.splitId();
                    currentReader.submitSplit(nextSplit);
                } else {
                    closeSnapshotReader();
                }
            }
            return records;
        } else {
            return finishedSplit();
        }
    } else if (currentReader instanceof BinlogSplitReader) {
        // (3) switch to snapshot split reading if there are newly added snapshot splits
        dataIt = currentReader.pollSplitRecords();
        if (dataIt != null) {
            // try to switch to read snapshot split if there are new added snapshot
            MySqlSplit nextSplit = snapshotSplits.poll();
            if (nextSplit != null) {
                closeBinlogReader();
                LOG.info("It's turn to switch next fetch reader to snapshot split reader");
                currentSplitId = nextSplit.splitId();
                currentReader = getSnapshotSplitReader();
                currentReader.submitSplit(nextSplit);
            }
            return MySqlRecords.forBinlogRecords(BINLOG_SPLIT_ID, dataIt);
        } else {
            // null will be returned after receiving suspend binlog event
            // finish current binlog split reading
            closeBinlogReader();
            return finishedSplit();
        }
    } else {
        throw new IllegalStateException("Unsupported reader type.");
    }
}
```

当我们将添加 `'scan.startup.mode' = 'initial'`参数，Flink CDC会先进行全量读取，所以 会先进去 `else if (snapshotSplits.size() > 0) {` 阶段， 接着处理以下内容：  
```java
MySqlSplit nextSplit = snapshotSplits.poll();
currentSplitId = nextSplit.splitId();
currentReader = getSnapshotSplitReader();
currentReader.submitSplit(nextSplit);
```
读取剩余的Chunk（nextSplit），getSnapshotSplitReader() 会创建2个非常重要的对象，1.StatefulTaskContext statefulTaskContext,2.SnapshotSplitReader。具体作用会在后面介绍 。    

>注意，请一定记住 MySqlSplitReader针对不同阶段，会创建不同阶段的Reader对象来进行数据读取，不要弄混淆。 若是binlog阶段，创建的是 `BinlogSplitReader`, 若是snapshot阶段，创建的是 `SnapshotSplitReader`
![flinkcdc2Reader02](images/flinkcdc2Reader02.png)  

>目前处理全量读取阶段   
在`SnapshotSplitReader#submitSplit()` 使用单线程池来异步执行全量阶段的读取，并且将读取逻辑封装到`MySqlSnapshotSplitReadTask`(类名长的差不多，注意后几位，例如Task，Reader), 所以接下来的重点就是看`MySqlSnapshotSplitReadTask`  
```java
// ...
executorService.execute(
        () -> {
            try {
                currentTaskRunning = true;
                // execute snapshot read task
                final SnapshotSplitChangeEventSourceContextImpl sourceContext =
                        new SnapshotSplitChangeEventSourceContextImpl();
                SnapshotResult<MySqlOffsetContext> snapshotResult =
                        splitSnapshotReadTask.execute(
                                sourceContext,
                                statefulTaskContext.getMySqlPartition(),
                                statefulTaskContext.getOffsetContext());
// ...
```

**MySqlSnapshotSplitReadTask**  

`MySqlSnapshotSplitReadTask#doExecute()`        

```java
@Override
protected SnapshotResult<MySqlOffsetContext> doExecute(
        ChangeEventSourceContext context,
        MySqlOffsetContext previousOffset,
        SnapshotContext<MySqlPartition, MySqlOffsetContext> snapshotContext,
        SnapshottingTask snapshottingTask)
        throws Exception {
    final MySqlSnapshotContext ctx = (MySqlSnapshotContext) snapshotContext;
    ctx.offset = previousOffset;
    final SignalEventDispatcher signalEventDispatcher =
            new SignalEventDispatcher(
                    previousOffset.getOffset(),
                    topicSelector.topicNameFor(snapshotSplit.getTableId()),
                    dispatcher.getQueue());

    final BinlogOffset lowWatermark = currentBinlogOffset(jdbcConnection);
    LOG.info(
            "Snapshot step 1 - Determining low watermark {} for split {}",
            lowWatermark,
            snapshotSplit);
    ((SnapshotSplitReader.SnapshotSplitChangeEventSourceContextImpl) (context))
            .setLowWatermark(lowWatermark);
    signalEventDispatcher.dispatchWatermarkEvent(
            snapshotSplit, lowWatermark, SignalEventDispatcher.WatermarkKind.LOW);

    LOG.info("Snapshot step 2 - Snapshotting data");
    createDataEvents(ctx, snapshotSplit.getTableId());

    final BinlogOffset highWatermark = currentBinlogOffset(jdbcConnection);
    LOG.info(
            "Snapshot step 3 - Determining high watermark {} for split {}",
            highWatermark,
            snapshotSplit);
    signalEventDispatcher.dispatchWatermarkEvent(
            snapshotSplit, highWatermark, SignalEventDispatcher.WatermarkKind.HIGH);
    ((SnapshotSplitReader.SnapshotSplitChangeEventSourceContextImpl) (context))
            .setHighWatermark(highWatermark);

    return SnapshotResult.completed(ctx.offset);
}
```


`MySqlSnapshotSplitReadTask#doExecute()`是 Reader 执行分配的Chunk任务入口。 从 doExecute() 方法log打印步骤，分为3步：   

**step01.**通过`SHOW MASTER STATUS` 查询当前的binlog offset , 并标记为 `lowWatermark`        
查询 binlog offset sql执行结果：    

![flinkcdc2analyse14](images/flinkcdc2analyse14.png)    

`signalEventDispatcher.dispatchWatermarkEvent()`, 会将 lowWatermark 封装成 `SourceRecord` 对象，然后放入 `StatefulTaskContext.EventDispatcherImpl<TableId> dispatcher.queue`队列中 (后续会再介绍Queue)。  


>此处，需提前介绍下 EventDispatcherImpl<TableId> dispatcher.queue的构造，因为后续会涉及到 queue的读写。在后续 处理Queue的Record 会涉及到 `SnapshotSplitReader#pollSplitRecords()`,该方法通过注释的方式告诉我们，queue的内部数据存放顺序： 
```
// data input: [low watermark event][snapshot events][high watermark event][binlog
// events][binlog-end event]
// data output: [low watermark event][normalized events][high watermark event]
``` 

所以根据上面的queue，我们已经通过 step01，将 lowWatermark 放入queue中       
![flinkcdc2analyse15](images/flinkcdc2analyse15.png)    


**step02.** 读取 Snapshot data  
```java
LOG.info("Snapshot step 2 - Snapshotting data");
createDataEvents(ctx, snapshotSplit.getTableId());
```
先按照chunk边界通过jdbc查出数据，while (rs.next())遍历 ResultSet集合，每条数据转换成Object数组 Row，在通过 debezium的 `SnapshotChangeRecordEmitter` + `BufferingSnapshotChangeRecordReceiver` 读取表中的快照数据。 请特别注意 下面逻辑：    
```java
changeRecordEmitter.emitChangeRecords(dataCollectionSchema, new Receiver<P>() {

    @Override
    public void changeRecord(P partition,
                                DataCollectionSchema schema,
                                Operation operation,
                                Object key, Struct value,
                                OffsetContext offset,
                                ConnectHeaders headers)
            throws InterruptedException {
        eventListener.onEvent(partition, dataCollectionSchema.id(), offset, key, value, operation);
        receiver.changeRecord(partition, dataCollectionSchema, operation, key, value, offset, headers);
    }
});
```
这与CDC 数据变更不同，此时 eventListener 定义监听事件类型，receiver来处理监听事件的数据。 BufferingSnapshotChangeRecordReceiver只会处理已存在表中的数据，对数据变更事件不做任何处理。    

>`SnapshotChangeRecordEmitter` 是 Debezium 内部的一个组件，用于处理数据库的初始快照事件。当你启动一个 Debezium 连接器并需要从数据库的当前状态开始捕获变更，而不是从某个历史点开始，Debezium 会执行一个称为 "快照" 的操作。这个快照操作会读取数据库的所有行（或配置的表的所有行）并生成对应的事件。
这里是该组件在 Debezium 中的用途：
1. **初始快照生成**：当你第一次启动 Debezium 连接器，或者当连接器配置为执行快照操作时，`SnapshotChangeRecordEmitter` 被用来生成每一行数据的记录事件。这些事件代表了表的当前状态。   
2. **处理大量数据**：生成快照可能涉及到处理数据库中的大量数据。`SnapshotChangeRecordEmitter` 被设计成可扩展的，能够高效地处理大数据集。 
3. **数据变更记录的一致性**：在快照过程中，`SnapshotChangeRecordEmitter` 会确保产生的记录与正在发生的数据库变更（通过 binlog 或其他机制）保持一致。这意味着，即使在快照过程中数据库状态发生了变更   


此时`receiver.changeRecord`会将 表中数据 写入 queue中。 

![flinkcdc2Reader03](images/flinkcdc2Reader03.png)  


**step03.**通过`SHOW MASTER STATUS` 查询当前的binlog offset , 并标记为 `highWatermark`  


目前queue已经存放的数据为： 
![flinkcdc2Reader04](images/flinkcdc2Reader04.png)  

我们现在回到 `SnapshotSplitReader#submitSplit()`， 当snapshot阶段读取完后，会比较 context的lowwatermark,highwatermark的大小，若相等则不存在 binlog读取，直接在queue追加一个 BINLOG_END Record。 
![flinkcdc2Reader05](images/flinkcdc2Reader05.png)      

```java
SnapshotResult<MySqlOffsetContext> snapshotResult =
        splitSnapshotReadTask.execute(
                sourceContext,
                statefulTaskContext.getMySqlPartition(),
                statefulTaskContext.getOffsetContext());

final MySqlBinlogSplit backfillBinlogSplit =
        createBackfillBinlogSplit(sourceContext);
// optimization that skip the binlog read when the low watermark equals high
// watermark
final boolean binlogBackfillRequired =
        backfillBinlogSplit
                .getEndingOffset()
                .isAfter(backfillBinlogSplit.getStartingOffset());
if (!binlogBackfillRequired) {
    dispatchBinlogEndEvent(backfillBinlogSplit);
    currentTaskRunning = false;
    return;
}

// execute binlog read task
if (snapshotResult.isCompletedOrSkipped()) {
    final MySqlBinlogSplitReadTask backfillBinlogReadTask =
            createBackfillBinlogReadTask(backfillBinlogSplit);
    final MySqlOffsetContext.Loader loader =
            new MySqlOffsetContext.Loader(
                    statefulTaskContext.getConnectorConfig());
    final MySqlOffsetContext mySqlOffsetContext =
            loader.load(
                    backfillBinlogSplit.getStartingOffset().getOffset());

    backfillBinlogReadTask.execute(
            new SnapshotBinlogSplitChangeEventSourceContextImpl(),
            statefulTaskContext.getMySqlPartition(),
            mySqlOffsetContext);
} else {
    setReadException(
            new IllegalStateException(
                    String.format(
                            "Read snapshot for mysql split %s fail",
                            currentSnapshotSplit)));
}
```

则，读取binlog数据再放入queue中。
![flinkcdc2Reader06](images/flinkcdc2Reader06.png)   


现在回到`MySqlSplitReader#pollSplitRecords()` 
```java
if (binlogSplits.size() > 0) {
    // the binlog split may come from:
    // (a) the initial binlog split
    // (b) added back binlog-split in newly added table process
    MySqlSplit nextSplit = binlogSplits.poll();
    currentSplitId = nextSplit.splitId();
    currentReader = getBinlogSplitReader();
    currentReader.submitSplit(nextSplit);
} else if (snapshotSplits.size() > 0) {
    MySqlSplit nextSplit = snapshotSplits.poll();
    currentSplitId = nextSplit.splitId();
    currentReader = getSnapshotSplitReader();
    currentReader.submitSplit(nextSplit);
} else {
    LOG.info("No available split to read.");
}
dataIt = currentReader.pollSplitRecords();
```

currentReader.pollSplitRecords()方法 如何是 全量 + 增量模式， 当读完全量时，会直接放入缓存中 `Map<Struct, List<SourceRecord>> snapshotRecords = new HashMap<>();`, 当读到binlog阶段，会进行upset操作，只有 splitKey在区间内，才合并， 具体根据不同操作符，对缓存中的数据进行合并处理。  

















refer       
1.https://cwiki.apache.org/confluence/display/FLINK/FLIP-27%3A+Refactor+Source+Interface    
2.https://debezium.io/       
3.https://zhjwpku.com/2022/01/16/flink-cdc-2-0-analysis.html    


