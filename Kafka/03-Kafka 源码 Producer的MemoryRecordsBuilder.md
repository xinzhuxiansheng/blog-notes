--In Blog
--Tags: Kafka

# Kafka Producer的MemoryRecordsBuilder 别忽略它
> 涉及Kafka是2.2.1版本

## 不可忽略它的理由
**1.** RecordAccumulator(消息累加器)在创建ProducerBatch对象需要它       
```java
//RecordAccumulator.append()
MemoryRecordsBuilder recordsBuilder = recordsBuilder(buffer, maxUsableMagic);
ProducerBatch batch = new ProducerBatch(tp, recordsBuilder, time.milliseconds());
FutureRecordMetadata future = Utils.notNull(batch.tryAppend(timestamp, key, value, headers, callback, time.milliseconds()));
```

**2.** Producer发送数据时构建的ProducerRequest对象需要它
```java
//Sender.sendProduceRequest()
for (ProducerBatch batch : batches) {
    TopicPartition tp = batch.topicPartition;
    // 重点! 重点！ 重点！
    MemoryRecords records = batch.records();
    // ...省略部分
    if (!records.hasMatchingMagic(minUsedMagic))
        records = batch.records().downConvert(minUsedMagic, 0, time).records();
    produceRecordsByPartition.put(tp, records);
    recordsByPartition.put(tp, batch);
}

// ...身略部分
ProduceRequest.Builder requestBuilder = ProduceRequest.Builder.forMagic(minUsedMagic, acks, timeout,
        produceRecordsByPartition, transactionalId);
RequestCompletionHandler callback = new RequestCompletionHandler() {
    public void onComplete(ClientResponse response) {
        handleProduceResponse(response, recordsByPartition, time.milliseconds());
    }
};

String nodeId = Integer.toString(destination);
ClientRequest clientRequest = client.newClientRequest(nodeId, requestBuilder, now, acks != 0,
        requestTimeoutMs, callback);
client.send(clientRequest, now);
```

**3.** Broker接受发送过来的数据，分析和校验数据需要它
```java
//这是MemoryRecords, 但 MemoryRecordsBuilder.build()会返回MemoryRecords
//MemoryRecordsBuilder.build()
/**
    * Close this builder and return the resulting buffer.
    * @return The built log buffer
    */
public MemoryRecords build() {
    if (aborted) {
        throw new IllegalStateException("Attempting to build an aborted record batch");
    }
    close();
    return builtRecords;
}

//Log.append()   records: MemoryRecords
val appendInfo = analyzeAndValidateRecords(records, isFromClient = isFromClient)
```

## MemoryRecordsBuilder的相关说明
`万物皆对象`        
Producer端的消息最终都已ByteBuffer类型存储，这里必须了解它的父类Buffer的3个私有fields： position、limit、capacity，对于消息的write其实就是涉及到Buffer的扩容，固定字节长度写入等等。再根据ProducerBatch的tryAppend()调用逻辑，最后的消息是存储在MemoryRecordsBuilder的 appendStream对象中。可继续分析下去之后，Sender线程在发送数据时候，appendStream又不是拼接ProducerRequest的存储对象，而是转换成 MemoryRecords。下面介绍了这几个容器的类图关系，后面会介绍消息在MemoryRecordsBuilder的处理过程。       

### 初步了解MemoryRecordsBuilder内部结构
>以下逻辑基于kafka-clients版本是2.2.1       

**下面展示的关于"容器"的类**        
![memoryrecordsbuilder 类图](http://img.xinzhuxiansheng.com/blogimgs/kafka/producer/producer_memoryrecordsbuilder02.png)
  
### ProducerRequest的结构及Record的结构
Producer会将消息构建出一个ProducerRequest对象，虽然数据的key，value，headers都写入ByteBuffer中，如果写入这个动作是有DefaultRecord类写入，那用户会很容易理解。它是怎么做的。请体会 `万物皆对象,面向对象编程`    
**ProducerRequest的类图**   
![ProducerRequest 类图](http://img.xinzhuxiansheng.com/blogimgs/kafka/producer/producer_memoryrecordsbuilder03.png) 

**Record结构**
请参考DefaultRecord writeTo()方法和它的类注释.       
```java
private void appendDefaultRecord(long offset, long timestamp, ByteBuffer key, ByteBuffer value,
                                    Header[] headers) throws IOException {
    //...省略部分
    // 使用DefaultRecord.java
    int sizeInBytes = DefaultRecord.writeTo(appendStream, offsetDelta, timestampDelta, key, value, headers);
    //...省略部分
}

// 请消息阅读DefaultRecord类的注释

/**
 * This class implements the inner record format for magic 2 and above. The schema is as follows:
 *
 *
 * Record =>
 *   Length => Varint
 *   Attributes => Int8
 *   TimestampDelta => Varlong
 *   OffsetDelta => Varint
 *   Key => Bytes
 *   Value => Bytes
 *   Headers => [HeaderKey HeaderValue]
 *     HeaderKey => String
 *     HeaderValue => Bytes
 *
 * Note that in this schema, the Bytes and String types use a variable length integer to represent
 * the length of the field. The array type used for the headers also uses a Varint for the number of
 * headers.
 *
 * The current record attributes are depicted below:
 *
 *  ----------------
 *  | Unused (0-7) |
 *  ----------------
 *
 * The offset and timestamp deltas compute the difference relative to the base offset and
 * base timestamp of the batch that this record is contained in.
 */
```

如果需要详细了解，请查看官网 `KAFKA PROTOCOL GUIDE`  http://kafka.apache.org/protocol.html#The_Messages_Produce

### MemoryRecords的创建
MemoryRecords对象的创建，在Sender线程发送数据时，会将消息累加器中的ProducerBatch的 memoryRecordsBuilder中的appendStream对象给close(),确保它不能再接受写入新的消息了。 然后才会将ByteBuffer用来创建一个新的MemoryRecords对象。 `后面会详细介绍`

## MemoryRecordsBuilder的流程图
![memoryrecordsbuilder 流程图](http://img.xinzhuxiansheng.com/blogimgs/kafka/producer/producer_memoryrecordsbuilder01.png)

* MemoryRecordsBuilder.append() 只会给消息分配相对Offset，并且每个ProducerPath的BaseOffset 就是起点Offset是 0         
* 消息的批次对象是ProducerBatch，所以 每个MemoryRecordsBuilder的lastOffset，也就是这批次的消息的lastOffset。        
* appendStream写入消息key，value，headers，还增加了 offsetDelta（offset相对baseOffset增量），timestampDelta（timestamp相对firstTimestamp增量）
```java
int offsetDelta = (int) (offset - baseOffset);
long timestampDelta = timestamp - firstTimestamp;
int sizeInBytes = DefaultRecord.writeTo(appendStream, offsetDelta, timestampDelta, key, value, headers);
```

## 构建ProducerRequest
![构建ProducerRequest](http://img.xinzhuxiansheng.com/blogimgs/kafka/producer/producer_memoryrecordsbuilder04.png)

Sender线程在读取ProducerBatch数据，才会将MemoryRecordsBuilder 转换成MemoryRecords。 `MemoryRecords records = batch.records()`。     
```java
for (ProducerBatch batch : batches) {
    TopicPartition tp = batch.topicPartition;
    MemoryRecords records = batch.records();
    //...省略部分
}
```

build()方法调用close()。这个处理过程包含:       
**1.** appendStream.close()；       
**2.** 将每批次的关键字段信息，再写入到request的header中。      
```java
DefaultRecordBatch.writeHeader(buffer, baseOffset, offsetDelta, size, magic, compressionType, timestampType,
        firstTimestamp, maxTimestamp, producerId, producerEpoch, baseSequence, isTransactional, isControlBatch,
        partitionLeaderEpoch, numRecords);
```

`这些header数据，对于Broker接受数据并解析成ProducerRequest对象，至关重要。`
```java
static void writeHeader(ByteBuffer buffer,
                            long baseOffset,
                            int lastOffsetDelta,
                            int sizeInBytes,
                            byte magic,
                            CompressionType compressionType,
                            TimestampType timestampType,
                            long firstTimestamp,
                            long maxTimestamp,
                            long producerId,
                            short epoch,
                            int sequence,
                            boolean isTransactional,
                            boolean isControlBatch,
                            int partitionLeaderEpoch,
                            int numRecords) {
    if (magic < RecordBatch.CURRENT_MAGIC_VALUE)
        throw new IllegalArgumentException("Invalid magic value " + magic);
    if (firstTimestamp < 0 && firstTimestamp != NO_TIMESTAMP)
        throw new IllegalArgumentException("Invalid message timestamp " + firstTimestamp);

    short attributes = computeAttributes(compressionType, timestampType, isTransactional, isControlBatch);

    int position = buffer.position();
    buffer.putLong(position + BASE_OFFSET_OFFSET, baseOffset);
    buffer.putInt(position + LENGTH_OFFSET, sizeInBytes - LOG_OVERHEAD);
    buffer.putInt(position + PARTITION_LEADER_EPOCH_OFFSET, partitionLeaderEpoch);
    buffer.put(position + MAGIC_OFFSET, magic);
    buffer.putShort(position + ATTRIBUTES_OFFSET, attributes);
    buffer.putLong(position + FIRST_TIMESTAMP_OFFSET, firstTimestamp);
    buffer.putLong(position + MAX_TIMESTAMP_OFFSET, maxTimestamp);
    buffer.putInt(position + LAST_OFFSET_DELTA_OFFSET, lastOffsetDelta);
    buffer.putLong(position + PRODUCER_ID_OFFSET, producerId);
    buffer.putShort(position + PRODUCER_EPOCH_OFFSET, epoch);
    buffer.putInt(position + BASE_SEQUENCE_OFFSET, sequence);
    buffer.putInt(position + RECORDS_COUNT_OFFSET, numRecords);
    long crc = Crc32C.compute(buffer, ATTRIBUTES_OFFSET, sizeInBytes - ATTRIBUTES_OFFSET);
    buffer.putInt(position + CRC_OFFSET, (int) crc);
    buffer.position(position + RECORD_BATCH_OVERHEAD);
}
```

**3.** DefaultRecordBatch类 涉及到RecordBatch
RecordBatch类是

**4.** 根据MemoryRecordsBuilder的bufferStream，创建MemoryRecords返回    
```java
builtRecords = MemoryRecords.readableRecords(buffer.slice());
```


## 总结
MemoryRecordsBuilder 存储数据，用以构建最后发送的MemoryRecords对象，特别关注writeHeader()方法，它涉及到`RecordBatch`类。 它在Client端和Broker端都同时使用到。所以 了解它，你会知道，ProducerRequest如何构建并且在Broker端又如何获取消息批次的相关信息。