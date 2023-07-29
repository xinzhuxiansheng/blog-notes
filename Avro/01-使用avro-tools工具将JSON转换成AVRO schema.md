## 使用 avro-tools.jar 工具将JSON转换成 AVRO schema 

### 引言    
在Kafka Producer 使用Kafka Schema组件发送数据时，需要提前设定 Schema格式，例如：    
```java
String userSchema = "{\"type\":\"record\"," +
        "\"name\":\"myrecord\"," +
        "\"fields\":[{\"name\":\"name\",\"type\":\"string\"},{\"name\":\"age\",\"type\":\"int\"}]}";;
org.apache.avro.Schema.Parser parser = new org.apache.avro.Schema.Parser();
org.apache.avro.Schema schema = parser.parse(userSchema);

Properties properties = new Properties();
properties.put("bootstrap.servers", "192.168.xxx.xxx:9092");
properties.put("key.serializer", StringSerializer.class.getName());
properties.put("value.serializer", KafkaAvroSerializer.class.getName());
properties.put("schema.registry.url", "http://192.168.xxx.xxx:8081");
org.apache.kafka.clients.producer.Producer<String, GenericRecord> producer = new KafkaProducer<String, GenericRecord>(properties);
Long i = 0L;
while (true) {
    // String data = i + "样例数据！";
    GenericRecord avroRecord = new GenericData.Record(schema);
    avroRecord.put("name", "Alice");
    avroRecord.put("age", 25 + i);
```

`userSchema` 变量设置了 schema。 

<!-- ### avro-tools.jar使用  
针对复杂对象，要定义schema格式会比较复杂，而`avro-tools.jar`就是为了简化这一步骤，支持 JSON 转 AVRO Schema。       

avro-tools.jar 下载地址：https://avro.apache.org/project/download/   

**使用步骤**        
```shell

# 嵌套的JSON数据转换成AVRO数据
java -jar avro-tools-1.11.2.jar fromjson products.json --schema-file products.avsc > products.avro
```  -->

refer   
1.https://converts.me/tools/generation/metadata/avro-schema-from-json