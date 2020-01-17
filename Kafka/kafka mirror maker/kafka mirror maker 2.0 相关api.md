

## 获取源集群某个 ConsumerGroupId所丢应的topic
```java
// 参数: 目标集群配置，源集群alias，ConsumerGroupid，时间
 Map<String,Object> targetClusterMap = new HashMap<String,Object>();
    targetClusterMap.put("bootstrap.servers","vm01.com:9082");
    Map<TopicPartition, OffsetAndMetadata> newOffsets = RemoteClusterUtils.translateOffsets(
            targetClusterMap, "hyvm02", "aaaabbbbcccc",Duration.ofSeconds(10));
    for(Map.Entry<TopicPartition,OffsetAndMetadata> item : newOffsets.entrySet()){
        System.out.println("aa");
}
```


## 读取 checkpoint topic数据

`特别注意` : Checkpoint.descrializeRecord(record) ;

```java
 Properties properties = new Properties();

    properties.put("bootstrap.servers", "vm01.com:9082");
    properties.put("client.id","yzhouclientid01");
    properties.setProperty("group.id", "gidvm01_0256");
    properties.put("enable.auto.commit", "true");
//        properties.put("auto.offset.reset", "earliest");
    properties.put("auto.commit.interval.ms", "1000");
    properties.setProperty("key.deserializer", "org.apache.kafka.common.serialization.ByteArrayDeserializer");
    properties.setProperty("value.deserializer", "org.apache.kafka.common.serialization.ByteArrayDeserializer");

    KafkaConsumer<byte[],byte[]> kafkaConsumer = new KafkaConsumer<byte[], byte[]>(properties);
    kafkaConsumer.subscribe(Arrays.asList("hyvm02.checkpoints.internal"));
    //kafkaConsumer.assign(Arrays.asList("yzhoutp01"));

    while(true) {
        ConsumerRecords<byte[],byte[]> records = kafkaConsumer.poll(Duration.ofSeconds(10));
        for(ConsumerRecord<byte[],byte[]> record : records){
            //System.out.println(record.value() + " , "+ record.offset());
            Checkpoint checkpoint = Checkpoint.deserializeRecord(record);
            System.out.println(checkpoint.consumerGroupId());
        }
    }
```