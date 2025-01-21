# JAVA - Memory-mapped I/O - 探索給 BigArray 加锁     

>JDK version: 1.8  
>refer bigqueue：https://github.com/bulldog2011/bigqueue       
>Kafka Version: 3.6.0, RocketMQ Version: 4.9.5 

## 引言     
上一篇 Blog `JAVA - Memory-mapped I/O - 探索构造 BigArray 读写 File 篇`中`BigArrayImpl#append()`方法写入数据，没有对并发写入做处理，`It is really a hot potato.`,接下来，继续探索`Append-only files` 可能涉及到的 Lock。     

## 引入锁    
`Append-only files` 强调数据写入是追加方式，在多线程`并发操作`时，会存在数据不一致的情况，所以，写入数据时，需要加一把锁。 

接下来，我们看下 Kafka，RocketMQ，bigqueue 对于锁的使用。             

### Kafka Lock   
在 `抽象类 AbstractIndex` 定义了 `ReentrantLock 类型的 lock`，`OffsetIndex#append()`方法写入 offset 数据时，调用`lock.lock()`加锁，在 finally 语法块中调用`lock.unlock`。 

`下面是涉及 Lock 部分的代码片段`   

**AbstractIndex**
```java
protected final ReentrantLock lock = new ReentrantLock();
```

**OffsetIndex** 
```java
public void append(long offset, int position) {
    lock.lock();
    try {
        if (isFull())
            throw new IllegalArgumentException("Attempt to append to a full index (size = " + entries() + ").");

        if (entries() == 0 || offset > lastOffset) {
            log.trace("Adding index entry {} => {} to {}", offset, position, file().getAbsolutePath());
            mmap().putInt(relativeOffset(offset));
            mmap().putInt(position);
            incrementEntries();
            lastOffset = offset;
            if (entries() * ENTRY_SIZE != mmap().position())
                throw new IllegalStateException(entries() + " entries but file position in index is " + mmap().position());
        } else
            throw new InvalidOffsetException("Attempt to append an offset " + offset + " to position " + entries() +
                " no larger than the last offset appended (" + lastOffset + ") to " + file().getAbsolutePath());
    } finally {
        lock.unlock();
    }
}
```

>Fluss 也有部分代码实现是借鉴 Kafka。           

### RocketMQ Lock       
RocketMQ 与 Kafka 关于 Lock 有一半的不同，它定义了一个`PutMessageLock interface`, 它拥有两个实现类 `PutMessageReentrantLock` 和 `PutMessageSpinLock`，并且可在 `broker.conf`配置文件中，通过参数`useReentrantLockWhenPutMessage=true`来修改写入数据时，使用哪种锁。         

* PutMessageReentrantLock 使用 `ReentrantLock`, 这部分与 Kafka的`AbstractIndex` 是一样的，并且都是非公平锁。   
* PutMessageSpinLock 使用 `AtomicBoolean` 实现自旋锁。  
![bigarrayaddlock01](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock01.png)      

>对于选哪种锁可参考下面是官网的一段介绍：from `https://rocketmq.io/learning/explore/rocketmq_learning-gvr7dx_awbbpb_dd4p22sg52gbcg6g/`      
```bash
独占锁实现顺序写

如何保证单机存储写 CommitLog 的顺序性，直观的想法就是对写入动作加独占锁保护，即同一时刻只允许一个线程加锁成功，那么该选什么样的锁实现才合适呢？RocketMQ 目前实现了两种方式。
1. 基于 AQS 的 ReentrantLock
2. 基于 CAS 的 SpinLock

那么什么时候选取 spinlock，什么时候选取 reentranlock？回忆下两种锁的实现，对于 ReentrantLock，底层 AQS 抢不到锁的话会休眠，但是 SpinLock 会一直抢锁，造成明显的 CPU 占用。SpinLock 在 trylock 失败时，可以预期持有锁的线程会很快退出临界区，死循环的忙等待很可能要比进程挂起等待更高效。这也是为什么在高并发下为了保持 CPU 平稳占用而采用方式一，单次请求响应时间短的场景下采用方式二能够减少 CPU 开销。两种实现适用锁内操作时间不同的场景，那线程拿到锁之后需要进行哪些动作呢？

    预计算索引的位置，即 ConsumeQueueOffset，这个值也需要保证严格递增。
    计算在 CommitLog 存储的位置，physicalOffset 物理偏移量，也就是全局文件的位置。
    记录存储时间戳 storeTimestamp，主要是为了保证消息投递的时间严格保序。

因此不少文章也会建议在同步持久化的时候采用 ReentrantLock，异步持久化的时候采用 SpinLock。那么这个地方还有没有优化的空间？目前可以考虑使用较新的 futex 取代 spinlock 机制。futex 维护了一个内核层的等待队列和许多个 SpinLock 链表。当获得锁时，尝试 cas 修改，如果成功则获得锁，否则就将当前线程 uaddr hash 放入到等待队列 (wait queue)，分散对等待队列的竞争，减小单个队列的长度。这听起来是不是也有一点点 concurrentHashMap 和 LongAddr 的味道，其实核心思想都是类似的，即分散竞争。     
```

`下面是涉及 Lock 部分的代码片段`     

**PutMessageLock** 
```java
/**
 * Used when trying to put message
 */
public interface PutMessageLock {
    void lock();

    void unlock();
}
```

**PutMessageReentrantLock**  
```java
/**
 * Exclusive lock implementation to put message
 */
public class PutMessageReentrantLock implements PutMessageLock {
    private ReentrantLock putMessageNormalLock = new ReentrantLock(); // NonfairSync

    @Override
    public void lock() {
        putMessageNormalLock.lock();
    }

    @Override
    public void unlock() {
        putMessageNormalLock.unlock();
    }
}
```

**PutMessageSpinLock**
```java
/**
 * Spin lock Implementation to put message, suggest using this with low race conditions
 */
public class PutMessageSpinLock implements PutMessageLock {
    //true: Can lock, false : in lock.
    private AtomicBoolean putMessageSpinLock = new AtomicBoolean(true);

    @Override
    public void lock() {
        boolean flag;
        do {
            flag = this.putMessageSpinLock.compareAndSet(true, false);
        }
        while (!flag);
    }

    @Override
    public void unlock() {
        this.putMessageSpinLock.compareAndSet(false, true);
    }
}
```

**CommitLog**
```java
protected final PutMessageLock putMessageLock;

public CommitLog(final DefaultMessageStore defaultMessageStore) {
    省略部分代码 ......
    this.putMessageLock = defaultMessageStore.getMessageStoreConfig().isUseReentrantLockWhenPutMessage() ? new PutMessageReentrantLock() : new PutMessageSpinLock();
    省略部分代码 ......
}


public CompletableFuture<PutMessageResult> asyncPutMessage(final MessageExtBrokerInner msg) {
    省略部分代码 ......
    putMessageLock.lock(); //spin or ReentrantLock ,depending on store config
    try {
        省略部分代码 ......
    } finally {
        beginTimeInLock = 0;
        putMessageLock.unlock();
    }
    省略部分代码 ......
}
```

## Bigqueue Lock        
在 bigqueue 项目中 (https://github.com/bulldog2011/bigqueue), `BigArrayImpl#append()` 分别使用了2种锁，`ReentrantReadWriteLock`和`ReentrantLock`，除了`ReentrantReadWriteLock`读写锁部分，对于`ReentrantLock`使用与 Kafka，RocketMQ 是一样的。  

`下面是涉及 Lock 部分的代码片段`            

**BigArrayImpl**
```java
// lock for appending state management
final Lock appendLock = new ReentrantLock();

// global lock for array read and write management
final ReadWriteLock arrayReadWritelock = new ReentrantReadWriteLock();
final Lock arrayReadLock = arrayReadWritelock.readLock();
final Lock arrayWriteLock = arrayReadWritelock.writeLock(); 

/**
 * Append the data into the head of the array
 */
public long append(byte[] data) throws IOException {
    try {
        arrayReadLock.lock(); 
        省略部分代码 ...
        try {
            appendLock.lock(); // only one thread can append
          省略部分代码 ...
        } finally {
            
            appendLock.unlock();
            省略部分代码 ...
        }
        return toAppendArrayIndex;
    
    } finally {
        arrayReadLock.unlock();
    }
}
```

### 小结 
看来，`ReentrantLock` 是它们的首选，`ReentrantLock`默认是非公平锁，相较于公平锁 `new ReentrantLock(true)` 来说，其优点是执行效率高，谁先获取到锁，锁就属于谁，不会按线程先后顺序执行，它的缺点是资源分配随机性强，可能会出现线程饿死的情况。  

## Lock 的作用域        
在 bigqueue 项目，`BigArrayImpl#append()`方法写入3个 File MappedByteBuffer，分别是 `Data MappedByteBuffer`,`Index MappedByteBuffer`,`Meta_data MappedByteBuffer`。如下图所示，`appendLock.lock()` 将3个写入封装在一个锁的作用域中,保证了数据写入的完整性。 可 Kafka，RocketMQ也是这样么？  `这部分值得深思!!!` RocketMQ 与 Kafka 的架构有较大的不同，它有一个`Dispatch`组件，负责消息再分发。
![bigarrayaddlock03](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock03.png)    

### 思考 Kafka Lock 作用域    
`LogSegment#append()`方法通过`@nonthreadsafe`标记该方法不是线程安全的，需要调用方来保证线程安全，这部分代码看似简单，但其实考验大伙对`index file`是否了解。`val appendedBytes = log.append(records)` 方法通过`FileChannel`写入数据后，但 `offsetIndex`,`timeIndex`是否追加索引数据，并不是每次都插入，它的判断条件`if (bytesSinceLastIndexEntry > indexIntervalBytes) `，如果当前累计追加的数据字节数超过阈值则记录索引，这里有个背景：Kafka 并不会每条消息建立索引，而是通过`稀疏索引`的策略间隔大小的字节数来构建索引，配置项是`log.index.interval.bytes 或者 index.interval.bytes`（默认值是 4096 = 4kb）注意，该参数在 Broker，Topic 两个 config 都支持配置, 关于`log.index.interval.bytes`参数可参考下图：        
![bigarrayaddlock04](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock04.png)    

当数据累计写入字节大小达到4kb,才会在`index file`追加一个 entry, 若某个 Topic 的消息数偏大 > 4kb，可以适当调节 `index.interval.bytes`参数，减少 `index file`写入次数。 下面是`LogSegment#append()`的代码逻辑：         

**kafka.log.LogSegment#append()**   
```java
@nonthreadsafe
def append(largestOffset: Long,
            largestTimestamp: Long,
            shallowOffsetOfMaxTimestamp: Long,
            records: MemoryRecords): Unit = {
if (records.sizeInBytes > 0) {
    trace(s"Inserting ${records.sizeInBytes} bytes at end offset $largestOffset at position ${log.sizeInBytes} " +
        s"with largest timestamp $largestTimestamp at shallow offset $shallowOffsetOfMaxTimestamp")
    val physicalPosition = log.sizeInBytes()
    if (physicalPosition == 0)
    rollingBasedTimestamp = Some(largestTimestamp)

    ensureOffsetInRange(largestOffset)

    // append the messages
    val appendedBytes = log.append(records)
    trace(s"Appended $appendedBytes to ${log.file} at end offset $largestOffset")
    // Update the in memory max timestamp and corresponding offset.
    if (largestTimestamp > maxTimestampSoFar) {
    maxTimestampAndOffsetSoFar = new TimestampOffset(largestTimestamp, shallowOffsetOfMaxTimestamp)
    }
    // append an entry to the index (if needed)
    if (bytesSinceLastIndexEntry > indexIntervalBytes) {
    offsetIndex.append(largestOffset, physicalPosition) 
    timeIndex.maybeAppend(maxTimestampSoFar, offsetOfMaxTimestampSoFar)
    bytesSinceLastIndexEntry = 0
    }
    bytesSinceLastIndexEntry += records.sizeInBytes
}
}
```     

### 思考 RocketMQ Lock 作用域   
RocketMQ 的 `consumequeue file`,`index file`的数据写入与`commitlog file`的写入并不是同步的，RocketMQ 会启动创建一个名为 `ReputMessageService.class.getSimpleName()` 的`ReputMessageService`线程来 `异步` 写入 `consumequeue file` 和 `index file`。是的，它的 `Dispatch`组件架构很大部分决定了这样了设计。根据 Kafka 稀疏索引写入的经验，那 RokcetMQ 又有什么特殊性呢？   有的。     

`index file`是为了提供可以根据 `key` 进行消息查询所构造的索引文件，那如果 Producer 发送的消息不包含 `key`，则不会写入。参考下图查看 index 的Dispatch,在 `org.apache.rocketmq.store.index.IndexService#buildIndex()`方法中判断 keys是否非空并且长度需大于0，才会写入索引数据。   

![bigarrayaddlock05](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock05.png)   

**org.apache.rocketmq.store.index.IndexService#buildIndex()**
```java
if (keys != null && keys.length() > 0) {
    String[] keyset = keys.split(MessageConst.KEY_SEPARATOR);
    for (int i = 0; i < keyset.length; i++) {
        String key = keyset[i];
        if (key.length() > 0) {
            indexFile = putKey(indexFile, msg, buildKey(topic, key));
            if (indexFile == null) {
                log.error("putKey error commitlog {} uniqkey {}", req.getCommitLogOffset(), req.getUniqKey());
                return;
            }
        }
    }
}
```

以上探讨中涉及到的 Kafka，RocketMQ `存储模型`是真很值得学习, `But` 如果出现下图所示的情况怎么办？   
![bigarrayaddlock06](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock06.png)  

目前我们探索的范围还不是`Cluster Wide`,仅是一个单节点，可能你也会说这种情况毕竟少见，但是在`机器宕机`或者`kill -9`情况下多个文件写入进度不协调是真实存在的。那又该怎么办呢？   

大家要是用过`Elasticsearch`，它有个`分片索引重建`功能，按照这个思路继续推演，例如 bigqueue 中 `data file` 能重新推导出 `index file`,`meta_data file`，那是不是就可以不太担心上述极端异常情况。      
![bigarrayaddlock07](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock07.png) 

但极端异常的处理逻辑总不能每次 broker 启动都触发重新构建吧？ Kafka，RocketMQ 对极端异常情况也都做了处理，下面就简单介绍下，该篇 Blog 暂不过多做补充，后面 Blog 会详细补充实现细节。       

**Kafka**  
Kafka Broker 启动时，会对 LogManager 进行初始化，在过程中会完成相关的恢复操作和 Log 加载，首先调用`LogManager#createAndValidateLogDirs()`方法保证 `${log.dir}`目录下的 log 都存在并且可读，之后会调用 `LogManager#loadLogs()` 方法加载 log 目录下的所有 Log。 整个过程中会检查 Broker 上次是否是正常关闭，并设置 Broker 的状态。在Broker正常关闭时，会创建一个`.kafka_cleanshutdown` 的文件，这里就是通过此文件进行判断的。   

![bigarrayaddlock08](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock08.png)    

```bash
[root@vm01 consumequeue]# find  / -name  .kafka_cleanshutdown
/var/lib/docker/overlay2/e247052e5ac9ec2fcb44ef45e6cc7d3aca8edc41912baf7c03cd6207f3350c8a/diff/tmp/kraft-combined-logs/.kafka_cleanshutdown
[root@vm01 consumequeue]#
```

对于 `log file`重建相应的 `index file`和 `timeindex file` 可阅读`kafka.log.LogSegment#recover()`方法。     

**RocketMQ**
RocketMQ 与 Kafka 处理逻辑也有几分相似 ，RocketMQ 中的`abort file`是 Broker是否异常关闭的标志。正常关闭时该文件会被删除，异常关闭则不会执行。 当 Broker 重新启动时，根据是否异常宕机决定是否需要重新构建 `index file`，`consumequeue file`。  

对于 `log file`重建相应的 `index file`和 `consumequeue file` 可阅读`DefaultMessageStore#recover()`方法。 

这让我意识到，对于 `存储`系统来说，写好 `Runtime.getRuntime().addShutdownHook()`真的特别必要，看似加锁动作，但对于保证数据一致性真的远远不够。   

>上面的探索都是`基于进程内`，那`进程外`是否就安全了呢？     

下面，我们接着分析进程外的，大家可别发散到磁盘矩阵这些，代码范畴即可。     

## FileLock    
![bigarrayaddlock02](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock02.png)  

关于 `FileLock` 在我之前的 Blog `Flink SQL - SQL Client - 探索 CLI 的实现逻辑 - 读取 SQL`中的`启动 Embedded Gateway,第一个 create()`章节中有介绍它的作用，在`Append-only files`场景下，`ReentrantLock`保障了每次写入时，只能有一个线程成功写入，而其他则阻塞等待。 那`FileLock`可以保障某些目录不能同时被多个 JVM 进程写入，下面以 RocketMQ为示例：         
![bigarrayaddlock09](http://img.xinzhuxiansheng.com/blogimgs/java/bigarrayaddlock09.png)   

**org.apache.rocketmq.store.DefaultMessageStore#start()**  
```java
public void start() throws Exception {

    lock = lockFile.getChannel().tryLock(0, 1, false);
    if (lock == null || lock.isShared() || !lock.isValid()) {
        throw new RuntimeException("Lock failed,MQ already started");
    }
    
    省略部分代码 ......
```

`lockFile.getChannel()`方法获取文件的 `FileChannel`，它是操作文件的核心对象，`tryLock(0, 1, false)`方法尝试将对文件加锁，`0`的含义是从文件的第0个字节开始加锁。`1`表达是锁住1个字节。`false`表达获取的锁是独占锁，如果设置`true`，则获取的是共享锁。  

* `tryLock` 返回 `null`：表示文件锁获取失败，可能已经被其他进程锁定。                  
* `lock.isShared()`：表示当前获得的是共享锁，而非独占锁，与目标不符。           
* `!lock.isValid()`：表示获取到的锁无效，可能因为文件已经关闭或锁已被释放。                 

通过文件锁确保不同进程的操作是互斥的，保证了同一时刻有且只有一个实例独占使用。      


## 总结     
`数据`总是脆弱的，经过上面探索分析，我们需要`ReentrantLock 加锁`,`WAL机制 可推导修复其他 files`，`FileLock 保证跨进程之间的数据占用` 等等。 

refer     
1.https://github.com/bulldog2011/bigqueue       
2.https://github.com/apache/rocketmq/issues/3948    
3.https://rocketmq.io/learning/explore/rocketmq_learning-gvr7dx_awbbpb_dd4p22sg52gbcg6g/            
4.https://www.cnblogs.com/xjwhaha/p/15772846.html        
