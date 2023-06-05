## Apache Pulsar Reader 开发 

### 引言    
Apache Pulsar的`Reader API`是一种灵活的数据访问机制，它可以从 Pulsar的主题（Topic）中的任意位置开始读取数据，并按顺序一直读取到最新的消息。与 Pulsar的Consumer API 不同，Reader不需要订阅，而且可以无视消息的确认和重传，这使得它在一些特定的场景下非常有用。例如最早和最后可以访问到的有效消息位置，你也可以通过构建Messageld来指定任意有效位置进行消息访问。  

在Pulsar作为流处理系统对外提供“精确一次”处理语义等用例时，Reader 接口非常有用。在内部实现中，Reader也是通过Consumer功能封装的，内部使用一个随机命名的订阅来对主题进行独占、非持久性订阅，以到达手动定位消息的目的。 
`示例`  
```java
Reader<String> reader = client.newReader(Schema.STRING)
        .topic("my-topic")
        .create();

while (true) {
    Message<String> message = reader.readNext();
    System.out.println(message.getValue());
}
```

### Reader实战

#### 模式管理   
在消息总线及大数据系统中，数据类型安全是极为重要的。在 Pulsar把消息写入服务端后，在 BookKeeper中存储的消息都是字节类型的。在应用程序中可以由用户提供序列类型管理工具。在客户端中可维护的序列化与反序列化方法如下所示。  
```java
Producer<byte[]> producer = client.newProducer()
        .topic("my-topic")
        .create();

DemoData sendDemoData = new DemoData(new Random().nextInt(),"test");
producer.send(sendDemoData.serialize());


Consumer<byte[]> consumer = client.newConsumer().topic("my-topic")
        .subscriptionName("subscriptionName-01")
        .subscribe();

Message<byte[]> message = consumer.receive();
DemoData receivedDemoData = DemoData.deserializer(message.getData());
consumer.acknowledge(message);
```

生产者和消费者可以发送和接收由原始字节数组组成的消息，并在此基础上将所有类型安全类处理工作留给用户的应用程序。我们只需要提供如下的序列化与反序列化方法就能构建出一个类型安全的数据类。  
```java
@Data
public class DemoData implements Serializable {
    private static final long serialVersionUID = 1L;
    private int intField;
    private String stringField;

    public DemoData(int intField, String stringField) {
        this.intField = intField;
        this.stringField = stringField;
    }

    public byte[] serialize() throws IOException {
        ByteArrayOutputStream byteArray = new ByteArrayOutputStream();
        ObjectOutputStream objectOutputStream = new ObjectOutputStream(byteArray);
        objectOutputStream.writeObject(this);
        return byteArray.toByteArray();
    }

    public static DemoData deserializer(byte[] bytes) throws Exception {
        ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(bytes);
        ObjectInputStream inputStream = new ObjectInputStream(byteArrayInputStream);
        return (DemoData) inputStream.readObject();
    }
}
``` 
在服务端和客户端共同作用下，Pulsar 可提供给用户一套通用的类型安全方法一模式(Schema)。模式是一种数据类型的定义方法，提供了统一的类型管理和序列化(或反序列化)方式，可以减轻用户维护类型安全的工作量。通过使用模式，Pulsar客户端会强制执行类型安全检查，并确保生产者和消费者保持同步。 


#### 模式类型
模式主要分为原始类型模式、复杂类型模式两类。

**原始类型模式**    
原始类型模式是 Pulsar支持的单一数据类型模式，也是构成复杂类型模式的基础。由于Pulsar基于Java开发，所以 Pulsar的数据类型和Java基本数据类型对应，目前有以下几类    
* **字节数组(BYTES)**: 默认的模式格式。对应 Java中的 byte[]、ByteBuffer类型 
* **布尔类型(BOOLEAN)**: 对应 Java Boolean 类型。   
* **警数类型(INT8、INT16、INT32、INT64)**: 按照数据所占字节不同又可分为8位、16位、32 位和64 位，对应 Java 的 byte、short、int、long 类型口浮点类型(FLOAT，DOUBLE): 分为 32位的单浮点数和64 位的双浮点数，对Java 的 foat 和 double。 
* **字符串(STRING)**: Unicode 字符串，对应着 Java 中的 String 类型      
* **时间截(TIMESTAMP、DATE、TIME、INSTANT、LOCAL DATE、LOCAL_TIMELOCAL DATE TIME)**: 时间字段类型，对应Java 中的java.sql.Timestamp、javasql.Time、java.util.Date 、 java.time.Instant、java.time.LocalDate 、java.time:Local.DateTime、java.time.LocalTime 类型。其中INSTANT 代表时间线上的单个瞬时点精度为纳秒。

Pulsar 中对基本数据类型模式的使用方式是在创建生产者和消费者时传人原始类型模式，Java 客户端中可以使用泛保证类型安全。    
`示例`  
```java
Producer<String> producer = client.newProducer(Schema.STRING)
        .topic("my-topic")
        .create();
producer.newMessage().value("String test");

Consumer<Float> consumer = client.newConsumer(Schema.FLOAT)
        .topic("my-topic")
        .subscriptionName("subscription name")
        .subscribe();
Message<Float> msg = consumer.receive();
```

**复杂类型模式**    
简单类型模式只能支持单一的数据应用，复杂类型模式是 Pulsar提供的更加丰富的结构，复杂类型模式分为两类: `键值对类型模式`和`结构体类型模式`。   

键值对类型模式的键值会被用作消息路由的一个条件变量。键值有两种编码形式一一`内联编码`和`分离编码`。
* 内联编码会将键和值在消息主体中一起编码    
* 分离编码会将键编码在消息密钥中，将值编码在消息主体中  

`示例代码如下`  
```java
Schema<KeyValue<Integer, String>> kvSchema1 = Schema.KeyValue(Schema.INT32,Schema.STRING, KeyValueEncodingType.INLINE);
Schema<KeyValue<Integer, String>> kvSchema2 = Schema.KeyValue(Schema.INT32,Schema.STRING, KeyValueEncodingType.SEPARATED);
ProducerBuilder<KeyValue<Integer, String>> producer = client.newProducer(kvSchema1);
```

结构体类型模式可以让用户很方便地传输 Java对象。目前 Pulsar支持`AvroBaseStructSchema`和`ProtobufNativeSchema`两种结构体类型模式。 AvroBaseStructSchema 支持AvroSchema、JsonSchema 和 ProtobufSchema。利用 Pulsar支持的几种模式可以预先定义结构体架构，它既可以是 Java 中的简单的Java对象 (POJO)、Go中的结构体，又可以是Avro或Protobuf工具生成的类。ProtobufNativeSchema 使用原生Protobuf协议的格式来进行序列化。 
`示例`  
```java
 Producer<DemoData> producer = client.newProducer(JSONSchema.of(DemoData.class))
                .topic("my-topic").create();

Producer<DemoData> producer2 = client.newProducer(AvroSchema.of(DemoData.class))
        .topic("my-topic").create();

// ProtobufSchema 使用的对象 ProtocolData 要能自 GeneratedMesaagov3
Producer<ProtocolData> producer3 = client.newProducer(ProtobufSchema.of(ProtocolData.class))
producer<ProtocolData> producer4mclient, newProducer (Protobufnativeschema
        of(ProtocolData.class))
        .topic("my-topic").create();
```

**自定义GenericSchema** 
Pulsar使用的结构体类型模式拥有提前定义好的结构(由预先定义的结构体或者类化而来)。若没有预定义的结构，那么就要使用 `GenericSchemaBuilder` 定义数据结构了。使用GenericRecordBuilder生成通用结构，生产和消费会将数据绑定到 GenericRecord 中GenericSchema有 GenericJsonSchema和 GenericAvroSchema 两种选择，具体使用哪种可以在RecordSchemaBuilder.build 方法中指定。示例代码如下 
```java
RecordSchemaBuilder recordSchemaBuilder = SchemaBuilder.record("schemaName");
recordSchemaBuilder.field("intField").type(SchemaType.INT32);
recordSchemaBuilder.field("StringField").type(SchemaType.STRING);
SchemaInfo schemaInfo = recordSchemaBuilder.build(SchemaType.JSON);
Producer<GenericRecord> producer  =  client.newProducer(
        GenericJsonSchema.of(schemaInfo))
        .topic("my-topic").create();// 鸭建 GenericRecord
GenericRecord record =  GenericJsonSchema.of(schemaInfo).newRecordBuilder()
        .set("intpield",10).set("stringpield","sting test").build();
producer.newMessage().value(record).send();
```

#### 自动模式   
如果你在使用生产者和消费者时，事先不知道 Pulsar主题的模式类型，则可以用AUTO模式。生产和消费中的AUTO式分别对应着`AUTO_PRODUCE`和`AUTO_CONSUME`，下面就来介绍它们的用法。     

**生产者侧的自动模式**    
生产者侧的自动模式(Auro Schema)为AUTO_PRODUCE模式。在使用该模式时Pulsar 会我们验证发送的字节是否与此主题的模式兼容。下面的示例演示了如何使AUTOPRODUCE模式。原有主题的格式为SchemaJSON(DemoData.class)，使用AUTOPRODUCE模式后，可以直接将JSON字符串转化后的字节发送到服务端。目前仅支AVRO和JSON类型模式。
    
**消费者侧自动模式**        
消费者侧的自动模式(Auto Schema)为AUTO CONSUME模式。它可以验证发送到消费者端的字节是否与消费者兼容。AUTO CONSUME 仅支持AVRO、JSON、ProtobufNative.Schema这类复杂模式类型。它会将消息统一反序列化为 GenericRecord。示例代码如下。


以下是 Reader API 的一些关键特性：  

1. **灵活的数据访问**：使用 Reader API，你可以从话题中的任意位置开始读取数据，无论这些数据是过去的历史数据，还是实时到达的新数据。你可以精确控制开始读取数据的位置，通过指定消息 ID 或者相对的时间戳。

2. **无需订阅**：Reader 不需要对话题进行订阅。这意味着你可以读取话题的数据，而不会影响话题的任何订阅者。这在你只想临时访问数据，而不想长期追踪数据时非常有用。

3. **无确认和重传**：Reader 读取的消息不需要确认，也不会因为没有被确认而被重传。这使得 Reader API 比消费者 API 更简单，也更容易预测。

4. **按顺序读取**：Reader API 按照消息被发布的顺序读取数据。这对于需要处理有序数据的应用程序来说非常有用。

你可以将 Reader API 视为一种低级 API，它提供了对 Pulsar 数据流的直接、无状态访问。尽管它在功能上比消费者 API 更少，但它提供了更大的灵活性，可以支持更多的使用场景。

refer   
1.https://pulsar.apache.org/docs/2.11.x/concepts-clients/#reader-interface  
2.《Apache Pulsar原理解析与应用实践》 