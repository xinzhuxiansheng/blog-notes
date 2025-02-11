# JAVA - RocksDB - 探索 Queue on RocksDB 

>在`存储`方向，2025年怎么夸 `RocksDB` 都不为过，它无处不在 :）       

## 引言   
>若大家对 `RocksDB`不太了解，可访问 `RocksDB Wiki`(https://github.com/facebook/rocksdb/wiki), 博主想找 RocksDB 的中文书太难啦，只好慢慢啃。              

`RocksDB`被广泛用于`存储`系统中，它的优势，可在下面列举的两个案例中感受一波：         
* RIP-73，Pop Consumption Improvement Based on RocksDB （https://github.com/apache/rocketmq/wiki/%5BRIP%E2%80%9073%5D-Pop-Consumption-Improvement-Based-on-RocksDB）   
![queueonrocksdb01](http://img.xinzhuxiansheng.com/blogimgs/java/queueonrocksdb02.png)   

对于 `POP Consumer`为了解决 `Consumer Rebalance 给业务服务带来的不稳定性`以及`一对一队列占用无法扩展并发性`，这块博主暂时还没有跟进过细节，就不过多阐述，以免误导大家。     

重点可以关注 RocketMQ `develop`分支下的 `PopConsumerRocksdbStore` https://github.com/apache/rocketmq/blob/develop/broker/src/main/java/org/apache/rocketmq/broker/pop/PopConsumerRocksdbStore.java  

* RIP-75，Supports timer message on RocksDB （https://github.com/apache/rocketmq/issues/9141）      
![queueonrocksdb03](http://img.xinzhuxiansheng.com/blogimgs/java/queueonrocksdb03.png)     

定时消息和延迟消息本质是一样的，都是服务端根据消息设置的定时时间在某一固定时刻将消息投递给消费者消费。在
RocketMQ SUMMIT 2022大会上，小米分享过`任意延迟消息`的功能扩展。  
![queueonrocksdb04](http://img.xinzhuxiansheng.com/blogimgs/java/queueonrocksdb04.png)        

>以上案例通过 RocksDB 优化存储问题。RocksDB 不得不学呀 :)          

>在之前的两篇 Blog `JAVA - Memory-mapped I/O - 探索构造 BigArray 读写 File 篇` 和 `JAVA - Memory-mapped I/O - 探索給 BigArray 加锁` 中，通过直接读写文件实现数据的存储和查询，那现在如果用 RocksDB 又是如何呢？      
![queueonrocksdb01](http://img.xinzhuxiansheng.com/blogimgs/java/queueonrocksdb01.png)       

>在 RocksDB 的 WIKI也给出了 `Implement Queue Service Using RocksDB` 思路 (https://github.com/facebook/rocksdb/wiki/Implement-Queue-Service-Using-RocksDB)   
 
## Queue on RocksDB     
先介绍下`整体架构`：       
![queueonrocksdb05](http://img.xinzhuxiansheng.com/blogimgs/java/queueonrocksdb05.png)      

RocksDB is `a storage engine with key/value interface`, where keys and values are arbitrary byte streams. It is a C++ library. It was developed at Facebook based on LevelDB and provides backwards-compatible support for LevelDB APIs.    

In RocksDB 3.0, we added support for `Column Families`.     

Each key-value pair in RocksDB is associated with exactly one Column Family. If there is no Column Family specified, key-value pair is associated with Column Family "default".     

Column Families provide a way to logically partition the database. Some interesting properties:     

* Atomic writes across Column Families are supported. This means you can atomically execute Write(`{cf1, key1, value1}, {cf2, key2, value2}`).    

* Consistent view of the database across Column Families.    

* Ability to configure different Column Families independently.    

* On-the-fly adding new Column Families and dropping them. Both operations are reasonably fast.      

首先 `RocksDB` 是一个 KV 存储引擎，Queue 的数据存储结构不会光有数据，还需要有 key，可以使用自增序列作为 key，再定义 HEAD，TAIL 分别指向 Queue 的头尾的 K。 所以在 RocksDB 中存储了两种数据，`enqueue`()方法中创建了一个`writeBatch`对象，将插入 `cfHandle` 簇的队列数据以及 将`indexCfHandle` 簇 Key 为 TAIL 的值 + 1 两个操作打包成一个批量操作，这样也保证两部分数据的完整性。     

**代码片段**
```java
try (final WriteBatch writeBatch = new WriteBatch()) {
    final byte[] indexId = Bytes.longToByte(id);
    writeBatch.put(cfHandle, indexId, value);
    writeBatch.merge(indexCfHandle, TAIL, ONE);
    store.write(writeBatch);

    this.rocksQueueMetric.onEnqueue(value.length);
} catch (RocksDBException e) {
    tail.decrementAndGet();
    log.error("Enqueue {} fails", id, e);
    throw new RocksQueueEnqueueException(store.getRockdbLocation(), e);
}
```

`RocksStore` 负责 RocksDB API 的调用逻辑。`StoreOptions` 责任 RocksDB 的相关配置参数。 

>这里需要注意数据存储的 key 一定要`递增`，这是因为 Queue 的读取方式先进先出，我们通过 HEAD 知道 头的 Key 的值，再通过 RocksDB get()方法获取 value值，但是下一个到谁了，我们并没有记录下来，这部分也是可以通过 `RocksDB 的 Iterator`计算出来的。  

**代码片段**
```java
// 示例代码
this.tailIterator = store.newIteratorCF(cfHandle); 

// 检查迭代器是否有效，如果有效，则继续执行循环。只要迭代器指向有效的元素，就可以访问它的键和值  
if (!tailIterator.isValid()) {  
......

// 定位到某个key 
tailIterator.seek(Bytes.longToByte(sid));  
// 当定位到key 后，调用 key(), value() 获取当前key所对应的数据   
tailIterator.key()), tailIterator.value()

```

下面示例的 key 保持了单调递增，所以下一个出队的数据 key 是可以计算出来，数据查找起来就更简单些了。         

All data in the database is logically arranged in `sorted order`. An application can specify a key comparison method that specifies a total ordering of keys. An Iterator API allows an application to do a range scan on the database. The Iterator can seek to a specified key and then the application can start scanning one key at a time from that point. The Iterator API can also be used to do a reverse iteration of the keys in the database. A consistent-point-in-time view of the database is created when the Iterator is created. Thus, all keys returned via the Iterator are from a consistent view of the database.     

Key 会按照有序顺序进行排列，若key 插入顺序为 1,3,2 那通过 `Iterator` 读出来的顺序是 1,2,3。 这显然就是很明显的使用不当了，所以需要特别注意。   

Queue 的 dequeue()方法将数据读取后，再封装 WriteBatch 对象，添加 remove 操作，此时，你可能跟我一开始有同样的疑问，数据是立马删除，还是等某个条件触发后，这里引出了 RocksDB 的 Compaction。     

Compaction algorithms constrain the LSM tree shape. They determine which sorted runs can be merged by it and which sorted runs need to be accessed for a read operation. You can read more on RocksDB Compactions here: Multi-threaded compactions       

>https://github.com/facebook/rocksdb/wiki/Compaction, RocksDB 的 Compaction 是会消耗 CPU 和 磁盘IO 的。  

**代码片段**        
```java
this.cfOpts = new ColumnFamilyOptions()
                .optimizeUniversalStyleCompaction()
                .setMergeOperatorName("uint64add")
                .setMaxSuccessiveMerges(64)
                .setWriteBufferSize(options.getWriteBufferSize())
                .setTargetFileSizeBase(options.getFileSizeBase())
                .setLevel0FileNumCompactionTrigger(8)
                .setLevel0SlowdownWritesTrigger(16)
                .setLevel0StopWritesTrigger(24)
                .setNumLevels(4)
                .setMaxBytesForLevelBase(512 * 1024 * 1024)
                .setMaxBytesForLevelMultiplier(8)
                .setTableFormatConfig(blockBasedTableConfig)
                .setMemtablePrefixBloomSizeRatio(0.1);
```

在这段代码中，以下参数与 **Compaction** 相关：
`optimizeUniversalStyleCompaction()` 该方法启用了 **Universal Compaction** 策略，这是一种基于文件大小、时间等条件的合并策略。Universal Compaction 适用于合并存储在不同层次中的数据，并优化压缩过程，减少写放大。              
   
`setMaxSuccessiveMerges(64)` 这个参数设置了在触发下一次压缩之前，允许的最大连续合并次数。当连续合并次数达到这个阈值时，会触发一次新的压缩。                
   
`setLevel0FileNumCompactionTrigger(8)` 该参数控制 **Level 0** 上的文件数量达到一定值时触发 Compaction。具体来说，当 **Level 0** 的 SST 文件数量达到 8 时，就会触发一次压缩。                  

`setLevel0SlowdownWritesTrigger(16)` 当 **Level 0** 上的文件数量达到 16 时，会触发一个写入速度的减缓机制，避免过多的 SST 文件积累导致性能下降。这个参数通过减缓写入速度，间接控制 Compaction 的触发。     

`setLevel0StopWritesTrigger(24)` 当 **Level 0** 上的文件数量达到 24 时，写入操作会被暂停，直到触发 Compaction 来清理和整理数据。这个参数用于避免 **Level 0** 上的文件过多导致严重的性能问题。       

`setNumLevels(4)`  该参数设置了 RocksDB 的 **Level** 数量。每个 Level 代表不同的数据层次，Compaction 会在不同的 Levels 之间进行，帮助将数据从 Level 0 转移到更低的 Levels，并减少磁盘 I/O。        

`setMaxBytesForLevelBase(512 * 1024 * 1024)` 该参数设置了每个 Level 的基础大小，表示每个 Level 的最大磁盘空间。当某个 Level 的数据大小超过这个限制时，会触发相应的 Compaction 操作。    

`setMaxBytesForLevelMultiplier(8)` 这个参数是一个倍数，用于计算每个 Level 的大小。具体来说，Level 1 的大小是 Level 0 的 8 倍，Level 2 是 Level 1 的 8 倍，以此类推。这个参数影响各个 Level 之间的 Compaction 过程。    

这些参数都与 **Compaction** 过程密切相关，控制了压缩的触发条件、合并策略、以及不同 Levels 之间的数据管理。通过合理设置这些参数，可以优化 RocksDB 的压缩行为，以提高读写性能、减少磁盘占用并降低写放大的问题。     

>https://github.com/facebook/rocksdb/wiki/Compaction     

### 小结            
Queue on RocksDB 大大简化了代码逻辑复杂度，不直接操作 file 相关 API。 但它终将是要写入磁盘的，所以`我们仍要花代价去监控它，尽早发现问题，解决问题。避免 RocksDB API 用错的坑`。   

### StoreOptions.java  
StoreOptions 定义 RocksDB 的参数 
```java
public class StoreOptions {
    private String directory;
    private int writeBufferSize;
    private int writeBufferNumber;
    private int memorySize;
    private long fileSizeBase;
    private CompressionType compression;
    private int parallel;
    private boolean disableAutoCompaction;
    private boolean disableWAL;
    private boolean disableTailing;
    private boolean writeLogSync;
    private boolean isDebug;
    private String database;

    private StoreOptions(Builder builder) {
        this.directory = builder.directory;
        this.database = builder.database;
        this.writeBufferSize = builder.writeBufferSize;
        this.writeBufferNumber = builder.writeBufferNumber;
        this.memorySize = builder.memorySize;
        this.fileSizeBase = builder.fileSizeBase;
        this.compression = builder.compression;
        this.parallel = builder.parallel;
        this.disableAutoCompaction = builder.disableAutoCompaction;
        this.disableWAL = builder.disableWAL;
        this.disableTailing = builder.disableTailing;
        this.writeLogSync = builder.writeLogSync;
        this.isDebug = builder.isDebug;

        if (this.memorySize <= 0) this.memorySize = 8 * 1024 * 1024;
        if (this.fileSizeBase <= 0) this.fileSizeBase = 64 * 1024 * 1024;
        if (this.writeBufferSize <= 0) this.writeBufferSize = 64 * 1024 * 1024;
        if (this.writeBufferNumber <= 0) this.writeBufferNumber = 4;
        if (this.parallel <= 0) this.parallel = Math.max(Runtime.getRuntime().availableProcessors(), 2);
//        if (this.compression == null) this.compression = CompressionType.;

        this.disableTailing = false;
        this.disableWAL = false;
        this.writeLogSync = true;
    }

    public static Builder builder() {
        return new Builder();
    }

    public String getDirectory() {
        return directory;
    }

    public int getWriteBufferSize() {
        return writeBufferSize;
    }

    public int getWriteBufferNumber() {
        return writeBufferNumber;
    }

    public int getMemorySize() {
        return memorySize;
    }

    public long getFileSizeBase() {
        return fileSizeBase;
    }

    public CompressionType getCompression() {
        return compression;
    }

    public int getParallel() {
        return parallel;
    }

    public boolean isDisableAutoCompaction() {
        return disableAutoCompaction;
    }

    public boolean isDisableWAL() {
        return disableWAL;
    }

    public boolean isDisableTailing() {
        return disableTailing;
    }

    public boolean isWriteLogSync() {
        return writeLogSync;
    }

    public boolean isDebug() {
        return isDebug;
    }

    public String getDatabase() {
        return database;
    }

    public static class Builder {
        public String database;
        private String directory;
        private int writeBufferSize;
        private int writeBufferNumber;
        private int memorySize;
        private long fileSizeBase;
        private CompressionType compression;
        private int parallel;
        private boolean disableAutoCompaction;
        private boolean disableWAL;
        private boolean disableTailing;
        private boolean writeLogSync;
        private boolean isDebug;

        public StoreOptions build() {
            return new StoreOptions(this);
        }

        public Builder database(String database) {
            this.database = database;
            return this;
        }

        public Builder directory(String directory) {
            this.directory = directory;
            return this;
        }

        public Builder writeBufferSize(int writeBufferSize) {
            this.writeBufferSize = writeBufferSize;
            return this;
        }

        public Builder writeBufferNumber(int writeBufferNumber) {
            this.writeBufferNumber = writeBufferNumber;
            return this;
        }

        public Builder memorySize(int memorySize) {
            this.memorySize = memorySize;
            return this;
        }

        public Builder fileSizeBase(long fileSizeBase) {
            this.fileSizeBase = fileSizeBase;
            return this;
        }

        public Builder compression(CompressionType compression) {
            this.compression = compression;
            return this;
        }

        public Builder parallel(int parallel) {
            this.parallel = parallel;
            return this;
        }

        public Builder disableAutoCompaction(boolean disableAutoCompaction) {
            this.disableAutoCompaction = disableAutoCompaction;
            return this;
        }

        public Builder disableWAL(boolean disableWAL) {
            this.disableWAL = disableWAL;
            return this;
        }

        public Builder disableTailing(boolean disableTailing) {
            this.disableTailing = disableTailing;
            return this;
        }

        public Builder writeLogSync(boolean writeLogSync) {
            this.writeLogSync = writeLogSync;
            return this;
        }
        public Builder debug(boolean debug) {
            isDebug = debug;
            return this;
        }
    }
}
```

### RocksStore.java    
RocksStore 是对 RocksDB API 的封装类 
```java
public class RocksStore {
    private static final Logger log = LoggerFactory.getLogger(RocksStore.class);
    private final String database;
    private final String directory;
    private final String fullPath;
    private final HashMap<String, RocksQueue> queues;
    private final DBOptions dbOptions;
    private final ColumnFamilyOptions cfOpts;
    private final ArrayList<ColumnFamilyHandle> cfHandles;
    private final ReadOptions readOptions;
    private final WriteOptions writeOptions;
    private final RocksDB db;
    private final RocksStoreMetric rocksStoreMetric;
    private final Map<String, ColumnFamilyHandle> columnFamilyHandleMap = new HashMap<>();

    static {
        RocksDB.loadLibrary();
    }

    public RocksStore(StoreOptions options) {
        if(Strings.nullOrEmpty(options.getDatabase())) {
            throw new RuntimeException("Empty database of store options");
        }

        if(options.isDebug()) {
            log.isDebugEnabled();
        }

        this.directory = options.getDirectory();
        this.database = options.getDatabase();
        this.fullPath = generateFullDBPath(directory, database);
        this.cfHandles = new ArrayList<>();
        this.queues = new HashMap<>();

        this.readOptions = new ReadOptions()
                .setFillCache(false)
                .setTailing(!options.isDisableTailing());
        this.writeOptions = new WriteOptions()
                .setDisableWAL(options.isDisableWAL())
                .setSync(options.isWriteLogSync());

        Files.mkdirIfNotExists(this.fullPath);

        this.dbOptions = new DBOptions()
                .setCreateIfMissing(true)
                .setIncreaseParallelism(options.getParallel())
                .setCreateMissingColumnFamilies(true)
                .setMaxTotalWalSize(64 * 1024 * 1024)
                .setKeepLogFileNum(10)
                .setMaxOpenFiles(-1);

        final BlockBasedTableConfig blockBasedTableConfig = new BlockBasedTableConfig()
                .setBlockCacheSize(options.getMemorySize())
                .setCacheIndexAndFilterBlocks(true) //By putting index and filter blocks in block cache to control memory usage
                .setPinL0FilterAndIndexBlocksInCache(true) //Tune for the performance impact
                .setFilter(new BloomFilter(10));

        this.cfOpts = new ColumnFamilyOptions()
                .optimizeUniversalStyleCompaction()
                .setMergeOperatorName("uint64add")
                .setMaxSuccessiveMerges(64)
                .setWriteBufferSize(options.getWriteBufferSize())
                .setTargetFileSizeBase(options.getFileSizeBase())
                .setLevel0FileNumCompactionTrigger(8)
                .setLevel0SlowdownWritesTrigger(16)
                .setLevel0StopWritesTrigger(24)
                .setNumLevels(4)
                .setMaxBytesForLevelBase(512 * 1024 * 1024)
                .setMaxBytesForLevelMultiplier(8)
                .setTableFormatConfig(blockBasedTableConfig)
                .setMemtablePrefixBloomSizeRatio(0.1);

        if(options.getCompression() != null) {
            cfOpts.setCompressionType(options.getCompression());
        }

        this.rocksStoreMetric = new RocksStoreMetric(this);
        this.rocksStoreMetric.register();

        db = openRocksDB();
    }

    private RocksDB openRocksDB() {
        RocksDB rocksDB;

        final List<ColumnFamilyDescriptor> cfDescriptors = new ArrayList<>();
        final List<ColumnFamilyHandle> columnFamilyHandleList = new ArrayList<>();

        //load existing column families
        try {
            List<byte[]> columnFamilies = RocksDB.listColumnFamilies(new Options(), this.fullPath);
            log.debug("Load existing column families {}", columnFamilies.stream().map(cf -> Bytes.bytesToString(cf)).collect(toList()));

            columnFamilies.forEach( cf -> cfDescriptors.add(new ColumnFamilyDescriptor(cf, cfOpts)));
        } catch (RocksDBException e) {
            log.warn("Load existing column families failed.", e);
        }

        if(cfDescriptors.isEmpty()) {
            cfDescriptors.add(new ColumnFamilyDescriptor(RocksDB.DEFAULT_COLUMN_FAMILY, cfOpts));
        }

        try {
            rocksDB = RocksDB.open(dbOptions, fullPath, cfDescriptors, columnFamilyHandleList);
        } catch (RocksDBException e) {
            log.error("Failed to open rocks database, try to remove rocks db database {}", fullPath, e);
            Files.deleteDirectory(fullPath);
            try {
                rocksDB = RocksDB.open(dbOptions, fullPath, cfDescriptors, columnFamilyHandleList);
                log.info("Recreate rocks db at {} again from scratch", fullPath);
            } catch (RocksDBException e1) {
                log.error("Failed to create rocks db again at {}", fullPath, e);
                throw new RuntimeException("Failed to create rocks db again.");
            }
        }

        this.rocksStoreMetric.onOpen();

        //Cache <columnFamilyName, columnFamilyHandle> relations
        for (int i = 0; i < cfDescriptors.size(); i++) {
            ColumnFamilyDescriptor columnFamilyDescriptor = cfDescriptors.get(i);
            if(columnFamilyDescriptor != null) {
                columnFamilyHandleMap.put(Bytes.bytesToString(columnFamilyDescriptor.columnFamilyName()),
                        columnFamilyHandleList.get(i));
            }
        }

        return rocksDB;
    }

    public void close() {
        readOptions.close();
        writeOptions.close();

        dbOptions.close();
        cfOpts.close();
        for(ColumnFamilyHandle handle: cfHandles) {
            handle.close();
        }

        for (RocksQueue rocksQueue: queues.values()) {
            if(rocksQueue != null) {
                rocksQueue.close();
            }
        }

        db.close();

        this.rocksStoreMetric.onClose();
    }

    public RocksQueue createQueue(final String queueName) {
        if(Strings.nullOrEmpty(queueName)) {
            throw new IllegalArgumentException("Create rocks queue name can't not be null or empty");
        }

        if(queues.containsKey(queueName)) {
            return queues.get(queueName);
        }

        RocksQueue queue = new RocksQueue(queueName, this);

        queues.put(queueName, queue);

        return queue;
    }

    public ColumnFamilyHandle createColumnFamilyHandle(String cfName) {

        if(Strings.nullOrEmpty(cfName)) {
            return null;
        }

        if(columnFamilyHandleMap.containsKey(cfName)) {
            return columnFamilyHandleMap.get(cfName);
        }

        final ColumnFamilyDescriptor cfDescriptor = new ColumnFamilyDescriptor(Bytes.stringToBytes(cfName),
                cfOpts);

        ColumnFamilyHandle handle = null;
        try {
            handle = db.createColumnFamily(cfDescriptor);
        } catch (RocksDBException e) {
            log.error("Create column family fail", e);
        }

        columnFamilyHandleMap.put(cfName, handle);

        return handle;
    }

    public RocksIterator newIteratorCF(ColumnFamilyHandle cfHandle) {
        return db.newIterator(cfHandle, this.readOptions);
    }

    public byte[] getCF(byte[] key, ColumnFamilyHandle cfHandle) {
        byte[] value = null;
        try {
            value = db.get(cfHandle, key);
        } catch (RocksDBException e) {
            log.error("Failed to get {} from rocks db, {}", key, e);
        }
        return value;
    }

    public void write(WriteBatch writeBatch) throws RocksDBException {
        db.write(this.writeOptions, writeBatch);
    }

    public int getQueueSize() {
        return queues.size();
    }

    public String getDatabase() {
        return this.database;
    }

    public String getRockdbLocation() {
        return this.fullPath;
    }

    private String generateFullDBPath(String base, String database) {
        if(Strings.nullOrEmpty(base)) {
            return "./" + database;
        }

        File baseFile = new File(directory);

        return baseFile.getAbsolutePath() + File.separator + database;
    }

    public RocksStoreMetric getRocksStoreMetric() {
        return this.rocksStoreMetric;
    }
}
```  

### RocksQueue.java  
包装 queue 的写入和读取     
```java
public class RocksQueue {
    private static final Logger log = LoggerFactory.getLogger(RocksQueue.class);

    private static final byte[] HEAD = Bytes.stringToBytes("head");
    private static final byte[] TAIL = Bytes.stringToBytes("tail");
    private static final byte[] ONE = Bytes.longToByte(1);

    private final String queueName;

    private final AtomicLong head = new AtomicLong();
    private final AtomicLong tail = new AtomicLong();

    private final ColumnFamilyHandle cfHandle;
    private final ColumnFamilyHandle indexCfHandle;
    private final RocksIterator tailIterator;
    private final RocksStore store;
    private final RocksQueueMetric rocksQueueMetric;

    public RocksQueue(final String queueName, final RocksStore store) {
        this.queueName = queueName;
        this.store = store;

        this.cfHandle = store.createColumnFamilyHandle(queueName);
        this.indexCfHandle = store.createColumnFamilyHandle(getIndexColumnFamilyName(queueName));

        this.tail.set(getIndexId(TAIL, 0));
        this.head.set(getIndexId(HEAD, 0));

        this.tailIterator = store.newIteratorCF(cfHandle);

        this.rocksQueueMetric = new RocksQueueMetric(this, this.store.getDatabase());
        this.rocksQueueMetric.register();
        this.rocksQueueMetric.onInit();
    }

    private long getIndexId(byte[] key, long defaultValue) {
        byte[] value = store.getCF(key, indexCfHandle);

        if (value == null) {
            return defaultValue;
        }

        return Bytes.byteToLong(value);
    }

    public long enqueue(byte[] value) throws RocksQueueException {
        long id = tail.incrementAndGet();

        try (final WriteBatch writeBatch = new WriteBatch()) {
            final byte[] indexId = Bytes.longToByte(id);
            writeBatch.put(cfHandle, indexId, value);
            writeBatch.merge(indexCfHandle, TAIL, ONE);
            store.write(writeBatch);

            this.rocksQueueMetric.onEnqueue(value.length);
        } catch (RocksDBException e) {
            tail.decrementAndGet();
            log.error("Enqueue {} fails", id, e);
            throw new RocksQueueEnqueueException(store.getRockdbLocation(), e);
        }

        return id;
    }

    /**
     * Get the head and remove it from queue
     *
     * @return
     */
    public QueueItem dequeue() throws RocksQueueException {
        QueueItem item = consume();
        try {
            removeHead();
        } catch (RocksQueueException e) {
            throw new RocksQueueDequeueException(store.getRockdbLocation(), e);
        }
        if (item != null && item.getValue() != null) {
            this.rocksQueueMetric.onDequeue(item.getValue().length);
        }
        return item;
    }

    /**
     * Get the head of queue, in case there will have many deleted tombstones,
     * the final return index maybe bigger than the startId.
     *
     * @return
     */
    public QueueItem consume() {
        if (this.getSize() == 0) {
            return null;
        }

        //for the first time, if head is 0, seek from 1
        long sid = head.get() + 1;

        if (!tailIterator.isValid()) {
            tailIterator.seek(Bytes.longToByte(sid));
        }

        //when dequeue happens faster than enqueue, the tail iterator would be exhausted,
        //so we seek it again
        if (!tailIterator.isValid()) {
            return null;
        }

        this.rocksQueueMetric.onConsume();

        return new QueueItem(Bytes.byteToLong(tailIterator.key()), tailIterator.value());
    }

    /**
     * Remove the head from queue
     *
     * @return
     */
    public void removeHead() throws RocksQueueException {
        if (this.getSize() <= 0) {
            return;
        }

        try (final WriteBatch writeBatch = new WriteBatch()) {
            writeBatch.remove(cfHandle, Bytes.longToByte(head.get()));
            writeBatch.merge(indexCfHandle, HEAD, ONE);
            store.write(writeBatch);
            head.incrementAndGet();
            tailIterator.next();
        } catch (RocksDBException e) {
            log.error("Remove head {} failed.", head.get());
            throw new RocksQueueRemoveHeadException(store.getRockdbLocation(), e);
        }
    }

    public void close() {
        cfHandle.close();
        indexCfHandle.close();
        tailIterator.close();

        this.rocksQueueMetric.onClose();
    }

    private String getIndexColumnFamilyName(String queueName) {
        return new StringBuilder()
                .append("_")
                .append(queueName)
                .toString();
    }

    public boolean isEmpty() {
        return tail.get() == 0 ? true : tail.get() <= head.get();
    }

    public long getSize() {
        return tail.get() - head.get();
    }

    public long getHeadIndex() {
        return head.get();
    }

    public long getTailIndex() {
        return tail.get();
    }

    public long approximateSize() {
        return getIndexId(TAIL, 0) - getIndexId(HEAD, 0);
    }

    public String getQueueName() {
        return this.queueName;
    }

    public RocksQueueMetric getRocksQueueMetric() {
        return this.rocksQueueMetric;
    }
}
```

### QueueItem.java
```java
public class QueueItem {

    private long index;
    private byte[] value;

    public QueueItem(long index, byte[] value) {
        this.index = index;
        this.value = value;
    }

    public long getIndex() {
        return index;
    }

    public void setIndex(long index) {
        this.index = index;
    }

    public byte[] getValue() {
        return value;
    }

    public void setValue(byte[] value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "QueueItem{" +
                "index=" + index +
                ", value=" + Arrays.toString(value) +
                '}';
    }
}
```


完整示例代码：https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-db/src/main/java/com/javamain/db/rocksdbqueue01      

refer   
1.https://github.com/facebook/rocksdb/wiki/Implement-Queue-Service-Using-RocksDB  
2.https://github.com/artiship/rocks-queue-java     
3.[RIP-73] https://github.com/apache/rocketmq/wiki/%5BRIP%E2%80%9073%5D-Pop-Consumption-Improvement-Based-on-RocksDB  
4.https://juejin.cn/post/7152768942195343390                
5.https://developer.aliyun.com/article/801815           
6.https://github.com/apache/rocketmq/issues/9141        
7.https://docs.google.com/document/d/1D6XWwY39p531c2aVi5HQll9iwzTUNT1haUFHqMoRkT0/edit?pli=1&tab=t.0#heading=h.hdivf7j4u03n           
8.https://github.com/facebook/rocksdb/wiki/Iterator   
9.https://github.com/facebook/rocksdb/wiki/Compaction    
