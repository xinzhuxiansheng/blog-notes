# JAVA - Memory-mapped I/O - 探索构造 BigArray 读写 File 篇  

>JDK version: 1.8  
>refer bigqueue：https://github.com/bulldog2011/bigqueue       

## 引言     
`Append-only files`是该篇 Blog 实践内容的核心思想，它呼应上一篇 Blog 内容 `了解内存映射 Memory-mapped 和 MappedByteBuffer (它值得你去花时间)`(https://mp.weixin.qq.com/s/QUG7bYxXTiSfKqc5J-kMXw)。                     

**In memory-mapped I/O, both memory and I/O devices use the same address space**. We assign some of the memory addresses to I/O devices. The CPU treats I/O devices like computer memory. The CPU either communicates with computer memory or some I/O devices depending on the address. Therefore, we reserve a part of the address space for I/O devices, which is not available for computer memory.                    

就如下图, 该篇 Blog 不会讨论 `Where do Memory-mapped I/O addresses come from`, 我们需要是`Just do it`。 (ps: 小红书老外附体 :)     
![bigarray01](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray01.png)      

接下来，探索构造 BigArray 读写 File 篇。 

## 结构介绍  
为了更好理解它的结构，我们先单测开始，这样可以更好理解它要做的事。            
```java
public class TestBigArray {
    final String storageDir = "E:\\Code\\Java\\javamain-services\\broker";

    @Test
    public void testSizeBigArray() throws IOException {
        IBigArray bigArray = new BigArrayImpl(storageDir, "array");

        for (int i = 0; i < 10; i++) {
            String item = String.valueOf(i);
            long index = bigArray.append(item.getBytes());
        }

        long size = bigArray.size();
        Assertions.assertEquals(10, size);
    }

    @Test
    public void testGetBigArray() throws IOException, InterruptedException {
        IBigArray bigArray = new BigArrayImpl(storageDir, "array");
        long size = bigArray.size();
        System.out.println("size: " + size);
        for (int i = 0; i < 10; i++) {
            byte[] bytes = bigArray.get(i);
            System.out.println(new String(bytes));
        }
        TimeUnit.SECONDS.sleep(10);
    }
}
```

![bigarray02](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray02.png)        

首先指定数据存储的目录，以及子目录名称，通过路径拼接，做到 `目录隔离`，例如`E:\Code\Java\javamain-services\broker\`+`array`(如下图) 。因为是 `Append-only files` 方式,所以，`bigArray` 调用 append()方法仅是追加，注意它操作的是byte[], size()方法返回的是 bigArray 的长度，get()方法根据下标获取追加的数据。   

![bigarray03](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray03.png)

通过 `testGetBigArray()` 测试方法，可得到之前通过 `testSizeBigArray()` 测试方法追加的10条数据。很明显它不像 Java Array类型存在内存中。 当 array 需要存储较大数据时，若直接放入内存，会增加 JVM 堆内存和GC 的压力，而使用 Memory-mapped I/O 方式处理它，排除了内存压力以及它的处理效率也会接近内存使用效率。    

## File 设计   
首先 file 存储的结构如下：      
```bash
E:\Code\Java\javamain-services\broker>tree /F
Folder PATH listing
Volume serial number is B298-123C
E:.
└─array
    ├─data
    │      page-0.dat
    │
    ├─index
    │      page-0.dat
    │
    └─meta_data
            page-0.dat
```

![bigarray04](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray04.png)    

* `array/data` 数据目录，用于存放数据 
* `array/index` 索引目录，用于记录 `array/data` 目录下存储的每条数据的起始 offset，还包括数据所属的文件，追加时间戳等。               
* `array/meta_data` 属性目录，用于记录 bigArray 的 size。  

### 01 文件命名 - 设计
`文件命名`，array/data、array/index、array/meta_data 目录下的文件是以`page-[index].dat`命名的，其中`index`是文件的下标，下面是 fileName 生成的流程：        
![bigarray06](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray06.png)  

从开始定义 file 的 mapped size，就已经定义好了 file 的存储大小，当前 file 剩余空间无法容纳当前数据大小时，需要创建新 file，并且对新文件做`内存映射`，此时根据上一个文件的 index + 1，来命名新文件的 fileName。 这部分与 Kafka，RocketMQ是有些区别，该篇仅仅是通过简单设计来实践，在生产服务中我更倾向于 Kafka，RocketMQ 它们的设计。    

在`Kafka`，它通过 file 的后缀来定义它的存储的数据，`.index`,`.log`等等，如下图：         
![bigarray07](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray07.png)  

在`RocketMQ`,它通过 file offset的起始位置来命名，如下图：               
![bigarray08](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray08.png)  

RocketMQ 的 topic file 默认的mapped size为 `1G = 1073741824 byte`,默认第一个文件 file offset 为 0，那第二个文件就是 `1073741824` 开始，这样的命名的好处，可减少 index file 写入的数据，后面会再介绍。  

>这部分，不知道大家是否也跟我刚接触时，有一个同样的疑问?, 为啥 segment 不管是 Kafka，RokcetMQ，都喜欢设置 1G，若为了减少 segment 滚动生成，那就把 segment 设置大一些，例如 4G，哈哈，若将 segment 设置过大，当file 通过 `内存映射`，会占用堆外内存，因为 file 都是追加写入，所以针对较老的数据，若不频繁读取，则多余的 memory-mapped I/O 占用的堆外内存会占用机器资源，若需要对老数据读取时，若file size 小，再重新经过 `内存映射` 则 mapped size 就会变小，这显然降低了资源浪费，所以 segment 不宜过大。 但是为啥设置 1G，博主也不懵逼。 (以1G 为单位，好与机器资源成倍数，1G也不算太小)。    

### 02 数据存储 - 设计   
![bigarray05](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray05.png)      

**1.array/data**: 每次的 append()，直接追加到 `Data MappedByteBuffer`, 没有过多设计   
![bigarray09](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray09.png)                

**2.array/index**: 把它理解成`array/data`的目录，每个`index item`是由`page index`、`data item offset`、`data length`,`currentTime`,`预留空间`，一共占用 32个字节。  

而`page index`、`data item offset`、`data length`,`currentTime` 一起占用 24个字节。     

![bigarray10](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray10.png)  

`page index` 记录 data 存储在哪个`page-[index].dat`下。                
`data item offset` 记录 data 写入时的起始 offset。              
`data length` 记录 data 长度，有了 data item offset + data length，就可以从`Data MappedByteBuffer` 读取数据。             
`currentTime` 记录 data 写入时的时间戳。      
剩余 8个字节作为 预留空间。 

>针对预留空间的做法，我很少会遇到这样处理，因为，浪费了存储空间，可以添加`version` 字段，来针对不同版本格式的数据做处理，就像 `Kafka Message`中的魔数一样，它分别支持 V0,V1,V2格式的数据。 当然，这部分会存在消息解压转换高版本格式后再存储。  

当然 bigqueue 的作者是将 index file 的 mapped size 取整为 4M，后面会介绍。       

**3.array/meta_data** 比较简单，因为它仅包含2个值，`head index`、`tail index`, 并且数据写入不是追加，而是每次指定 MappedByteBuffer 的 position为 0,再写入，所以它仅占用 16个字节大小的长度。        

![bigarray11](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray11.png)  

`head index` 记录 `Data MappedByteBuffer`,`Index MappedByteBuffer` 追加的次数。              
`tail index` 记录 bigArray 的 起始小标，默认是 0。          

>接下来非常重要：我们先回顾下 MappedByteBuffer 的读取方法： 
```java
public byte[] readContent(int pos, int length) {
    ByteBuffer readBuf = readByteBuffer.slice();
    readBuf.position(pos);
    byte[] readBytes = new byte[length];
    readBuf.get(readBytes);
    return readBytes;
}
```  

通过 `readContent()` 可知晓 MappedByteBuffer 读取必须有2个信息，一个是 position，一个是 length。 `Data MappedByteBuffer`读取所需的2个信息，是从`Index MappedByteBuffer`解析后得到的，那在读取`Index MappedByteBuffer`数据时，又如何知道 position，关于 length，它是固定 32个长度字节。         
 
![bigarray13](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray13.png)       

答案就在`Meta_data MappedByteBuffer`，假设我们需要获取下标为 `i`的数据，bigArray.get(i), 在 `Meta_data file` 中存储 bigArray Size `head index`,因为 Index file Offset 是从 0 开始，所以 

### 03 Index File 查找 设计 
当数据通过 Memory-mapped I/O 写入本地 file（data，index，meta_data）, 那下次启动时，需要将文件通过 `内存映射`加载，`注意,那我们默认加载哪些文件呢`,答案肯定是`上一次写入的文件`。 

`array/data`和`array/index`这两个文件夹包含多个文件，而`array/meta_data`则是单文件。            
![bigarray14](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray14.png)  

所以 `array/data`和`array/index`加载`上一次写入的文件`是由`array/meta_data/page-0.dat`中的`head index`推导而来。    

>重点：一定是上一次写入的文件，而不是即将写入的文件，因为检查file 是否写满，需不需要滚动新的file，是触发在 append() 方法中的。  

首先，bigqueue的作者给 index file做了一些设置：         
* 追加次数不能超过 2^17 = 1024*128 = 131072  
* index item 的固长为 32 字节，所以 每个 index file 它对应的 mapped size 为 4m。   

有了上面条件约束，那计算 Index File 的 `page index`，以及`page offset`。

![bigarray16](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray16.png) 

它的计算公式是：           
```java
long previousIndex = this.arrayHeadIndex.get() - 1;
// 计算上一次 page index
previousIndexPageIndex = Calculator.div(previousIndex, INDEX_ITEMS_PER_PAGE_BITS);  
// 加载 page mapped  
previousIndexPage = this.indexPageFactory.acquirePage(previousIndexPageIndex); 
// 计算上一次的 offset 
int previousIndexPageOffset = (int) (Calculator.mul(Calculator.mod(previousIndex, INDEX_ITEMS_PER_PAGE_BITS), INDEX_ITEM_LENGTH_BITS));
```

>该处也同样重要，因为，append()数据时，获取当前写入的 Index file `page index`是根据 `head index`, 而在 get(i) 获取读取 Index file`page index`是根据传入的 i，这里 i，它的下标默认从0 开始，所以，它不需要 -1。 `这非常重要!!!`   

![bigarray17](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray17.png)    

接下来，我们进入代码实践环节，下面部分对 bigqueue代码做了部分取舍，因为该篇 Blog 主要内容是对 File 的读写操作的介绍，针对`并发加锁处理`，`删除数据`等操作不涉及，后面会逐渐整理出来缺失的功能实现，这样 Blog 介绍的内容会有层次感，细节还原度高，希望读者可以更好的接收它。        

## interface 设计      
**项目结构**        
![bigarray18](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray18.png) 
**类图**  
![bigarray12](http://img.xinzhuxiansheng.com/blogimgs/java/bigarray12.png)      
  
## 代码实现细节     
>我知道贴代码，的确会不少读者恼火，因为它破坏了 M端阅读 Blog 的`可读性`，但我还是想着它是一个完整的 Demo，Blog 最后我也会贴出涉及到的代码地址。 若有其他疑问，也欢迎留私信讨论。        

`MappedPageImpl` 和 `MappedPageFactoryImpl` 两个实现类已经通过 Memory-mapped I/O 方式加载 Files。 但是直接操作（data，index，meta_data）MappedByteBuffer 放在了`BigArrayImpl`。     

>注意点：在 MappedPageImpl 有一些不同点，`MappedPageImpl#setPosReturnSliceBuffer()`方法返回的是 ByteBuffer的切片（slice()），它会创建一个新的 ByteBuffer，它与原始的缓冲区共享相同的底层字节数组。新创建的缓冲区是原始缓冲区的一个`视图`，这意味着对切片的修改（如读取或写入）会影响原始缓冲区，反之亦然。然而，`切片会有自己的 position、limit 和 capacity`，这使得它能够在某些操作中表现为独立的缓冲区。所以，我们读取完数据，并不需要单独释放内存。     

目前读取操作使用 slice()足以，而 bigqueue 使用的是 duplicate(),而`duplicate() 会复制原缓冲区的 position 和 limit，但不影响原缓冲区`。这与 slice() `大不同`。  

在涉及到 MappedByteBuffer 写入数据时，我们总要考虑是否强制写入，就像 RocketMQ 配置的那样。 当然这里仅仅是一个 Demo ，可能不太需要把它做成参数控制，但咱们得知道它。     

下一个关注点在 `BigArrayImpl`的属性, 定义 files 存储目录，设置 MappedByteBuffer 和 读写 MappedByteBuffer 计算相关 position，offset的 常量 等。  

### IMappedPage  
```java
public interface IMappedPage {
    ByteBuffer setPosReturnSliceBuffer(int pos);
    ByteBuffer setPosReturnSelfBuffer(int pos);
    byte[] readContent(int pos, int length);
}
```

### IMappedPageFactory  
```java
public interface IMappedPageFactory {
    IMappedPage acquireMappedPage(long index) throws IOException;
    void flush();
    void close();
}
```

### MappedPageImpl
```java
public class MappedPageImpl implements IMappedPage {

    private String fileFullPath;
    private FileChannel fileChannel;
    private MappedByteBuffer mappedByteBuffer;
    private ByteBuffer readByteBuffer;

    public MappedPageImpl(String fileFullPath, FileChannel fileChannel, MappedByteBuffer mappedByteBuffer) {
        this.fileFullPath = fileFullPath;
        this.fileChannel = fileChannel;
        this.mappedByteBuffer = mappedByteBuffer;
        this.readByteBuffer = mappedByteBuffer.slice();
    }

    public byte[] readContent(int pos, int length) {
        ByteBuffer readBuf = readByteBuffer.slice();
        readBuf.position(pos);
        byte[] readBytes = new byte[length];
        readBuf.get(readBytes);
        return readBytes;
    }

    /**
     * 仅用于读取
     *
     * @param pos
     * @return
     */
    public ByteBuffer setPosReturnSliceBuffer(int pos) {
        ByteBuffer readBuf = readByteBuffer.slice();
        readBuf.position(pos);
        return readBuf;
    }

    public ByteBuffer setPosReturnSelfBuffer(int pos) {
        mappedByteBuffer.position(pos);
        return mappedByteBuffer;
    }

    public void flush(){
        mappedByteBuffer.force();
    }

    /**
     * 释放mmap内存占用
     */
    public void clean() {
        if (mappedByteBuffer == null || !mappedByteBuffer.isDirect() || mappedByteBuffer.capacity() == 0)
            return;
        invoke(invoke(viewed(mappedByteBuffer), "cleaner"), "clean");
    }

    private Object invoke(final Object target, final String methodName, final Class<?>... args) {
        return AccessController.doPrivileged(new PrivilegedAction<Object>() {
            public Object run() {
                try {
                    Method method = method(target, methodName, args);
                    method.setAccessible(true);
                    return method.invoke(target);
                } catch (Exception e) {
                    throw new IllegalStateException(e);
                }
            }
        });
    }

    private Method method(Object target, String methodName, Class<?>[] args)
            throws NoSuchMethodException {
        try {
            return target.getClass().getMethod(methodName, args);
        } catch (NoSuchMethodException e) {
            return target.getClass().getDeclaredMethod(methodName, args);
        }
    }

    private ByteBuffer viewed(ByteBuffer buffer) {
        String methodName = "viewedBuffer";
        Method[] methods = buffer.getClass().getMethods();
        for (int i = 0; i < methods.length; i++) {
            if (methods[i].getName().equals("attachment")) {
                methodName = "attachment";
                break;
            }
        }

        ByteBuffer viewedBuffer = (ByteBuffer) invoke(buffer, methodName);
        if (viewedBuffer == null)
            return buffer;
        else
            return viewed(viewedBuffer);
    }
}
```  

### MappedPageFactoryImpl
```java
public class MappedPageFactoryImpl implements IMappedPageFactory {

    /**
     * 存储目录
     */
    private String storageDirectoryPath;
    private File storageDirectoryFile;
    private String filePrefix;
    private int mappedSize;

    private Map<Long, MappedPageImpl> cache;

    public MappedPageFactoryImpl(String storageDirectoryPath, int mappedSize) throws FileNotFoundException {
        this.storageDirectoryPath = storageDirectoryPath;
        this.mappedSize = mappedSize;

        storageDirectoryFile = new File(storageDirectoryPath);
        if (!storageDirectoryFile.exists()) {
            storageDirectoryFile.mkdirs();
        }
        if (!this.storageDirectoryPath.endsWith(File.separator)) {
            this.storageDirectoryPath += File.separator;
        }
        this.filePrefix = this.storageDirectoryPath + BrokerConstants.PAGE_FILE_NAME + "-";
        cache = new HashMap<>();
    }

    public IMappedPage acquireMappedPage(long index) throws IOException {
        MappedPageImpl mappedFile = cache.get(index);
        if (mappedFile == null) {
            String fileFullPath = getFullPathByIndex(index);
            FileChannel fileChannel = new RandomAccessFile(new File(fileFullPath), "rw").getChannel();
            MappedByteBuffer mappedByteBuffer = fileChannel.map(FileChannel.MapMode.READ_WRITE, 0, mappedSize);
            mappedFile = new MappedPageImpl(fileFullPath, fileChannel, mappedByteBuffer);
            cache.put(index, mappedFile);
            return mappedFile;
        }
        return mappedFile;
    }

    @Override
    public void flush() {
        Collection<MappedPageImpl> values = cache.values();
        for (MappedPageImpl mp : values) {
            mp.flush();
        }
    }

    @Override
    public void close() {
        Collection<MappedPageImpl> values = cache.values();
        for (MappedPageImpl mp : values) {
            mp.clean();
        }
    }

    /**
     * file 完整路径
     *
     * @param index
     * @return
     */
    private String getFullPathByIndex(long index) {
        return this.filePrefix + index + BrokerConstants.PAGE_FILE_SUFFIX;
    }
}
```

### IBigArray 

```java
public interface IBigArray extends Closeable {
	
	public static final long NOT_FOUND = -1;
	
	long append(byte[] data) throws IOException;
	
	byte[] get(long index) throws IOException;
	
	long getTimestamp(long index) throws IOException;

	long size();
	
	int getDataPageSize();
	
	long getHeadIndex();
	
	long getTailIndex();
	
	boolean isEmpty();
	
	boolean isFull();
	
	void flush();
}
```

### BigArrayImpl 
```java
public class BigArrayImpl implements IBigArray {

    // folder name for index page
    public static String INDEX_PAGE_FOLDER = "index";
    // folder name for data page
    public static String DATA_PAGE_FOLDER = "data";
    // folder name for meta data page
    public static String META_DATA_PAGE_FOLDER = "meta_data";

    // 2 ^ 17 = 1024 * 128
    final static int INDEX_ITEMS_PER_PAGE_BITS = 17; // 1024 * 128
    // number of items per page
    final static int INDEX_ITEMS_PER_PAGE = 1 << INDEX_ITEMS_PER_PAGE_BITS;
    // 2 ^ 5 = 32
    final static int INDEX_ITEM_LENGTH_BITS = 5;
    // length in bytes of an index item
    final static int INDEX_ITEM_LENGTH = 1 << INDEX_ITEM_LENGTH_BITS;
    // size in bytes of an index page
    final static int INDEX_PAGE_SIZE = INDEX_ITEM_LENGTH * INDEX_ITEMS_PER_PAGE;

    // size in bytes of a data page
    final int DATA_PAGE_SIZE;

    // default size in bytes of a data page
    final static int DEFAULT_DATA_PAGE_SIZE = 128 * 1024 * 1024;
    // minimum size in bytes of a data page
    final static int MINIMUM_DATA_PAGE_SIZE = 32 * 1024 * 1024;
    // 2 ^ 4 = 16
    final static int META_DATA_ITEM_LENGTH_BITS = 4;
    // size in bytes of a meta data page
    final static int META_DATA_PAGE_SIZE = 1 << META_DATA_ITEM_LENGTH_BITS;

    // only use the first page
    static final long META_DATA_PAGE_INDEX = 0;

    final AtomicLong arrayHeadIndex = new AtomicLong();
    // tail index of the big array,
    // readers can't read items before this tail
    final AtomicLong arrayTailIndex = new AtomicLong();

    // head index of the data page, this is the to be appended data page index
    long headDataPageIndex;
    // head offset of the data page, this is the to be appended data offset
    int headDataItemOffset;

    // timestamp offset of an data item within an index item
    final static int INDEX_ITEM_DATA_ITEM_TIMESTAMP_OFFSET = 16;


    private String storageDirectoryPath;
    // factory for index page management(acquire, release, cache)
    IMappedPageFactory indexPageFactory;
    // factory for data page management(acquire, release, cache)
    IMappedPageFactory dataPageFactory;
    // factory for meta data page management(acquire, release, cache)
    IMappedPageFactory metaPageFactory;

    public BigArrayImpl(String storageDirectoryPath, String arrayName) throws IOException {
        this(storageDirectoryPath, arrayName, DEFAULT_DATA_PAGE_SIZE);
    }

    /**
     * 支持自定义 mapped size
     */
    public BigArrayImpl(String arrayDir, String arrayName, int mappedSize) throws IOException {
        this.storageDirectoryPath = arrayDir;
        if (!storageDirectoryPath.endsWith(File.separator)) {
            storageDirectoryPath += File.separator;
        }
        // append array name as part of the directory
        this.storageDirectoryPath = storageDirectoryPath + arrayName + File.separator;

        // validate directory
//        if (!FileUtil.isFilenameValid(arrayDirectory)) {
//            throw new IllegalArgumentException("invalid array directory : " + arrayDirectory);
//        }

        if (mappedSize < MINIMUM_DATA_PAGE_SIZE) {
            throw new IllegalArgumentException("invalid mapped size, allowed minimum is : " + MINIMUM_DATA_PAGE_SIZE + " bytes.");
        }

        DATA_PAGE_SIZE = mappedSize;

        this.commonInit();
    }

    void commonInit() throws IOException {
        // initialize page factories
        this.indexPageFactory = new MappedPageFactoryImpl(
                this.storageDirectoryPath + INDEX_PAGE_FOLDER, INDEX_PAGE_SIZE);
        this.dataPageFactory = new MappedPageFactoryImpl(
                this.storageDirectoryPath + DATA_PAGE_FOLDER, DATA_PAGE_SIZE);
        // the ttl does not matter here since meta data page is always cached
        this.metaPageFactory = new MappedPageFactoryImpl(
                this.storageDirectoryPath + META_DATA_PAGE_FOLDER, META_DATA_PAGE_SIZE);

        // initialize array indexes
        initArrayIndex();

        // initialize data page indexes
        initDataPageIndex();
    }

    // find out array head/tail from the meta data
    void initArrayIndex() throws IOException {
        IMappedPage metaDataFile = this.metaPageFactory.acquireMappedPage(META_DATA_PAGE_INDEX);
        ByteBuffer metaBuf = metaDataFile.setPosReturnSliceBuffer(0);
        long head = metaBuf.getLong(); // 头
        long tail = metaBuf.getLong(); // 尾

        arrayHeadIndex.set(head);
        arrayTailIndex.set(tail);
    }

    // find out data page head index and offset
    void initDataPageIndex() throws IOException {

        if (this.isEmpty()) {
            headDataPageIndex = 0L;
            headDataItemOffset = 0;
        } else {
            IMappedPage previousIndexPage = null;
            long previousIndexPageIndex = -1;
            try {
                long previousIndex = this.arrayHeadIndex.get() - 1;
                previousIndexPageIndex = Calculator.div(this.arrayHeadIndex.get(), INDEX_ITEMS_PER_PAGE_BITS); // shift optimization
                previousIndexPage = this.indexPageFactory.acquireMappedPage(previousIndexPageIndex);
                int previousIndexPageOffset = (int) (Calculator.mul(Calculator.mod(previousIndex, INDEX_ITEMS_PER_PAGE_BITS), INDEX_ITEM_LENGTH_BITS));
                ByteBuffer previousIndexItemBuffer = previousIndexPage.setPosReturnSliceBuffer(previousIndexPageOffset);
                long previousDataPageIndex = previousIndexItemBuffer.getLong(); // 数据 index file
                int previousDataItemOffset = previousIndexItemBuffer.getInt();  // 数据 offset
                int perviousDataItemLength = previousIndexItemBuffer.getInt();  // 数据 length

                headDataPageIndex = previousDataPageIndex;
                headDataItemOffset = previousDataItemOffset + perviousDataItemLength; // 下一次数据写入 Offset
            } finally {
                if (previousIndexPage != null) {
                    //this.indexPageFactory.releasePage(previousIndexPageIndex);
                }
            }
        }
    }

    @Override
    public long append(byte[] data) throws IOException {
        long toAppendIndexPageIndex = -1L;
        long toAppendDataPageIndex = -1L;

        // 获取 data file 写入数据的起始 offset
        // prepare the data pointer
        if (this.headDataItemOffset + data.length > DATA_PAGE_SIZE) { // not enough space
            this.headDataPageIndex++;
            this.headDataItemOffset = 0;
        }

        toAppendDataPageIndex = this.headDataPageIndex;
        int toAppendDataItemOffset  = this.headDataItemOffset;

        long toAppendArrayIndex = this.arrayHeadIndex.get();

        // data
        IMappedPage toAppendDataPage = this.dataPageFactory.acquireMappedPage(toAppendDataPageIndex);
        ByteBuffer toAppendDataPageBuffer = toAppendDataPage.setPosReturnSelfBuffer(toAppendDataItemOffset);
        toAppendDataPageBuffer.put(data);
        // update to next
        this.headDataItemOffset += data.length;

        int toAppendIndexItemOffset = convertIndexFileOffset(toAppendArrayIndex);
        // index
        IMappedPage toAppendIndexPage = this.indexPageFactory.acquireMappedPage(toAppendIndexPageIndex);
        ByteBuffer toAppendIndexPageBuffer = toAppendIndexPage.setPosReturnSelfBuffer(toAppendIndexItemOffset);
        toAppendIndexPageBuffer.putLong(toAppendDataPageIndex);
        toAppendIndexPageBuffer.putInt(toAppendDataItemOffset);
        toAppendIndexPageBuffer.putInt(data.length);
        long currentTime = System.currentTimeMillis();
        toAppendIndexPageBuffer.putLong(currentTime);

        // 增加数据 size
        arrayHeadIndex.incrementAndGet();

        // meta_data
        IMappedPage metaDataPage = this.metaPageFactory.acquireMappedPage(META_DATA_PAGE_INDEX);
        ByteBuffer metaDataByteBuffer = metaDataPage.setPosReturnSliceBuffer(0);
        metaDataByteBuffer.putLong(arrayHeadIndex.get());
        metaDataByteBuffer.putLong(arrayTailIndex.get());

        return toAppendArrayIndex;
    }

    @Override
    public byte[] get(long index) throws IOException {
        ByteBuffer sliceIndexByteBuffer = calcArrayIndexReturnSliceIndexByteBuffer(index);
        long dataPageIndex = sliceIndexByteBuffer.getLong();
        int dataItemOffset = sliceIndexByteBuffer.getInt();
        int dataLength = sliceIndexByteBuffer.getInt();
        IMappedPage dataMappedPage = this.dataPageFactory.acquireMappedPage(dataPageIndex);
        byte[] content = dataMappedPage.readContent(dataItemOffset, dataLength);
        return content;
    }

    /**
     * 根据数据下标，获取 item 追加时间
     *
     * @param index valid data index
     * @return
     * @throws IOException
     */
    @Override
    public long getTimestamp(long index) throws IOException {
        ByteBuffer sliceIndexByteBuffer = calcArrayIndexReturnSliceIndexByteBuffer(index);
        int position = sliceIndexByteBuffer.position();
        sliceIndexByteBuffer.position(position + INDEX_ITEM_DATA_ITEM_TIMESTAMP_OFFSET);
        long timestamp = sliceIndexByteBuffer.getLong();
        return timestamp;
    }

    @Override
    public long size() {
        return this.arrayHeadIndex.get() - this.arrayTailIndex.get();
    }

    /**
     * 获取 Data File 设置的 mapped size
     *
     * @return
     */
    @Override
    public int getDataPageSize() {
        return DATA_PAGE_SIZE;
    }

    @Override
    public long getHeadIndex() {
        return arrayHeadIndex.get();
    }

    @Override
    public long getTailIndex() {
        return arrayTailIndex.get();
    }

    @Override
    public boolean isEmpty() {
        return this.arrayHeadIndex.get() == this.arrayTailIndex.get();
    }

    @Override
    public boolean isFull() {
        return false;
    }

    @Override
    public void flush() {
        dataPageFactory.flush();
        indexPageFactory.flush();
        metaPageFactory.flush();
    }

    @Override
    public void close() throws IOException {
        this.dataPageFactory.close();
        this.indexPageFactory.close();
        this.metaPageFactory.close();
    }

    /**
     * 根据 arrayIndex 获取 对应的 ByteBuffer
     * 该方法会根据 index 计算出 indexFileOffset，并且根据 indexFileOffset 设置 pos
     */
    private ByteBuffer calcArrayIndexReturnSliceIndexByteBuffer(long arrayIndex) throws IOException {
        long indexPageIndex = 0L;
        IMappedPage indexMappedPage = this.indexPageFactory.acquireMappedPage(indexPageIndex);
        int indexFileOffset = convertIndexFileOffset(arrayIndex);
        ByteBuffer mappedBuffer = indexMappedPage.setPosReturnSliceBuffer(indexFileOffset);
        return mappedBuffer;
    }

    /**
     * 根据 arrayIndex 计算 index file 写入数据的起始 offset
     *
     * @param arrayIndex
     * @return
     */
    private int convertIndexFileOffset(long arrayIndex) {
        int toAppendIndexItemOffset = (int) (Calculator.mul(Calculator.mod(arrayIndex, INDEX_ITEMS_PER_PAGE_BITS), INDEX_ITEM_LENGTH_BITS));
        return toAppendIndexItemOffset;
    }
}
```

## 总结     
注意，目前它还是 Demo ，不够完善，后续陆续补齐实践细节。    

>代码目录： `javamain-services\javamain-db\src\main\java\com\javamain\db\bigqueue01`，github: `https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-db/src/main/java/com/javamain/db/bigqueue01`          

refer   
1.https://bulldog2011.github.io/blog/categories/big-queue/      
2.https://github.com/bulldog2011/bigqueue 