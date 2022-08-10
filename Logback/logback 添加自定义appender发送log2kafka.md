## logback添加自定义appender发送log2kafka 

### 1.添加pom.xml 依赖

logback将log发送至kafka，需要添加 "logback-kafka-appender"jar依赖（https://github.com/danielwegener/logback-kafka-appender tag：0.2.0-RC2），这里需要注意kafka及kafka-clients的版本根据自己的kafka集群的版本而定。为了统一版本，所以需要将logback-kafka-appender的kafka相关依赖排除，显示依赖2.7.1版本的kafka及kafka-clients（我的version 是2.7.1）。

```xml
<dependency>
    <groupId>ch.qos.logback</groupId>
    <artifactId>logback-classic</artifactId>
    <version>1.2.3</version>
</dependency>

<dependency>
    <groupId>com.github.danielwegener</groupId>
    <artifactId>logback-kafka-appender</artifactId>
    <version>0.2.0-RC2</version>
    <exclusions>
        <exclusion>
            <artifactId>kafka-clients</artifactId>
            <groupId>org.apache.kafka</groupId>
        </exclusion>
        <exclusion>
            <artifactId>slf4j-api</artifactId>
            <groupId>org.slf4j</groupId>
        </exclusion>
    </exclusions>
</dependency>
<dependency>
    <groupId>org.apache.kafka</groupId>
    <artifactId>kafka-clients</artifactId>
    <version>2.7.1</version>
</dependency>
<dependency>
    <groupId>org.apache.kafka</groupId>
    <artifactId>kafka_2.12</artifactId>
    <version>2.7.1</version>
</dependency>
```

### 2.logback.xml配置

2.1 添加自定义appender

请参考 "logback-kafka-appender"的github文档，

<topic> ：写入的Topic名称，

<producerConfig>： producer的config信息，对应就是java代码中 properties配置项，填写形式是 key=value

这里请注意: 别忘记Kafka Producer的优化参数

```
linger.ms = xxxx
retries = xxx
compression.type = xxx(snappy / gzip)
batch.size= xxx

<!-- This is the kafkaAppender -->
<appender name="kafkaAppender" class="com.github.danielwegener.logback.kafka.KafkaAppender">
    <encoder>
        <pattern>%d{HH:mm:ss.SSS} [%thread] %-5level %logger{36} - %msg%n</pattern>
    </encoder>
    <topic>yzhoutp02</topic>
    <keyingStrategy class="com.github.danielwegener.logback.kafka.keying.NoKeyKeyingStrategy" />
    <deliveryStrategy class="com.github.danielwegener.logback.kafka.delivery.AsynchronousDeliveryStrategy" />
    <producerConfig>bootstrap.servers=ip:port</producerConfig>
</appender>
```

2.2 是否添加异步 "AsyncAppender"
请参考 https://logback.qos.ch/manual/appenders.html#AsyncAppender
```xml
<appender name="KAFKA-ASYNC" class="ch.qos.logback.classic.AsyncAppender">
    <appender-ref ref="kafkaAppender" />
</appender>
```
若启用 AsyncAppender，虽会增大性能，但请必须阅读logback官网关于AsyncAppender的章节。以下是官网tips：
1.LOSSY BY DEFAULT IF 80% FULL
2.APPLICATION STOP/REDEPLOY
3.POST SHUTDOWN CLEANUP
但同时 logback的AsyncAppender也提供了参数调优: 
![AsyncAppenderConfig](http://img.xinzhuxiansheng.com/blogimgs/logback/AsyncAppenderConfig.png)

下面列举 queueSize,discardingThreshold参数，其他参数也同样配置，参数调整的格式:

```xml
<appender name="KAFKA-ASYNC" class="ch.qos.logback.classic.AsyncAppender">
    <queueSize>256</queueSize>
    <discardingThreshold>0</discardingThreshold>
    <appender-ref ref="kafkaAppender" />
</appender>
```

### 3.指定某个类的log发送到kafka
在应用很多场景中，希望不同类或者不同级别写入不同文件的场景，而对于log发送到kafka来说，也可能会有只希望某个类的log或者某一种log是发送kafka的。 所以这里就利用 <logger>标签来识别某个类

3.1 配置logger

```xml
<logger name="com.yzhou.demo.common.util.Log2KafkaProducer" level="INFO">
    <appender-ref ref="kafkaAppender" />
</logger>
```

3.2 定义Log2KafkaProducer类

```java
public class Log2KafkaProducer {
    private static Logger logger = LoggerFactory.getLogger(Log2KafkaProducer.class);
    public static void send(String msg){
        logger.info(msg);
    }
}
```

若要发送kafka，仅调用”Log2KafkaProducer.send(String msg)“方法即可