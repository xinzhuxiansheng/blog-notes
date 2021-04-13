--In Blog
--Tags: Kafka

# Kafka Consumer设置enable.auto.commit=true，是如何提交Commit Offsets的？

>涉及Kafka是2.2.1版本


## enable.auto.commit,auto.commit.interval.ms介绍
`关键字`

```java
public static void main(String[] args) {
    Properties properties = new Properties();
    properties.put("enable.auto.commit", "true");
    properties.put("auto.commit.interval.ms", "1000");
    // 省略其他配置项 ......
    KafkaConsumer<String,String> kafkaConsumer = new KafkaConsumer<String, String>(properties);
    kafkaConsumer.subscribe(Arrays.asList("test01"));
    while(true) {
        ConsumerRecords<String,String> records = kafkaConsumer.poll(Duration.ofSeconds(10));
        for(ConsumerRecord<String,String> record : records){
            System.out.println(record.value());
        }
    }
}
```
