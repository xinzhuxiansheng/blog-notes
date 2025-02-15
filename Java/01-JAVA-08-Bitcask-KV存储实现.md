# Bitcask - 探索 Bitcask 的 K/V 存储实现      

## Bitcask Paper    
`bitcask is a log-structured hash table for fast key/value data.` 它是由做分布式存储软件的`riak`公司提出的，论文原地址：`https://riak.com/assets/bitcask-intro.pdf`, 你也可以访问我对 bitcask paper做机翻的 Blog `http://www.xinzhuxiansheng.com/articleDetail/85`，了解它的实现细节。  

>希望你阅读过我之前几篇的 Blog & Implement, 因为阅读完 Bitcask paper 之后，我们仍然还是在探索 index files 和 data files 之间的事。     
1.JAVA - 了解内存映射 Memory-mapped 和 MappedByteBuffer (它值得你去花时间)   
2.JAVA - Memory-mapped I/O - 探索构造 BigArray 读写 File 篇   
3.JAVA - Memory-mapped I/O - 探索給 BigArray 加锁  
4.JAVA - RocksDB - 探索 Queue on RocksDB    

![bitcask01](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask01.png)  

## Bitcask 存储结构      

### Data Files  
在 bitcask 数据模型中，data file 是以日志追加（`append-only`）的方式写入。`即将`写入数据的文件称为`active data file`, 当 active data file 文件大小增加到某个阈值时（`segment size`），会创建一个新的 data file 用于写入，此时 active data file 则指向新文件,所以可追加写入的 file 有且仅有 1个，而旧的文件（`older data file`）仅用于读取数据,它已不可再改变（immutable data file）。       

![bitcask02](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask02.png)        

data file 中每个 key/value 数据格式非常简单，包含 crc,tstamp,ksz,vsz,k,v 如下图：       
![bitcask03](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask03.png)  

如果我们想给 Key/Value 存储服务增加一个过期时间 ttl，那我们扩展后的数据结构如下，在后面的代码实现案例中，使用的是带有 ttl 信息的数据结构存储。              
![bitcask04](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask04.png)

设置某个 key 的ttl 时间为 1min，那 ttl = tstamp(毫秒) + 1 * 60 * 1000。  

关于 crc 循环冗余检查，主要用于数据完整性校验，确保在读写操作过程中数据没有被损坏或者篡改，这里对于用过 Kafka 可能会清楚一些，在 Kafka Record 的格式中也包含 CRC字段信息。              
在 bitcask paper 中提到一个关键字 `a special tombstone value`,这个会在后面的删除以及合并章节中介绍。   

### KeyDir  
除了上述的 data files以外，bitcask 还在内存中定义了以 Hash Table 为数据结构的`KeyDir`，当 K/V 数据追加写入到 data file 之后，会将部分元数据信息插入到 Hash Table中。    
keydir 的数据存储结构如下图, 它包含 key，file_id,vsz,v_pos,tstamp。       
![bitcask05](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask05.png)   

在真实的案例中，我们存储的信息可不止这些，例如 file_id 会变成 logsegment，需要提前通过 data file Path 类 创建 FileChannel 对象，不过大家不要受到内存映射 Memory I/O 影响，FileChannel 并不会立刻占用 磁盘和内存大小。 添加 ttl 过期时间。   
![bitcask06](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask06.png)   

若给定某个 key 值，通过 keydir 检索数据也是比较简单的，而且只需要一次磁盘寻址，查找 Key后，通过 Logsegment，v_pos, vsz 非常容易定位到具体 data file 的record。在内存中 Hash Table 查找某个 Key的时间复杂度为 O(1)，它会比预期要快的多。  
**在 bitcask paper 的 keydir 也给出通过 keydir index 如何查询 data file 的数据链路图,如下**                 
![bitcask07](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask07.png)   

在之前存储开发的经验来说，KeyDir 或者其他的 index file 如果出现异常或者丢失，都是可以从 data files 重新构造索引信息的。   

这里想扩展一下，RocketMQ 的 index file 的结构设计，它就类似于 JDK 的 HashMap 的结构。   

>在 bitcask 存储模型中还涉及到 hint file，这部分会在下面的`Merge and Compaction` 中会介绍。    

## K/V 数据的操作      

### 插入一个新的 K/V 数据       
当提交新的 K/V 数据时，首先将其追加到`Active Data File`中，然后在 `KeyDir` 中创建一个新的条目，以指定存储该值的偏移量、文件、存储大小等元数据信息。这两种动作通过读写锁机制保证保证原子执行，通过读写锁机制保证。      

### 更新已存在的 K/V 数据    
在 bitcask 存储模型中，data file 只能是 append-only方式写入的，针对 K/V 数据的 update操作，是在 `active data file`中追加更新的数据，注意，更新操作与插入新数据是同等操作，不存在部分更新值，而是全覆盖。  
![bitcask08](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask08.png)    

### 删除某个 Key 
对于删除某个 key 的操作，在 bitcask的存储模型中，它对应是 update，但此时它的 `value is a special tombstone value`, `不过需要注意的是`，在 active data file 追加完数据后，针对 keydir的处理，调用的是 remove(key)方法，用于在 Hash Table 中移除某个 key。     

那此时，删除某个 key 得到最后的结果是 在 active data file 追加一条 value 为 tombstone value 的 K/V 数据，移除 KeyDir 中的 key，所以内存中已经不存在该 key 的数据了。

![bitcask09](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask09.png)

>关于 tombstone value 在开发案例中将 value 赋值成空字节数组 `Bytes EMPTY = new Bytes(new byte[0])。  `    

>注意，更新、删除 两个操作是会增加 data file 的数据的冗余的，因为我们一直都是在追加，所以它必然会导致相同 key 的数据，存储在 data files中，后面我们会介绍 bitcask 的 merge and compaction。   

### 读取 K/V 数据 
K/V 数据的读取逻辑是先从 keydir Hash Table 结构中 get(key), 获取元数据信息，若不为空，则判断 ttl 是否过期，再根据 logsegment,v_pos,vsz 在 data file 中读取 value 值，`注意`,我们得先判断 value 是否是一个 tombstone value，我们得通过它来区分当前的 K/V 数据是否已经删除了。若没有删除，则返回 value 值。  
![bitcask10](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask10.png)  

## 合并和压缩   
更新和删除操作都是在 active data file 追加新的 K/V 数据，这部分逻辑会出现大量的冗余数据，在高并发情况下势必会占用多余的磁盘空间。 在 bitcask 存储模型中，会对`older data files`遍历读取，针对有效数据会重新插入到 new data file 中，创建 new index record 存储到 .hint file 中，并且再 keydir 中更新元数据信息，但这并不意味着`new data file 会是 active data file`,     

![bitcask12](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask12.png)  

在 bitcask paper 针对合并介绍的很少，但是它标注出了合并后新创建的文件`memged data files` 以及 `hint file`，不过 hint file 存储的数据结构为：  
![bitcask17](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask17.png)   

下面给出一个较为完整的处理逻辑：                
![bitcask13](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask13.png)          

只针对 older data files 进行合并，但并不是所有的 older data files，这里提出一个概念 `dirty data files`。这部分在 bitcask paper 中无从可查，可参考`riakkv doc`(https://docs.riak.com/riak/kv/2.2.3/setup/planning/backend/bitcask.1.html#merge-policy)      
![bitcask14](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask14.png)   

>注意：riakkv 的 merge prolicy 可以作为我们技术实现的参考，那我们回到判定`dirty data files`规则来。           

![bitcask15](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask15.png)    
1. Fragmentation (碎片化)               
若某个 older data file 中的无效 K/V数据个数占比达到 阈值后，则判定为 dirty data file            

2. Dead Bytes(死字节)           
若某个 older data file 中的无效 K/V数据字节大小和占比达到 阈值后，则判定为 dirty data file              

3. Small File(小文件)           
若某个 older data file 字节大小小于 阈值后，则不会被判定为 dirty data file，这样避免频繁合并操作带来的性能的开销。              

有了`dirty data files`之后，遍历&判定是否是有效数据，有效则插入 new data file，注意，此时的 index record 是写入名为 .hint.partial 文件， 这是为了保障 .log 与 .hint 两个文件的完整性，当 .log 合并完成后，再将 .hint.partial 重命名为 .hint。 `.hint file`的完整性尤为重要，因为，它存在后，当服务重启后，keydir 并不会直接先从 .log 文件恢复，而是直接读取 .hint file。这对提高启动速度尤其重要。   
![bitcask16](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask16.png)  

**示例** 
![bitcask11](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask11.png)     
`00000000000000000000.log` 文件被判定需要合并，遍历数据将有效数据插入到 `00000000000000000005.log` 并且创建 index record，存储在 `00000000000000000005.hint`，`注意`，index data 需要在 keydir Hash Table 做更新，因为 keydir 中的 record 会存有 data file 相关元数据信息。    

>这部分内容是否让你想起 RocksDB 中的 Compact 特性。     

## 代码实现
项目结构:       
![bitcask18](http://img.xinzhuxiansheng.com/blogimgs/java/bitcask18.png)   

### BitcaskStore.java 
```java
public class BitcaskStore implements KeyValueStore<Bytes, Bytes> {
    private static final Logger logger = LoggerFactory.getLogger(BitcaskStore.class);

    private final KeyDir keyDir;
    private LogSegment activeSegment;
    private final Clock clock;
    private final long logSegmentBytes;
    private final LogSegmentNameGenerator segmentNameGenerator;
    private final LogCleaner logCleaner;
    private final SegmentWriter writer;

    private final ReadWriteLock rw = new ReentrantReadWriteLock();

    private BitcaskStore(
            Path logDir,
            KeyDir keyDir,
            LogSegment activeSegment,
            Clock clock,
            long logSegmentBytes,
            long compactionSegmentMinBytes,
            Duration compactionInterval,
            double minDirtyRatio,
            int compactionThreads,
            SegmentWriterFactory writerFactory) {
        this.keyDir = keyDir;
        this.activeSegment = activeSegment;
        this.clock = clock;
        this.logSegmentBytes = logSegmentBytes;
        this.segmentNameGenerator = LogSegmentNameGenerator.from(activeSegment);

        this.logCleaner = new LogCleaner(
                logDir,
                keyDir,
                activeSegmentSupplier(),
                segmentNameGenerator,
                minDirtyRatio,
                compactionSegmentMinBytes,
                logSegmentBytes,
                compactionThreads);

        this.logCleaner.start(compactionInterval);

        this.writer = writerFactory.create(activeSegmentSupplier());
    }

    public static BitcaskStore open() {
        return open(Options.defaults.storage);
    }

    public static BitcaskStore open(Path logDir) {
        return new Builder(logDir).build();
    }

    public static BitcaskStore open(StorageConfig storageConfig) {
        return new Builder(storageConfig).build();
    }

    @Override
    public Optional<Bytes> get(Bytes key) {
        Objects.requireNonNull(key, "key cannot be null");
        ValueReference valueRef = keyDir.get(key);
        if (valueRef == null) {
            return Optional.empty();
        }
        if (valueRef.isExpired(clock.millis())) {
            keyDir.remove(key);
            return Optional.empty();
        }
        try {
            Bytes valueBytes = valueRef.get();
            if (valueBytes.equals(Record.TOMBSTONE)) {
                return Optional.empty();
            }
            return Optional.of(valueBytes);
        } catch (IOException ex) {
            throw new KiwiException("Failed to read value from active segment " + activeSegment.name(), ex);
        }
    }

    @Override
    public void put(Bytes key, Bytes value) {
        put(key, value, 0L);
    }

    @Override
    public void delete(Bytes key) {
        Objects.requireNonNull(key, "key cannot be null");
        put(key, Record.TOMBSTONE);
    }

    @Override
    public void put(Bytes key, Bytes value, long ttl) {
        Objects.requireNonNull(key, "key cannot be null");
        long now = clock.millis();
        Record record = Record.of(key, value, now, ttl != 0 ? now + ttl : 0);

        rw.readLock().lock();
        try {
            int written = writer.append(record);
            if (written > 0) {
                keyDir.update(record, activeSegment);
            } else {
                throw new KiwiException("Failed to write to segment");
            }
        } finally {
            rw.readLock().unlock();
        }

        maybeRollSegment();
    }

    private void maybeRollSegment() {
        // Optimistic check to avoid acquiring lock.
        if (shouldRoll()) {
            rw.writeLock().lock();
            try {
                // We need to check again after acquiring lock to prevent
                // because multiple threads can enter maybeRollSegment and pass first optimistic check.
                if (shouldRoll()) {
                    activeSegment.markAsReadOnly();
                    activeSegment = LogSegment.open(segmentNameGenerator.next());
                    logger.info("Opened new log segment {}", activeSegment.name());
                }
            } finally {
                rw.writeLock().unlock();
            }
        }
    }

    private boolean shouldRoll() {
        return activeSegment.size() >= logSegmentBytes;
    }

    @Override
    public boolean contains(Bytes key) {
        Objects.requireNonNull(key, "key cannot be null");
        return keyDir.containsKey(key);
    }

    @Override
    public int size() {
        return keyDir.size();
    }

    @Override
    public void purge() {
        keyDir.keys().asIterator().forEachRemaining(this::delete);
    }

    @Override
    public void close() {
        logCleaner.close();
        writer.close();
    }

    private Supplier<LogSegment> activeSegmentSupplier() {
        return () -> activeSegment;
    }

    public static Builder Builder() {
        return new Builder();
    }

    public static Builder Builder(Path logDir) {
        return new Builder(logDir);
    }

    public static Builder Builder(StorageConfig storageConfig) {
        return new Builder(storageConfig);
    }

    public static class Builder {

        private Path logDir;
        private KeyDir keyDir;
        private LogSegment activeSegment;
        private Clock clock = Clock.systemUTC();
        private int keyDirBuilderThreads;
        private long logSegmentBytes;
        private long compactionSegmentMinBytes;
        private Duration compactionInterval;
        private double minDirtyRatio;
        private int compactionThreads;
        private final SegmentWriterFactory writerFactory;

        Builder() {
            this(Options.defaults.storage);
        }

        Builder(Path logDir) {
            this(Options.defaults.storage);
            this.logDir = logDir;
        }

        Builder(StorageConfig config) {
            this.logDir = config.log.dir;
            this.keyDirBuilderThreads = config.log.keyDirBuilderThreads;
            this.logSegmentBytes = config.log.segmentBytes;
            this.compactionSegmentMinBytes = config.log.compaction.segmentMinBytes;
            this.compactionInterval = config.log.compaction.interval;
            this.minDirtyRatio = config.log.compaction.minDirtyRatio;
            this.compactionThreads = config.log.compaction.threads;
            this.writerFactory = new SegmentWriterFactory(config.log.sync);
        }

        public Builder withLogDir(Path logDir) {
            this.logDir = logDir;
            return this;
        }

        public Builder withKeyDirBuilderThreads(int threads) {
            this.keyDirBuilderThreads = threads;
            return this;
        }

        public Builder withLogSegmentBytes(long logSegmentBytes) {
            this.logSegmentBytes = logSegmentBytes;
            return this;
        }

        public Builder withCompactionSegmentMinBytes(long minBytes) {
            this.compactionSegmentMinBytes = minBytes;
            return this;
        }

        public Builder withCompactionInterval(Duration compactionInterval) {
            this.compactionInterval = compactionInterval;
            return this;
        }

        public Builder withMinDirtyRatio(double minDirtyRatio) {
            this.minDirtyRatio = minDirtyRatio;
            return this;
        }

        public Builder withCompactionThreads(int threads) {
            this.compactionThreads = threads;
            return this;
        }

        public Builder withClock(Clock clock) {
            this.clock = clock;
            return this;
        }

        public BitcaskStore build() {
            init(logDir);
            return new BitcaskStore(
                    logDir,
                    keyDir,
                    activeSegment,
                    clock,
                    logSegmentBytes,
                    compactionSegmentMinBytes,
                    compactionInterval,
                    minDirtyRatio,
                    compactionThreads,
                    writerFactory);
        }

        private void init(Path logDir) {
            try {
                Files.createDirectories(logDir);
            } catch (IOException ex) {
                throw new KiwiException("Failed to create log directory " + logDir, ex);
            }

            logger.info("Building keydir from log directory {}", logDir.normalize().toAbsolutePath());

            try (Stream<Path> paths = Files.walk(logDir)) {
                List<Path> segmentPaths = paths.filter(Files::isRegularFile)
                        .filter(path -> path.getFileName().toString().endsWith(".log"))
                        .sorted()
                        .toList();

                ExecutorService executor = Executors.newFixedThreadPool(keyDirBuilderThreads, NamedThreadFactory.create("keydir"));
                List<Future<KeyValue<Path, Map<Bytes, ValueReference>>>> futures = new ArrayList<>();

                for (Path segmentPath : segmentPaths) {
                    futures.add(executor.submit(() -> {
                        LogSegment segment;
                        if (segmentPath.equals(segmentPaths.get(segmentPaths.size() - 1))) { // 按 fileName 排序
                            activeSegment = LogSegment.open(segmentPath);
                            segment = activeSegment;
                        } else {
                            segment = LogSegment.open(segmentPath, true);
                        }
                        Map<Bytes, ValueReference> partialKeyDir = segment.buildKeyDir();
                        return KeyValue.of(segmentPath, partialKeyDir);
                    }));
                }

                // Wait for all tasks to complete
                List<KeyValue<Path, Map<Bytes, ValueReference>>> results = new ArrayList<>();
                for (Future<KeyValue<Path, Map<Bytes, ValueReference>>> future : futures) {
                    results.add(future.get());
                }
                executor.shutdown();

                // Collect all key-value pairs from all segments ordered by segment number.
                // This is necessary to ensure that the latest value for a key is retained.
                keyDir = new KeyDir();

                results.stream()
                        .sorted(Comparator.comparing(KeyValue::key))
                        .map(KeyValue::value)
                        .forEach(partialKeyDir -> {
                            for (Map.Entry<Bytes, ValueReference> entry : partialKeyDir.entrySet()) {
                                Bytes key = entry.getKey();
                                ValueReference value = entry.getValue();
                                if (value == null) {
                                    keyDir.remove(key);
                                } else {
                                    keyDir.put(key, value);
                                }
                            }
                        });

                // Remove tombstones.
                keyDir.values().removeIf(Objects::isNull);

                // Create new active segment when there are no segment files.
                if (activeSegment == null) {
                    Path activeSegmentPath = new LogSegmentNameGenerator(logDir).next();
                    activeSegment = LogSegment.open(activeSegmentPath);
                }
            } catch (IOException | InterruptedException | ExecutionException ex) {
                throw new KiwiReadException("Failed to read log directory " + logDir, ex);
            }

            logger.info("Store initialized with {} entries and {} active log segment", keyDir.size(), activeSegment.name());
        }
    }
}
```

### LogSegment.java
```java
public class LogSegment {
    private static final Logger logger = LoggerFactory.getLogger(LogSegment.class);

    public static final String EXTENSION = ".log";

    private final Path file;
    private FileChannel channel;
    private final Clock clock;

    LogSegment(Path file, FileChannel channel) {
        this(file, channel, Clock.systemUTC());
    }

    LogSegment(Path file, FileChannel channel, Clock clock) {
        this.file = file;
        this.channel = channel;
        this.clock = clock;
    }

    public static LogSegment open(Path file) {
        return open(file, false, Clock.systemUTC());
    }

    public static LogSegment open(Path file, boolean readOnly) throws KiwiException {
        return open(file, readOnly, Clock.systemUTC());
    }

    public static LogSegment open(Path file, boolean readOnly, Clock clock) {
        try {
            FileChannel channel;
            if (readOnly) {
                channel = FileChannel.open(file, StandardOpenOption.READ);
            } else {
                channel = FileChannel.open(file, StandardOpenOption.CREATE, StandardOpenOption.READ, StandardOpenOption.WRITE);
                channel.position(channel.size());
            }
            return new LogSegment(file, channel, clock);
        } catch (Exception e) {
            throw new KiwiException("Failed to open log segment " + file, e);
        }
    }

    public int append(Record record) throws KiwiWriteException {
        try {
            return channel.write(record.toByteBuffer());
        } catch (IOException | IllegalStateException e) {
            throw new KiwiWriteException("Failed to append record to log segment " + file, e);
        }
    }

    public ByteBuffer read(long position, int size) throws KiwiReadException {
        try {
            ByteBuffer buffer = ByteBuffer.allocate(size);
            channel.read(buffer, position);
            return buffer;
        } catch (IOException | IllegalStateException e) {
            throw new KiwiReadException("Failed to read from log segment " + file, e);
        }
    }

    public long position() throws KiwiReadException {
        try {
            return channel.position();
        } catch (IOException e) {
            throw new KiwiReadException("Failed to get valuePosition of log segment " + file, e);
        }
    }

    public long size() throws KiwiReadException {
        try {
            return channel.size();
        } catch (IOException e) {
            throw new KiwiReadException("Failed to get size of log segment " + file, e);
        }
    }

    public String name() {
        return file.getFileName().toString().replace(EXTENSION, "");
    }

    public Path baseDir() {
        return file.getParent();
    }

    public Path file() {
        return file;
    }

    public void sync() {
        try {
            if (channel.isOpen()) {
                channel.force(true);
            }
        } catch (IOException e) {
            logger.error("Failed to sync log segment {}", file, e);
        }
    }

    public void close() {
        try {
            if (channel.isOpen()) {
                sync();
                channel.close();
            }
        } catch (IOException e) {
            logger.error("Failed to close log segment {}", file, e);
        }
    }

    public void markAsReadOnly() {
        try {
            close();
            channel = FileChannel.open(file, StandardOpenOption.READ);
        } catch (IOException e) {
            throw new KiwiException(e);
        }
    }

    public void markAsDeleted() {
        close();

        String deletedFileName = file.getFileName().toString() + ".deleted";
        Path deletedFile = file.resolveSibling(deletedFileName); // 创建一个新路径

        Utils.renameFile(file, deletedFile);
        logger.info("Marked log segment {} for deletion", file);
    }

    public boolean equalsTo(LogSegment other) {
        if (this == other) {
            return true;
        }
        if (other == null || getClass() != other.getClass()) {
            return false;
        }
        return file.equals(other.file);
    }

    public boolean isSamePath(Path other) {
        return file.equals(other);
    }

    public double dirtyRatio(Map<Bytes, Long> keyTimestampMap) {
        long total = 0;
        long dirtyCount = 0;

        try {
            channel.position(0);
            ByteBuffer buffer = ByteBuffer.allocate(Header.BYTES);
            while (channel.read(buffer) != -1) {
//                flip() 是用于切换模式的，它将缓冲区从写模式切换到读模式。具体来说，flip() 做了以下两件事：
//                position 被设置为 0，表示从缓冲区的开头开始读取数据。
//                limit 被设置为当前的写入位置，表示可读取的数据的结束位置。
                buffer.flip();

                // Skip the checksum.
                buffer.position(buffer.position() + Long.BYTES);

                long timestamp = buffer.getLong();
                long ttl = buffer.getLong();
                int keySize = buffer.getInt();
                int valueSize = buffer.getInt();

                buffer.clear();

                if (ttl > 0 && clock.millis() > ttl) {
                    dirtyCount += 1;

                    // Skip the key and value
                    channel.position(channel.position() + keySize + valueSize);
                } else {
                    ByteBuffer keyBuffer = ByteBuffer.allocate(keySize);
                    channel.read(keyBuffer);

                    Bytes key = Bytes.wrap(keyBuffer.array());
                    // Stale records are considered dirty
                    if (keyTimestampMap.getOrDefault(key, Long.MAX_VALUE) == timestamp) {
                        dirtyCount += 1;
                    }
                    // Skip the value;
                    channel.position(channel.position() + valueSize);
                }
                total += 1;
            }
        } catch (IOException | IllegalStateException e) {
            throw new KiwiReadException("Failed to calculate dirty ratio for log segment " + file, e);
        }

        if (total == 0) {
            return 0;
        }
        return (double) dirtyCount / total;
    }

    public Iterable<Record> getRecords() {
        return () -> new RecordIterator(channel, keyHeader -> true);
    }

    public Iterable<Record> getActiveRecords(Map<Bytes, Long> keyTimestampMap) {
        return () -> new RecordIterator(channel, keyHeader -> isActiveRecord(keyHeader, keyTimestampMap));
    }

    public Map<Bytes, ValueReference> buildKeyDir() {
        String hintPath = file.getFileName().toString().replace(EXTENSION, HintSegment.EXTENSION);
        Path hintFile = file.resolveSibling(hintPath);

        if (Files.exists(hintFile)) {
            try {
                return buildKeyDirFromHint(hintFile);
            } catch (KiwiReadException e) {
                logger.warn("Failed to build keydir from segment hint file {}", hintFile, e);
            }
        }
        return buildKeyDirFromData();
    }

    private Map<Bytes, ValueReference> buildKeyDirFromData() throws KiwiReadException {
        logger.info("Building from segment data file {}", file);

        // Data file format: [checksum:8][timestamp:8][ttl:8][keySize:4][valueSize:4][key:keySize][value:valueSize]
        try {
            channel.position(0);

            Map<Bytes, ValueReference> keyDir = new HashMap<>();
            ByteBuffer buffer = ByteBuffer.allocate(Header.BYTES);
            while (channel.read(buffer) != -1) {
                buffer.flip();

                // Skip the checksum.
                // Checksum validation is done during background compaction.
                buffer.position(buffer.position() + Long.BYTES);

                long timestamp = buffer.getLong();
                long ttl = buffer.getLong();
                int keySize = buffer.getInt();
                int valueSize = buffer.getInt();

                buffer.clear();

                ByteBuffer keyBuffer = ByteBuffer.allocate(keySize);
                channel.read(keyBuffer);

                long valuePosition = channel.position();

                // Skip the value.
                channel.position(valuePosition + valueSize);

                // Skip the record if TTL has expired.
                boolean expired = ttl > 0 && System.currentTimeMillis() > ttl;

                Bytes key = Bytes.wrap(keyBuffer.array());
                if (valueSize > 0 && !expired) {
                    ValueReference valueRef = new ValueReference(this, valuePosition, valueSize, ttl, timestamp);
                    keyDir.put(key, valueRef);
                } else {
                    // Skip expired records and tombstone records.
                    keyDir.put(key, null);
                }
            }
            return keyDir;
        } catch (IOException | IllegalStateException e) {
            throw new KiwiReadException("Failed to build hash table from log segment " + file, e);
        }
    }

    private Map<Bytes, ValueReference> buildKeyDirFromHint(Path hintFile) throws KiwiReadException {
        logger.info("Building keydir from segment hint file {}", hintFile);

        HintSegment hintSegment = HintSegment.open(hintFile, true);
        Map<Bytes, ValueReference> keyDir = new HashMap<>();
        for (Hint hint : hintSegment.getHints()) {
            // Skip the record if TTL has expired
            boolean expired = hint.header().ttl() > 0 && System.currentTimeMillis() > hint.header().ttl();

            if (hint.header().valueSize() > 0 && !expired) {
                ValueReference valueRef = new ValueReference(
                        this,
                        hint.valuePosition(),
                        hint.header().valueSize(),
                        hint.header().ttl(),
                        hint.header().timestamp()
                );
                keyDir.put(hint.key(), valueRef);
            } else {
                // Skip expired record and tombstone
                keyDir.put(hint.key(), null);
            }
        }
        return keyDir;
    }

    private boolean isActiveRecord(KeyHeader keyHeader, Map<Bytes, Long> keyTimestampMap) {
        long ttl = keyHeader.header().ttl();
        if (ttl > 0 && clock.millis() > ttl) {
            return false;
        }
        long timestamp = keyHeader.header().timestamp();
        return keyTimestampMap.getOrDefault(keyHeader.key(), Long.MAX_VALUE) == timestamp;
    }

    private static class RecordIterator implements Iterator<Record> {
        private final FileChannel channel;
        private final Predicate<KeyHeader> predicate;
        private long position;
        private Record nextRecord;

        public RecordIterator(FileChannel channel, Predicate<KeyHeader> predicate) {
            this.channel = channel;
            this.predicate = predicate;
            this.position = 0;
            this.nextRecord = null;
        }


        @Override
        public boolean hasNext() {
            if (nextRecord != null) {
                return true;
            }
            try {
                while (position < channel.size()) {
                    channel.position(position);

                    ByteBuffer headerBuffer = ByteBuffer.allocate(Header.BYTES);
                    if (channel.read(headerBuffer) < Header.BYTES) {
                        return false; // Not enough data for a header
                    }
                    headerBuffer.flip();
                    Header header = Header.fromByteBuffer(headerBuffer);

                    // 下一个 Record 的 position
                    position = channel.position() + header.keySize() + header.valueSize();

                    ByteBuffer keyBuffer = ByteBuffer.allocate(header.keySize());
                    if (channel.read(keyBuffer) < header.keySize()) {
                        return false; // Not enough data for a key
                    }
                    Bytes key = Bytes.wrap(keyBuffer.array());

                    KeyHeader keyHeader = new KeyHeader(key, header);
                    if (predicate.test(keyHeader)) {
                        ByteBuffer valueBuffer = ByteBuffer.allocate(header.valueSize());
                        if (channel.read(valueBuffer) < header.valueSize()) {
                            return false; // Not enough data for a value
                        }
                        valueBuffer.flip();
                        Bytes value = Bytes.wrap(valueBuffer.array());
                        nextRecord = new Record(header, key, value);
                        return true;
                    }
                }
            } catch (IOException ex) {
                throw new KiwiReadException("Failed to read next record from log segment", ex);
            }
            return false;
        }

        @Override
        public Record next() {
            if (!hasNext()) {
                throw new NoSuchElementException();
            }
            Record record = nextRecord;
            nextRecord = null;
            return record;
        }
    }

}
```

### HintSegment.java
```java
public class HintSegment {
    private static final Logger logger = LoggerFactory.getLogger(HintSegment.class);

    public static String EXTENSION = ".hint";
    public static String PARTIAL_EXTENSION = EXTENSION + ".partial";

    private final Path file;
    private final FileChannel channel;

    HintSegment(Path file, FileChannel channel) {
        this.file = file;
        this.channel = channel;
    }

    public static HintSegment open(Path file) {
        return open(file, false);
    }

    public static HintSegment open(Path file, boolean readOnly) {
        try {
            FileChannel channel;
            if (readOnly) {
                channel = FileChannel.open(file, StandardOpenOption.READ);
            } else {
                channel = FileChannel.open(file, StandardOpenOption.CREATE, StandardOpenOption.APPEND);
            }
            return new HintSegment(file, channel);
        } catch (Exception e) {
            throw new KiwiException("Failed to open hint segment " + file, e);
        }
    }

    public int append(Hint hint) throws KiwiWriteException {
        try {
            return channel.write(hint.toByteBuffer());
        } catch (IOException | IllegalStateException e) {
            throw new KiwiWriteException("Failed to write hint to segment " + file, e);
        }
    }

    public void close() {
        try {
            if (channel.isOpen()) {
                channel.force(true);
                channel.close();
            }
        } catch (IOException e) {
            throw new KiwiException("Failed to close hint segment " + file, e);
        }
    }

    public void commit() {
        String fileName = file.getFileName().toString();
        if (!fileName.endsWith(PARTIAL_EXTENSION)) {
            return;
        }
        close();

        String commitedFileName = fileName.replace(PARTIAL_EXTENSION, EXTENSION);
        Path committedFile = file.resolveSibling(commitedFileName);

        Utils.renameFile(file, committedFile);
        logger.info("Marked hint segment {} as committed", file);
    }

    public Iterable<Hint> getHints() {
        return () -> new HintIterator(channel);
    }

    private static class HintIterator implements Iterator<Hint> {
        private final FileChannel channel;
        private long position;
        private Hint nextHint;

        public HintIterator(FileChannel channel) {
            this.channel = channel;
            this.position = 0;
            this.nextHint = null;
        }

        @Override
        public boolean hasNext() {
            if (nextHint != null) {
                return true;
            }

            // Hint file format: [checksum:8][timestamp:8][ttl:8][keySize:4][valueSize:4][valuePosition:8][key:keySize]
            try {
                if (position >= channel.size()) {
                    return false;
                }

                channel.position(position);

                int headerSize = Header.BYTES + Long.BYTES;
                ByteBuffer buffer = ByteBuffer.allocate(headerSize);
                if (channel.read(buffer) < headerSize) {
                    return false; // Not enough data for a header
                }
                buffer.flip();

                long checksum = buffer.getLong();
                long timestamp = buffer.getLong();
                long ttl = buffer.getLong();
                int keySize = buffer.getInt();
                int valueSize = buffer.getInt();
                long valuePosition = buffer.getLong();
                Header header = new Header(checksum, timestamp, ttl, keySize, valueSize);

                buffer.clear();

                ByteBuffer keyBuffer = ByteBuffer.allocate(keySize);
                if (channel.read(keyBuffer) < keySize) {
                    return false; // Not enough data for a key
                }
                Bytes key = Bytes.wrap(keyBuffer.array());

                // Next hint position.
                position = channel.position();

                nextHint = new Hint(header, valuePosition, key);
                return true;
            } catch (IOException ex) {
                throw new KiwiReadException("Failed to read next record from log segment", ex);
            }
        }

        @Override
        public Hint next() {
            if (!hasNext()) {
                throw new NoSuchElementException();
            }
            Hint hint = nextHint;
            nextHint = null;
            return hint;
        }
    }
}
```

### LogCleaner.java
```java
public class LogCleaner implements AutoCloseable {
    private static final Logger logger = LoggerFactory.getLogger(LogCleaner.class);

    private static final double JITTER = 0.3;

    private final Path logDir;
    private final KeyDir keyDir;
    private final Supplier<LogSegment> activeSegmentSupplier;
    private final LogSegmentNameGenerator segmentNameGenerator;
    private final double minDirtyRatio;
    private final long compactionSegmentMinBytes;
    private final long logSegmentBytes;
    private final int threads;
    private final ScheduledExecutorService scheduler;


    public LogCleaner(
            Path logDir,
            KeyDir keyDir,
            Supplier<LogSegment> activeSegmentSupplier,
            LogSegmentNameGenerator segmentNameGenerator,
            double minDirtyRatio,
            long compactionSegmentMinBytes,
            long logSegmentBytes,
            int threads) {
        this.logDir = logDir;
        this.keyDir = keyDir;
        this.activeSegmentSupplier = activeSegmentSupplier;
        this.segmentNameGenerator = segmentNameGenerator;
        this.minDirtyRatio = minDirtyRatio;
        this.compactionSegmentMinBytes = compactionSegmentMinBytes;
        this.logSegmentBytes = logSegmentBytes;
        this.threads = threads;

        this.scheduler = Executors.newSingleThreadScheduledExecutor(NamedThreadFactory.create("cleaner"));

        Runtime.getRuntime().addShutdownHook(new Thread(this::close));
    }

    public void start(Duration interval) {
        if (interval.isZero()) {
            logger.info("Log cleaner is disabled");
            return;
        }

        // Add some jitter to prevent all log cleaners from running at the same time.
        long compactIntervalSeconds = intervalWithJitterSeconds(interval);
        long cleanIntervalSeconds = intervalWithJitterSeconds(interval);

        scheduler.scheduleAtFixedRate(this::compactLog, compactIntervalSeconds, compactIntervalSeconds, TimeUnit.SECONDS);
        scheduler.scheduleAtFixedRate(this::cleanLog, 0, cleanIntervalSeconds, TimeUnit.SECONDS);
    }

    private long intervalWithJitterSeconds(Duration interval) {
        long jitter = (long) (interval.toSeconds() * JITTER);
        // Shift the interval by a random amount between -jitter and +jitter.
        return interval.toSeconds() + (long) ((Math.random() - 0.5) * 2 * jitter);
    }

    void compactLog() {
        logger.info("Log compaction started");

        Map<Bytes, Long> keyTimestampMap = buildKeyTimestampMap();
        List<LogSegment> dirtySegments = findDirtySegments(keyTimestampMap);

        if (dirtySegments.isEmpty()) {
            logger.info("No dirty segments found");
            return;
        }

        List<HintSegment> hintSegments = new ArrayList<>();
        LogSegment newLogSegment = null;
        HintSegment newHintSegment = null;

        for (LogSegment dirtySegment : dirtySegments) {
            for (Record record : dirtySegment.getActiveRecords(keyTimestampMap)) {
                if (newLogSegment == null || newLogSegment.size() >= logSegmentBytes) {
                    // When new segment is full, fsync and close log and hint channels.
                    if (newLogSegment != null) {
                        newLogSegment.close();
                        newHintSegment.close();
                    }

                    Path logFile = segmentNameGenerator.next();
                    newLogSegment = LogSegment.open(logFile);

                    Path hintFile = logFile.resolveSibling(newLogSegment.name() + HintSegment.PARTIAL_EXTENSION);
                    newHintSegment = HintSegment.open(hintFile);
                    hintSegments.add(newHintSegment);

                    logger.info("Opened new compacted log segment {}", newLogSegment.name());
                }

                newLogSegment.append(record);

                long valuePosition = newLogSegment.position() - record.valueSize();
                newHintSegment.append(new Hint(record.header(), valuePosition, record.key()));

                // Prevent keydir from being updated with stale values.
                ValueReference currentValue = keyDir.get(record.key());
                if (currentValue != null && currentValue.timestamp() <= record.header().timestamp()) {
                    keyDir.update(record, newLogSegment);
                }
            }
        }

        if (newLogSegment != null) {
            newLogSegment.close();
            newHintSegment.close();
        }

        for (LogSegment dirtySegment : dirtySegments) {
            // Hint files are first marked as deleted before log files are deleted to prevent data loss.
            // If process fails after hint file is marked as deleted but before log file is deleted,
            // data can be recovered.
            String hintFileName = dirtySegment.name() + HintSegment.EXTENSION;
            Path hintFile = dirtySegment.file().resolveSibling(hintFileName);
            if (Files.exists(hintFile)) {
                Path deletedHintFile = hintFile.resolveSibling(hintFileName + ".deleted");
                Utils.renameFile(hintFile, deletedHintFile);
            }

            dirtySegment.markAsDeleted();
        }

        for (HintSegment hintSegment : hintSegments) {
            hintSegment.commit();
        }

        logger.info("Log compaction ended");
    }

    void cleanLog() {
        try (Stream<Path> paths = Files.walk(logDir)) {
            paths.filter(Files::isRegularFile)
                    .filter(path -> path.getFileName().toString().endsWith(".deleted"))
                    .forEach(path -> {
                        try {
                            Files.deleteIfExists(path);
                            logger.info("Deleted marked segment {}", path);
                        } catch (IOException ex) {
                            logger.warn("Failed to delete segments", ex);
                        }
                    });
        } catch (IOException ex) {
            logger.warn("Failed to clean dirty segments", ex);
        }
    }


    private Map<Bytes, Long> buildKeyTimestampMap() {
        return keyDir.entrySet()
                .stream()
                .collect(Collectors.toMap(Map.Entry::getKey, entry -> entry.getValue().timestamp()));
    }

    private List<LogSegment> findDirtySegments(Map<Bytes, Long> keyTimestampMap) {
        ExecutorService executor = Executors.newFixedThreadPool(threads, NamedThreadFactory.create("compaction"));

        List<LogSegment> dirtySegments = new ArrayList<>();
        try (Stream<Path> paths = Files.walk(logDir)) {
            dirtySegments = paths.filter(Files::isRegularFile)
                    .filter(path -> path.getFileName().toString().endsWith(".log"))
                    .filter(path -> !activeSegmentSupplier.get().isSamePath(path))
                    .map(path -> executor.submit(() -> {
                        try {
                            LogSegment segment = LogSegment.open(path, true);
                            double ratio = segment.dirtyRatio(keyTimestampMap);
                            if (ratio >= minDirtyRatio) {
                                logger.info("Found segment {} with dirty ratio {}", segment.name(), String.format("%.4f", ratio));
                                return segment;
                            } else if (segment.size() < compactionSegmentMinBytes) {
                                // Compact empty or almost empty segments.
                                logger.info("Found small segment {} with {} bytes", segment.name(), segment.size());
                                return segment;
                            } else {
                                return null;
                            }
                        } catch (KiwiReadException ex) {
                            logger.warn("Failed to read log segment {}", path, ex);
                            return null;
                        }
                    }))
                    .map(future -> {
                        try {
                            return future.get();
                        } catch (InterruptedException | ExecutionException ex) {
                            logger.error("Failed to mark dirty segments", ex);
                            return null;
                        }
                    })
                    .filter(Objects::nonNull)
                    .sorted(Comparator.comparing(LogSegment::name))
                    .toList();
        } catch (IOException ex) {
            logger.error("Failed to mark dirty segments", ex);
        }

        executor.shutdown();

        // Prevents infinite compaction loop when only one dirty segment is found.
        if (dirtySegments.size() == 1 && dirtySegments.stream().findFirst().get().size() < compactionSegmentMinBytes) {
            logger.info("Single dirty segment found with {} bytes. Skipping compaction.", dirtySegments.stream().findFirst().get().size());
            dirtySegments = Collections.emptyList();
        }

        return dirtySegments;
    }

    @Override
    public void close() {
        logger.info("Shutting down log cleaner");
        scheduler.shutdown();
        try {
            if (!scheduler.awaitTermination(3, TimeUnit.MINUTES)) {
                logger.warn("Log cleaner did not shutdown in time. Forcing shutdown.");
                scheduler.shutdownNow();
            }
        } catch (InterruptedException ex) {
            logger.error("Failed to shutdown log cleaner", ex);
            scheduler.shutdownNow();
        }
    }
}
```

完整代码： https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-bitcask  

refer     
1.https://riak.com/assets/bitcask-intro.pdf         
2.https://arpitbhayani.me/blogs/bitcask        
3.https://docs.riak.com/riak/kv/2.2.3/setup/planning/backend/bitcask.1.html#merge-policy         
4.https://github.com/nemanja-m/kiwi       
5.http://xinzhuxiansheng.com/articleDetail/85             
