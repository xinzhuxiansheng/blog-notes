--In Blog
--Tags: Kafka

# Kafka Broker关于处理ProduceRequest的MessageConversions(非常重要)

> 涉及Kafka是2.2.1版本，Producer(kafka-clients)是0.10.0.0版本并且配置压缩格式为**snappy**

## 1. 前言
在之前的推文["Kafka Broker处理ProduceRequest的过程"](https://mp.weixin.qq.com/s/4siSxGScg1wqI6H7NKLVCw) 中第三章节 **Magic Value**中介绍了Kafka的消息格式的版本发布历程，这篇推文介绍不同版本的差异化。     

>为了说明章节4 前面1至3铺垫过长，若不表达清楚，但又缺少上下文， 不过这篇推文非常建议看下，对于Kafka集群优化来说是一个方向。

**这里提出一个的issue：高版本Kafka Broker如何兼容低版本的produceRequest?** 

## 2. 转换
* 在业务代码中，有时会遇到ClassA的对象使用Fastjson转换成JSONObject temp对象，再将temp转成ClassB(参考图2-1)， 若ClassA的对象字节**较大**，这对于Fastjson的转换来说是耗时的。     
`图2-1`
![fastjson转换](http://img.xinzhuxiansheng.com/blogimgs/kafka/broker/broker_oldmessageformat04.png)

* 这里可以引出Kafka针对ProduceRequest的消息转换(参考图2-2)，对于Producer来说消息的发送   很多情况下都是批量+压缩，所以若V2以下的消息集**较大**进行解压遍历再压缩转换V2格式的消息集，这同样也是耗时。    
`图2-2`
![messageConversions](http://img.xinzhuxiansheng.com/blogimgs/kafka/broker/broker_oldmessageformat05.png)

## 3. LogValidator.validateMessagesAndAssignOffsets(...)
请阅读之前的推文["Kafka Broker处理ProduceRequest的过程"](https://mp.weixin.qq.com/s/4siSxGScg1wqI6H7NKLVCw) ，快速了解它的上下文。 图3-1给出了MessageConversions处理逻辑的地方 **Log.append()方法**，它涉及主要的3个步骤如图3-2，其中 LogValidator.validateMessagesAndAssignOffsets(...)方法，它会涉及到消息的解压和转换。  下面会详细说明validateMessagesAndAssignOffsets()方法的处理过程。    
`图3-1`
![起始点](http://img.xinzhuxiansheng.com/blogimgs/kafka/broker/broker_oldmessageformat01.png)

`图3-2`
![validateMessagesAndAssignOffsets](http://img.xinzhuxiansheng.com/blogimgs/kafka/broker/broker_oldmessageformat02.png)

### 3.1 LogValidator.validateMessagesAndAssignOffsets(...)

### *3.1.1 入口*
```java
  private[kafka] def validateMessagesAndAssignOffsets(records: MemoryRecords,
                                                      offsetCounter: LongRef,
                                                      time: Time,
                                                      now: Long,
                                                      sourceCodec: CompressionCodec,
                                                      targetCodec: CompressionCodec,
                                                      compactedTopic: Boolean,
                                                      magic: Byte,
                                                      timestampType: TimestampType,
                                                      timestampDiffMaxMs: Long,
                                                      partitionLeaderEpoch: Int,
                                                      isFromClient: Boolean,
                                                      interBrokerProtocolVersion: ApiVersion): ValidationAndOffsetAssignResult = {
    if (sourceCodec == NoCompressionCodec && targetCodec == NoCompressionCodec) {
     // ...省略部分代码， 假定Producer的压缩格式是snappy
    } else {
      validateMessagesAndAssignOffsetsCompressed(records, offsetCounter, time, now, sourceCodec, targetCodec, compactedTopic,
        magic, timestampType, timestampDiffMaxMs, partitionLeaderEpoch, isFromClient, interBrokerProtocolVersion)
    }
  }
```

### *3.2 LogValidator.validateMessagesAndAssignOffsetsCompressed(...)*
validateMessagesAndAssignOffsetsCompressed()方法,它的功能正如它的名称一样：validate Message And assign Offset。 由于消息是批量发送，请参考**代码3-2-1**、**图3-2-2** 它申明了一个可变数组来存放校验过的消息，用两个for循环遍历消息，最终将record放入可变数组中。  
`代码3-2-1 LogValidator.validateMessagesAndAssignOffsetsCompressed()`
```java
val validatedRecords = new mutable.ArrayBuffer[Record]

for (batch <- records.batches.asScala) {  // AbstractLegacyRecordBatch$BasicLegacyRecordBatch
    // 省略部分代码

    for (record <- batch.asScala) {  // AbstractLegacyRecordBatch$BasicLegacyRecordBatch
      // 省略部分代码

      // No in place assignment situation 4
      if (!record.hasMagic(toMagic))  //AbstractLegacyRecordBatch$BasicLegacyRecordBatch
        inPlaceAssignment = false

      validatedRecords += record
    }
  }
```

`图3-2-2 for循环` 
![for循环](http://img.xinzhuxiansheng.com/blogimgs/kafka/broker/broker_oldmessageformat03.png)

**3.2.1**   
外层for循环 *records.batches* 将MemoryRecords的ByteBuffer类型的buffer，根据Records.java中(**参考代码3-2-3**) buffer当前的position + SIZE的偏移量(OFFSET_OFFSET+OFFSET_LENGTH)，通过buffer.getInt()固定读取4个长度(SIZE_LENGTH)，。并根据Magic Value判断若大于V1创建DefaultRecrdBatch否则ByteBufferLegacyRecordBatch对象，直到将ByteBuffer读取完(**参考代码3-2-4**)。  

`代码3-2-3 Records`
```java
public interface Records extends BaseRecords {
    int OFFSET_OFFSET = 0;
    int OFFSET_LENGTH = 8;
    int SIZE_OFFSET = OFFSET_OFFSET + OFFSET_LENGTH;
    int SIZE_LENGTH = 4;
    int LOG_OVERHEAD = SIZE_OFFSET + SIZE_LENGTH;

    // the magic offset is at the same offset for all current message formats, but the 4 bytes
    // between the size and the magic is dependent on the version.
    int MAGIC_OFFSET = 16;
    int MAGIC_LENGTH = 1;
    int HEADER_SIZE_UP_TO_MAGIC = MAGIC_OFFSET + MAGIC_LENGTH;
```

`代码3-2-4 ByteBufferLogInputStream.nextBatch()`
```java
public MutableRecordBatch nextBatch() {
    int remaining = buffer.remaining();

    Integer batchSize = nextBatchSize();
    if (batchSize == null || remaining < batchSize)
        return null;

    byte magic = buffer.get(buffer.position() + MAGIC_OFFSET);

    ByteBuffer batchSlice = buffer.slice();
    batchSlice.limit(batchSize);
    buffer.position(buffer.position() + batchSize);

    if (magic > RecordBatch.MAGIC_VALUE_V1)
        return new DefaultRecordBatch(batchSlice);
    else {
        return new AbstractLegacyRecordBatch.ByteBufferLegacyRecordBatch(batchSlice);
    }
}
```

>在本文的第一行就已经特别说明Producer的version是0.10.0.0 且Magic Value=1

**3.2.2**   
根据章节3.2.1 外层for循环会得到**ByteBufferLegacyRecordBatch类型的 batch**, 在内层for循环 *batch.asScala*，batch继承了AbstractLegacyRecordBatch，也重写了iterator<Record>的iterator()方法。 在iterator()方法中，在配置压缩情况下会创建 **DeepRecordsIterator类型**的对象，在它的构造方法(参考代码3-2-2-1)中，会将batch**解压**转成Stream(DataLogInputStream)，同样像外循环一样(参考代码3-2-2-2), 首先读取batchSize，再将读取固定的长度数据放入ByteBuffer,不过这里不同的地方是通过字节流来读取**Utils.readFully(stream, batchBuffer)**。并在最后将Offset和batchBuffer封装成BasicLegacyRecordBatch。      

DeepRecordsIterator的absoluteBaseOffset字段，需要重点说明，在V2消息batch中(参考DefaultRecordBatch.java)，包含baseOffset()获取相对开始的Offset，lastOffsetDelta() 获取最后Offset的增值，所以才会后面针对V1格式的消息batch的absoluteBaseOffset的计算以及在**DeepRecordsIterator.makeNext()** 赋值相对Offset。     



`代码3-2-2-1 DeepRecordsIterator`
```java
private static class DeepRecordsIterator extends AbstractIterator<Record> implements CloseableIterator<Record> {
    private final ArrayDeque<AbstractLegacyRecordBatch> innerEntries;
    private final long absoluteBaseOffset;   // 重点
    private final byte wrapperMagic;

    private DeepRecordsIterator(AbstractLegacyRecordBatch wrapperEntry,
                                boolean ensureMatchingMagic,
                                int maxMessageSize,
                                BufferSupplier bufferSupplier) {
        LegacyRecord wrapperRecord = wrapperEntry.outerRecord();
       
        // 省略部分代码

        // 解压并转成stream
        InputStream stream = compressionType.wrapForInput(wrapperValue, wrapperRecord.magic(), bufferSupplier);
        LogInputStream<AbstractLegacyRecordBatch> logStream = new DataLogInputStream(stream, maxMessageSize);

        long lastOffsetFromWrapper = wrapperEntry.lastOffset();
        long timestampFromWrapper = wrapperRecord.timestamp();
        this.innerEntries = new ArrayDeque<>();

        // If relative offset is used, we need to decompress the entire message first to compute
        // the absolute offset. For simplicity and because it's a format that is on its way out, we
        // do the same for message format version 0
        try {
            while (true) {
                AbstractLegacyRecordBatch innerEntry = logStream.nextBatch();  // BasicLegacyRecordBatch
                if (innerEntry == null)
                    break;

                LegacyRecord record = innerEntry.outerRecord();
                byte magic = record.magic();

                if (ensureMatchingMagic && magic != wrapperMagic)
                    throw new InvalidRecordException("Compressed message magic " + magic +
                            " does not match wrapper magic " + wrapperMagic);

                // Magic Value = 1 ，设置了Timestamp字段值
                if (magic == RecordBatch.MAGIC_VALUE_V1) {
                    LegacyRecord recordWithTimestamp = new LegacyRecord(
                            record.buffer(),
                            timestampFromWrapper,
                            wrapperRecord.timestampType());
                    innerEntry = new BasicLegacyRecordBatch(innerEntry.lastOffset(), recordWithTimestamp);
                }

                innerEntries.addLast(innerEntry);
            }

            if (innerEntries.isEmpty())
                throw new InvalidRecordException("Found invalid compressed record set with no inner records");

            if (wrapperMagic == RecordBatch.MAGIC_VALUE_V1) {
                if (lastOffsetFromWrapper == 0) {
                    // The outer offset may be 0 if this is produce data from certain versions of librdkafka.
                    this.absoluteBaseOffset = 0;
                } else {
                    long lastInnerOffset = innerEntries.getLast().offset();
                    if (lastOffsetFromWrapper < lastInnerOffset)
                        throw new InvalidRecordException("Found invalid wrapper offset in compressed v1 message set, " +
                                "wrapper offset '" + lastOffsetFromWrapper + "' is less than the last inner message " +
                                "offset '" + lastInnerOffset + "' and it is not zero.");
                    this.absoluteBaseOffset = lastOffsetFromWrapper - lastInnerOffset;
                }
            } else {
                this.absoluteBaseOffset = -1;
            }
        } catch (IOException e) {
            throw new KafkaException(e);
        } finally {
            Utils.closeQuietly(stream, "records iterator stream");
        }
    }

    @Override
    protected Record makeNext() {
        if (innerEntries.isEmpty())
            return allDone();

        AbstractLegacyRecordBatch entry = innerEntries.remove();

        // Convert offset to absolute offset if needed.
        if (wrapperMagic == RecordBatch.MAGIC_VALUE_V1) {
            long absoluteOffset = absoluteBaseOffset + entry.offset();
            entry = new BasicLegacyRecordBatch(absoluteOffset, entry.outerRecord());
        }

        if (entry.isCompressed())
            throw new InvalidRecordException("Inner messages must not be compressed");

        return entry;
    }

    @Override
    public void close() {}
}
```

`代码3-2-2-2 AbstractLegacyRecordBatch.nextBatch()`
```java
public AbstractLegacyRecordBatch nextBatch() throws IOException {
        offsetAndSizeBuffer.clear();
        Utils.readFully(stream, offsetAndSizeBuffer);
        if (offsetAndSizeBuffer.hasRemaining())
            return null;

        long offset = offsetAndSizeBuffer.getLong(Records.OFFSET_OFFSET);
        int size = offsetAndSizeBuffer.getInt(Records.SIZE_OFFSET);
        System.out.println("0625 yzhou offset: "+ offset + " , size: " + size);
        if (size < LegacyRecord.RECORD_OVERHEAD_V0)
            throw new CorruptRecordException(String.format("Record size is less than the minimum record overhead (%d)", LegacyRecord.RECORD_OVERHEAD_V0));
        if (size > maxMessageSize)
            throw new CorruptRecordException(String.format("Record size exceeds the largest allowable message size (%d).", maxMessageSize));

        ByteBuffer batchBuffer = ByteBuffer.allocate(size);
        Utils.readFully(stream, batchBuffer);
        if (batchBuffer.hasRemaining())
            return null;
        batchBuffer.flip();

        return new BasicLegacyRecordBatch(offset, new LegacyRecord(batchBuffer));
    }
```

## 4. 重点！！！ LogValidator.buildRecordsAndAssignOffsets() 
在章节3中花了较多的篇幅来说明嵌套for循环，它并不是那么简单。 下面该到我们的重点：buildRecordsAndAssignOffsets()方法。 在代码4-1中，我添加**@标记4.1**，我从这里开始说起。 record的Magic Vlue = 1，toMiagic表示的是Broker的Magic Value，所以inPlaceAssignment始终等于false。 当执行完嵌套循环，会调用 buildRecordsAndAssignOffsets()方法。  

在代码4-2中，若熟悉Producer的send()代码的话 ，这里会非常的熟悉。    
1. 申请堆内存缓冲区     
2. 按照Maigc Value = 2创建MemoryRecordsBuilder类型 builder对象
3. 在遍历消息集合，将每条消息重新append到 builder中 
4. 最后调用build()方法，构建MemoryRecords类型的消息批次     

在**代码4-2**中@标记4.2会涉及到三个非常重要的指标，指标项： TemporaryMemoryBytes，MessageConversionsTimeMs，MessageConversionsTimeMs，
请参考Kafka官网[6.6 Monitoring](http://kafka.apache.org/22/documentation.html#monitoring)    

| DESCRIPTION      |    MBEAN NAME | NORMAL VALUE  |
| :-------- | --------:| :--: |
| Temporary memory size in bytes  | kafka.network:type=RequestMetrics,name=TemporaryMemoryBytes,request={Produce\Fetch} |  Temporary memory used for message format conversions and decompression.  |
| Message conversion time     |   	kafka.network:type=RequestMetrics,name=MessageConversionsTimeMs,request={Produce\Fetch} |  Time in milliseconds spent on message format conversions.  |
| Message conversion rate      | kafka.server:type=BrokerTopicMetrics,name={Produce|Fetch}MessageConversionsPerSec,topic=([-.\w]+) | Number of records which required message format conversion.  |


`代码4-1 LogValidator.validateMessagesAndAssignOffsetsCompressed()`
```java
def validateMessagesAndAssignOffsetsCompressed(records: MemoryRecords,
                                            offsetCounter: LongRef,
                                            time: Time,
                                            now: Long,
                                            sourceCodec: CompressionCodec,
                                            targetCodec: CompressionCodec,
                                            compactedTopic: Boolean,
                                            toMagic: Byte,
                                            timestampType: TimestampType,
                                            timestampDiffMaxMs: Long,
                                            partitionLeaderEpoch: Int,
                                            isFromClient: Boolean,
                                            interBrokerProtocolVersion: ApiVersion): ValidationAndOffsetAssignResult = {
    // 省略部分代码                                            
for (batch <- records.batches.asScala) {  // AbstractLegacyRecordBatch$BasicLegacyRecordBatch
    // 省略部分代码

    for (record <- batch.asScala) {  // AbstractLegacyRecordBatch$BasicLegacyRecordBatch
    
    // 省略部分代码

    // No in place assignment situation 4
    //@标记4.1
    if (!record.hasMagic(toMagic))  //AbstractLegacyRecordBatch$BasicLegacyRecordBatch
        inPlaceAssignment = false

    validatedRecords += record
    }
}

if (!inPlaceAssignment) {
    val (producerId, producerEpoch, sequence, isTransactional) = {
    // note that we only reassign offsets for requests coming straight from a producer. For records with magic V2,
    // there should be exactly one RecordBatch per request, so the following is all we need to do. For Records
    // with older magic versions, there will never be a producer id, etc.
    val first = records.batches.asScala.head
    (first.producerId, first.producerEpoch, first.baseSequence, first.isTransactional) // ByteBufferLegacyRecordBatch
    }
    buildRecordsAndAssignOffsets(toMagic, offsetCounter, time, timestampType, CompressionType.forId(targetCodec.codec), now,
    validatedRecords, producerId, producerEpoch, sequence, isTransactional, partitionLeaderEpoch, isFromClient,
    uncompressedSizeInBytes)
} else {
    // 省略部分代码
    }
}
```

`代码4-2 LogValidator.buildRecordsAndAssignOffsets()`
```java
private def buildRecordsAndAssignOffsets(magic: Byte,
                                           offsetCounter: LongRef,
                                           time: Time,
                                           timestampType: TimestampType,
                                           compressionType: CompressionType,
                                           logAppendTime: Long,
                                           validatedRecords: Seq[Record],
                                           producerId: Long,
                                           producerEpoch: Short,
                                           baseSequence: Int,
                                           isTransactional: Boolean,
                                           partitionLeaderEpoch: Int,
                                           isFromClient: Boolean,
                                           uncompresssedSizeInBytes: Int): ValidationAndOffsetAssignResult = {
    val startNanos = time.nanoseconds
    val estimatedSize = AbstractRecords.estimateSizeInBytes(magic, offsetCounter.value, compressionType,
      validatedRecords.asJava)
    val buffer = ByteBuffer.allocate(estimatedSize)
    val builder = MemoryRecords.builder(buffer, magic, compressionType, timestampType, offsetCounter.value,
      logAppendTime, producerId, producerEpoch, baseSequence, isTransactional, partitionLeaderEpoch)

    validatedRecords.foreach { record =>
      builder.appendWithOffset(offsetCounter.getAndIncrement(), record)
    }

    val records = builder.build()

    val info = builder.info

    // This is not strictly correct, it represents the number of records where in-place assignment is not possible
    // instead of the number of records that were converted. It will over-count cases where the source and target are
    // message format V0 or if the inner offsets are not consecutive. This is OK since the impact is the same: we have
    // to rebuild the records (including recompression if enabled).
    val conversionCount = builder.numRecords

    //@标记4.2
    val recordConversionStats = new RecordConversionStats(uncompresssedSizeInBytes + builder.uncompressedBytesWritten,
      conversionCount, time.nanoseconds - startNanos)

    ValidationAndOffsetAssignResult(
      validatedRecords = records,
      maxTimestamp = info.maxTimestamp,
      shallowOffsetOfMaxTimestamp = info.shallowOffsetOfMaxTimestamp,
      messageSizeMaybeChanged = true,
      recordConversionStats = recordConversionStats)
  }
```


## 5. 总结
对于低版本ProduceRequest请求，高版本Broker会经历解压遍历且重新构建V2的格式的消息batch ，这种情况对于大数据量来说，性能的损耗较大，所以非常建议将kafka-clients也升级至V2格式，也就是0.11版本以上。 对于Broker监控来说，请随时关注`章节4`提到的三个非常重要指标。 每次的较长的处理速度都有可能会拖累Broker的请求队列。