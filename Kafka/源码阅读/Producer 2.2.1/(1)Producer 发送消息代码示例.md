

## Producer 发送消息代码示例

`Apache Kafka` 这里就不过多介绍，请阅读Kafka官网:http://kafka.apache.org/, github:https://github.com/apache/kafka

本篇主要讲解 `如何利用Java编写Kafka Producer发送数据` ，至于其他的语言或者框架,请阅读Kafka官网的Clients 文档： https://cwiki.apache.org/confluence/display/KAFKA/Clients


目前Java项目中使用的kafka client jar有2个是非常主流:    
1）kafka-clients (Apache Kafka官方出品)  
2）Spring-kafka (Spring系列出品)

本人建议还是使用官网出品的kafka-clients。这里不是表达Spring-kafka不好，有以下几点因素会干扰你的判断：   
1）会忽略kafka-clients版本是否与kafka broker版本是否保持一致（也许有人会说kafka-client与broker会兼容高低版本，这里不要忽略client低，broker高的情况）  
2）Spring-kafka对kafka-clients做了二次封装，对于不熟悉源码的同学来说，即使是使用Spring-kafka为了简化Kafka开发的成本，但也不忽略排查问题的成本

## 利用kafka-client开发Producer
>项目是基于maven构建以及Kafka broker版本是2.2.1

1. kafka相关jar的依赖
```xml
<dependency>
    <groupId>org.apache.kafka</groupId>
    <artifactId>kafka-clients</artifactId>
    <version>2.2.1</version>
</dependency>

<!--若你会调用Kafka 除kafka-clients jar以外类库，一般是不需要依赖kafka完整的jar -->
<dependency>
    <groupId>org.apache.kafka</groupId>
    <artifactId>kafka_2.12</artifactId>
    <version>2.2.1</version>
</dependency>
```

2. 使用API构建KafkaProducer，发送数据
>代码执行流程图
![avatar](images/(1)Producer发送消息代码流程图.png)

```java
public static void main(String[] args) throws InterruptedException {

    //配置Producer的参数
    Properties properties = new Properties();
    properties.put("bootstrap.servers", "连接地址");
    properties.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
    properties.put("value.serializer", "org.apache.kafka.common.serialization.StringSerializer");
    //根据参数实例化KafkaProducer
    Producer<String,String> producer = new KafkaProducer<String, String>(properties);

    //示例代码，请勿直接套用生产环境
    Long i = 0L;
    while(true){
        String data = i+"样例数据！";
        producer.send(new ProducerRecord<String, String>("yzhoutp01",data), 
        //使用异步发送,并通过回调方法处理返回值
        new Callback() {
            @Override
            public void onCompletion(RecordMetadata recordMetadata, Exception e) {
                // 判断返回结果是否异常
                if(null == recordMetadata){
                    e.printStackTrace();
                }
            }
        });
        Thread.currentThread().sleep(1000L);
        i++;
    }

}
```

上面的示例代码已经完成构建并发送消息及返回值的处理。下面就分析代码逻辑：   
1）创建Properties对象，配置KafkaProducer相关参数,通过表格列举通用参数的配置
| 参数名称      | 描述  |  用于生产环境可参考的值   |
| :-------- |:--------| :-- | :-- |
| bootstrap.servers   |  Kafka集群连接地址   | 生产环境连接地址 |
| key 序列化     |  请参考 Kafka源码 org.apache.kafka.common.serialization路径的class  | 生产环境一般使用 StringSerializer较多 |
| value 序列化     | 请参考 Kafka源码 org.apache.kafka.common.serialization路径的class  | 生产环境一般使用 StringSerializer较多 |

请注意，key,value的参数会直接影响KafkaProducer泛型的指定，请不要忽略    
```java
Producer<String,String> producer = new KafkaProducer<String, String>(properties);
```
不过在生产环境中，还有很多优化的参数来提高Producer的吞吐量
* batch.size：批次字节大小，默认是16k
* compression.type：数据压缩格式 默认是none
* linger.ms：等待到给定的延迟时间才允许发送消息 默认是 0
* buffer.memory： 缓存池字节大小 默认是32M
上面是通用的优化参数，其他参数可以根据不同的业务处理情况来合理调整，所以 了解及熟悉不同参数对Producer的含义及默认值有非常重要，同时可以依据Client采集metric 来分析、判断并设置合理的值

`完整的生产环境经常用的参数：`
```java
Properties properties = new Properties();
properties.put("bootstrap.servers", "xxxxxx");
properties.put("retries", 3); // 重试次数
properties.put("batch.size", "1048576"); // 1MB 当该值设置较大值，请务必于压缩参数一起使用
properties.put("compression.type", "snappy");
properties.put("linger.ms", "20"); // 这个值一定要改，linger.ms默认是0 ，对于批量来说一点都不友好
properties.put("buffer.memory", "67108864");// 64MB 参考jvm大小 设置该值
properties.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
properties.put("value.serializer", "org.apache.kafka.common.serialization.StringSerializer");
```

>备注： KafkaProducer 是线程安全的，请不要大量实例化KafkaProducer，经常会遇到一些入门使用者，send一次数据，就new KafkaProducer。

2）Producer 发送方式    
KafkaProducer只用了一个send方法，就可以完成同步和异步两种模式的消息发送，这是因为send方法返回的是一个Futrue。基于Future，我们可以实现同步或异步的消息发送语义。     
**同步** ：调用send返回Future时，需要立刻调用get，因为Future.get在没有返回结果时会一直阻塞。  
**异步** ：提供一个回调，调用send后可以继续发送消息而不用等待。当有结果返回时，会自动处理回调函数。 

>代码执行图:




>部分   
1.《Kafka技术内幕》