
## 指定分区、指定偏移量消费

## assign(),seek()

```java
public class test02_consumer_main {

    public static void main(String[] args) {
        Properties properties = new Properties();

        properties.put("bootstrap.servers", "xx.xx.4.yyy:9092");
        properties.put("client.id","xxxxxx");
        properties.setProperty("group.id", "xxxxxxx");
        properties.put("enable.auto.commit", "true");
        properties.put("auto.commit.interval.ms", "1000");
        properties.put("auto.offset.reset","earliest");
        properties.setProperty("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
        properties.setProperty("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");

        KafkaConsumer<String,String> kafkaConsumer = new KafkaConsumer<String, String>(properties);
        TopicPartition p = new TopicPartition("topicName01", 3);
        kafkaConsumer.assign(Collections.singletonList(p));
        kafkaConsumer.seek(p,偏移量);

        boolean iswhile = true;
        while(iswhile) {
            ConsumerRecords<String,String> records = kafkaConsumer.poll(Duration.ofSeconds(10));
            for(ConsumerRecord<String,String> record : records){
                System.out.println("timestamp: "+record.timestamp()+",p: "+record.partition()+" ,offset: "+record.offset()+" ,value: "+record.value());
            }
        }
    }
}
```


