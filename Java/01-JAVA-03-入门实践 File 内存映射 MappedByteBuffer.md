# JAVA - 了解内存映射 Memory-mapped 和 MappedByteBuffer (它值得你去花时间)   

>JDK version: 1.8    

## 引言           
**它不是一个迷!**,它就像 `Netty ByteBuf`,`Java ByteBuffer` 一样，一直藏在应用服务的底层，大多数应用开发过程中几乎很少会直接调用它们相关的 API，更多的是上层框架来处理。在博主热爱的`MQ`领域中，它支撑了半边天。                

### Kafka 源码中的 MappedByteBuffer         
![mmpgetstarted01](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted01.png)        

在 Kafka 源码`AbstractIndex`抽象类中使用`MappedByteBuffer`，通过下面类图中，估计没看过 Kafka Store 设计的大伙，也差不多可以猜出 Kafka Topic 的 `.index` , `timeindex`的文件读写是采用`内存映射机制`进行读写。                           

![mmpgetstarted02](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted02.png)               

**Topic 数据存储的目录结构:**           
```bash
[root@vm01 yzhoutpjson01-0]# ll
total 20
-rw-r--r--. 1 root root 10485760 Dec 21 10:03 00000000000000000000.index
-rw-r--r--. 1 root root      243 Dec 21 10:25 00000000000000000000.log
-rw-r--r--. 1 root root 10485756 Dec 21 10:03 00000000000000000000.timeindex
-rw-r--r--. 1 root root       10 Dec 21 10:03 00000000000000000001.snapshot
-rw-r--r--. 1 root root        8 Dec 21 10:03 leader-epoch-checkpoint
-rw-r--r--. 1 root root       43 Dec 20 19:03 partition.metadata
```

### RocketMQ 源码中的 MappedByteBuffer     
![mmpgetstarted03](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted03.png)      

RocketMQ 的存储读写也是基于内存映射(MappedByteBuffer)的，消息存储时首先将消息追加到内存中，在根据配置的刷盘策略在不同时间刷盘。             

**Topic 数据存储的目录结构:**       
```bash
[root@vm01 store]# tree .  
.
├── checkpoint
├── commitlog
│   └── 00000000000000000000
├── config
│   ├── consumerFilter.json
│   ├── consumerFilter.json.bak
│   ├── consumerOffset.json
│   ├── consumerOffset.json.bak
│   ├── delayOffset.json
│   ├── delayOffset.json.bak
│   ├── subscriptionGroup.json
│   ├── topics.json
│   └── topics.json.bak
├── consumequeue
│   ├── mytopic
│   │   ├── 0
│   │   │   └── 00000000000000000000
│   │   ├── 1
            ... 省略部分
│   │   └── 7
│   │       └── 00000000000000000000
│   ├── %RETRY%MyConsumerGroup
│   │   └── 0
│   │       └── 00000000000000000000
│   └── SCHEDULE_TOPIC_XXXX
│       ├── 2
│       │   └── 00000000000000000000
            ... 省略部分
│       └── 5
│           └── 00000000000000000000
├── index
│   └── 20250102214748511
└── lock
```

>其实，在编程领域中，`磁盘 IO`、`网络 IO` 通常涉及到它们各自的`Buffer`操作，关于这部分看似一学就会，一上手就懵呀。 :)     

## 内存映射         
  
### Java Memory-mapped IO        
If you know how java IO works at lower level, then you will be aware of buffer handling, memory paging and other such concepts. For conventional file I/O, in which user processes issue read() and write() system calls to transfer data, there is almost always one or more copy operations to move the data between these filesystem pages in kernel space and a memory area in user space. This is because there is not usually a one-to-one alignment between filesystem pages and user buffers.               

There is, however, a special type of I/O operation supported by most operating systems that allows user processes to take maximum advantage of the page-oriented nature of system I/O and completely avoid buffer copies. This is called memory-mapped I/O and we are going to learn few things here around memory-mapped files.    

### Java Memory-Mapped Files              
Memory-mapped I/O uses the filesystem to establish a virtual memory mapping from user space directly to the applicable filesystem pages. With a memory-mapped file, we can pretend that the entire file is in memory and that we can access it by simply treating it as a very large array. This approach greatly simplifies the code we write in order to modify the file.         

内存映射是指操作系统将内存中的某⼀块区域与磁盘中的⽂件关联起来，当要求访问内存中的
⼀段数据时，转换为访问⽂件的某⼀段数据，这种⽅式的⽬的同样是减少数据从内核空间缓存到⽤⼾空间缓存的数据复制操作，因为这两个空间的数据是共享的。当然机器是否有足够预留的空间也相当重要。           

![mmpgetstarted04](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted04.png)    

>摘自 MappedByteBuffer doc,https://docs.oracle.com/javase/8/docs/api/java/nio/MappedByteBuffer.html  
```bash
public abstract class MappedByteBuffer
extends ByteBuffer

A direct byte buffer whose content is a memory-mapped region of a file.

Mapped byte buffers are created via the FileChannel.map method. This class extends the ByteBuffer class with operations that are specific to memory-mapped file regions.

A mapped byte buffer and the file mapping that it represents remain valid until the buffer itself is garbage-collected.

The content of a mapped byte buffer can change at any time, for example if the content of the corresponding region of the mapped file is changed by this program or another. Whether or not such changes occur, and when they occur, is operating-system dependent and therefore unspecified.

All or part of a mapped byte buffer may become inaccessible at any time, for example if the mapped file is truncated. An attempt to access an inaccessible region of a mapped byte buffer will not change the buffer's content and will cause an unspecified exception to be thrown either at the time of the access or at some later time. It is therefore strongly recommended that appropriate precautions be taken to avoid the manipulation of a mapped file by this program, or by a concurrently running program, except to read or write the file's content.

Mapped byte buffers otherwise behave no differently than ordinary direct byte buffers. 
```

### 小结        
内存映射是使用 java.nio package 下的`MappedByteBuffer`实现的，并且它占用的是`direct byte buffer`,也就是我们经常说的堆外内存。  

到这里，你一定要意识到 `MappedByteBuffer` 是`零拷贝`的一种实现。            

`MappedByteBuffer` 的使用要小心, 虽然它提高了 I/O 效率，但也需要注意内存管理。因为操作系统会根据需要将文件的部分内容映射到内存，内存页可能会随着程序访问而被换入或换出，可能带来性能波动。这里被称为`PageCache 污染`。      

>特别注意一点：`direct byte buffer` 并不是永无止境，我们仍然要考虑回收内存空间，就像`Netty ByteBuf`。  

## 实践 java.nio MappedByteBuffer           
`MappedFile`是`MappedByteBuffer`的操作类,`MappedFileTest`是`MappedFile`的测试类。
**MappedFile**
```java
/**
 * MappedByteBuffer 操作类
 */
public class MappedFile {
    private File file;
    private MappedByteBuffer mappedByteBuffer;
    private FileChannel fileChannel;

    /**
     * 将文件内存映射
     *
     * @param filePath    文件路径
     * @param startOffset 开始映射的 offset
     * @param mappedSize  映射的体积
     * @throws IOException
     */
    public void loadFileInMMap(String filePath, int startOffset, int mappedSize) throws IOException {
        file = new File(filePath);
        if (!file.exists()) {
            throw new FileNotFoundException("filePath is " + filePath + " inValid");
        }
        fileChannel = new RandomAccessFile(file, "rw").getChannel();
        mappedByteBuffer = fileChannel.map(FileChannel.MapMode.READ_WRITE, startOffset, mappedSize);
    }

    /**
     * 支持从文件的指定 offset 开始读取内容
     *
     * @param readOffset
     * @param size
     * @return
     */
    public byte[] readContent(int readOffset, int size) {
        mappedByteBuffer.position(readOffset);
        byte[] content = new byte[size];
        int j = 0;
        for (int i = 0; i < size; i++) {
            byte b = mappedByteBuffer.get(readOffset + i);
            content[j++] = b;
        }
        return content;
    }

    /**
     * 高性能写入磁盘
     */
    public void writeContent(byte[] content) {
        this.writeContent(content, false);
    }


    /**
     * 写入数据到磁盘当中
     */
    public void writeContent(byte[] content, boolean force) {
        mappedByteBuffer.put(content);
        if (force) {
            mappedByteBuffer.force();
        }
    }

    public void clean(){
        if(mappedByteBuffer == null || !mappedByteBuffer.isDirect() || mappedByteBuffer.capacity() ==0 ){
            return;
        }
        invoke(invoke(viewed(mappedByteBuffer),"cleaner"),"clean");
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

>注意，内存映射占用的内存释放方法，是参考RokcetMQ 源码`org.apache.rocketmq.store.MappedFile#clean()`方法处理。而`MappedByteBuffer`类自身的`clear()`方法并不会自动释放内存，它仅是重置偏移量。    

**MappedFileTest**
```java
public class MappedFileTest {
    private static final String filePath = "E:\\Code\\Java\\javamain-services\\javamain-java8\\file\\00000000";

    public static void main(String[] args) throws IOException, InterruptedException {
        // 内存映射文件
        loadFileTest();
    }

    public static void loadFileTest() throws IOException, InterruptedException {
        Scanner input = new Scanner(System.in);
        int size = input.nextInt();
        MappedFile mappedFile = new MappedFile();
        mappedFile.loadFileInMMap(filePath, 0, 1024 * 1024 * size);
        System.out.println("映射了" + size + "m的空间");
        TimeUnit.SECONDS.sleep(5);
        System.out.println("释放内存");
        mappedFile.clean();
        TimeUnit.SECONDS.sleep(10000);
    }

    public static void testWriteAndReadFile() throws IOException {
        MappedFile mappedFile = new MappedFile();
        mappedFile.loadFileInMMap(filePath, 0, 1024 * 1024);
        System.out.println("映射了 m的空间");
        String str = "Hi MappedByteBuffer! :)";
        byte[] content = str.getBytes();
        mappedFile.writeContent(content);
        byte[] readContent = mappedFile.readContent(0, content.length);
        System.out.println(new String(readContent));
    }

}
```

### MappedFile#loadFileTest()       
该方法是使用内存映射方式加载一个文件，休眠 20 秒后，释放内存。          

为了验证这点，可以使用`arthas`组件（下载地址：https://arthas.aliyun.com/）。  

为了更好了解测试流程，请查看下图:           
![mmpgetstarted05](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted05.png)   

**1.** 执行 MappedFile#loadFileTest() 方法，它会一直等待一个输入的状态。            

**2.** 启动 arthas,观察 memory  
```shell
# 启动 arthas  
java -jar arthas-boot.jar   

# 选择 MappedFile#loadFileTest()方法   
* [1]: 26852 com.javamain.java8.mappedbytebuffer.MappedFileTest
  [2]: 29060 org.jetbrains.idea.maven.server.RemoteMavenServer36
  [3]: 20556 org.jetbrains.jps.cmdline.Launcher
1

# 查看 memory ,注意，它无法自动刷新，每次查看都输入一次 
memory  
```

#### 第一次观察结果：  
```bash
mapped used:0K, total:0K, max:-, usage:0.00%
```
![mmpgetstarted06](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted06.png)    

### 第二次观察结果：    
控制台输入 `100`
**idea 控制台** 
```bash
100
映射了100m的空间
```    

**mmaped**
```bash
mapped used:100m, total:100m, max:-, usage:100.00%
```
![mmpgetstarted07](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted07.png)  

### 第三次观察结果   
等待 n 秒，释放内存
**idea 控制台** 
```bash
100
映射了100m的空间
释放内存
```    

**mmaped**
```bash
mapped used:0K, total:0K, max:-, usage:0.00%
```
![mmpgetstarted08](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted08.png)     

>此时，通过终端查看`00000000`,及时释放内存了，但它的`Length`被标记为`104857600 byte = 100M`                 
![mmpgetstarted09](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted09.png)

>到这里，它与 Netty ByteBuf 的统计有所区别：               
![mmpgetstarted10](http://img.xinzhuxiansheng.com/blogimgs/java/mmpgetstarted10.png)     


### MappedFile#testWriteAndReadFile()       
`MappedByteBuffer`的`write()`方法依靠操作系统页缓存刷盘机制，主打高效。若没有处理好分布式架构，这部分的确会`存在数据丢失风险`, `MappedByteBuffer`也提供了强制将内存缓冲区写入文件，它降低了减少数据丢失的风险。所以，我们用它开发写入服务的时候，都会`提供可配置参数来修改它的写入策略`。          

**idea 控制台**     
```bash
映射了100m的空间
Hi MappedByteBuffer! :)    
```

## 总结   
到这里，我们算是揭开了`MappedByteBuffer`面纱，以及它涉及的关键字 `零拷贝`，`Page Cache 污染`。                   

refer   
1.https://docs.oracle.com/javase/8/docs/api/java/nio/MappedByteBuffer.html      
2.https://arthas.aliyun.com/            