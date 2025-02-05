# Spring-kafka - 不停服重置 Consumer Offset     

>Kafka version: 3.6.0 

>该篇 Blog 内容更像是贴在公司 Confluence Kafka 工作空间 doc，毕竟让别人知晓怎么用，也是我们的责任。                    

## 背景     
在 Kafka consumer 消费数据场景中，消费方会提出 `reset consumer offset`需求，他们希望`跳过`或者`回溯` 某部分的数据，大多时候都在处理数据异常后逻辑。这的确很难拒绝。     

>对于`冷数据`的回溯，在数据量较大情况必然会造成 `pagecache 污染`，从而影响到 Kafka 读写性能。（该篇 Blog 暂时不讨论性能，当然贴在Confluence Kafka 工作空间 doc 最好标明一个说明 `应尽可能避免在高峰期间执行 reset consumer offset相关操作`）。    

>知识点：重置消费组的 offset，前提是必须先停止消费动作。   

## Kafka 重置 Offset 相关命令  

### 查看消费组列表
```bash 
[root@vm01 bin]# ./kafka-consumer-groups.sh --bootstrap-server 192.168.0.201:9092 --list
gid012401
groupA
```

### 查看某个消费者信息
```bash
[root@vm01 bin]# ./kafka-consumer-groups.sh --describe --group gid012401 --bootstrap-server 192.168.0.201:9092

Consumer group 'gid012401' has no active members.

GROUP           TOPIC           PARTITION  CURRENT-OFFSET  LOG-END-OFFSET  LAG             CONSUMER-ID     HOST            CLIENT-ID
gid012401       yzhoutpjson01   0          11              11              0               -               -               -
```

>此时特别注意：控制台打印出 `Consumer group 'gid012401' has no active members.` 表示当前消费组中并没有消费者正在消费数据，这里强调的是 consumer 与 broker 建立连接，即使 consumer 与 broker 建立连接但没有消费到数据也算是 active members。 

要是有消费者正在消费，控制会打印什么样呢？ 如下所示：           
```bash
[root@vm01 bin]# ./kafka-consumer-groups.sh --describe --group gid012401 --bootstrap-server 192.168.0.201:9092

GROUP           TOPIC           PARTITION  CURRENT-OFFSET  LOG-END-OFFSET  LAG             CONSUMER-ID                                               HOST            CLIENT-ID
gid012401       yzhoutpjson01   0          11              11              0               consumer-gid012401-1-b5398cc2-5609-448e-8330-48ce9c79f06c /192.168.0.2    consumer-gid012401-1
```

### 修改 consumer offset  
刚才通过上面的示例查看 `CURRENT-OFFSET` 为 11，若要是修改成 8，会怎么样？
```bash
[root@vm01 bin]# ./kafka-consumer-groups.sh --bootstrap-server 192.168.0.201:9092 --group gid012401 --reset-offsets --topic yzhoutpjson01 --to-offset 8 --execute

Error: Assignments can only be reset if the group 'gid012401' is inactive, but the current state is Stable.

GROUP                          TOPIC                          PARTITION  NEW-OFFSET
```

若当前消费组存在 active members，重置 consumer offset 会提示`Assignments can only be reset if the group 'gid012401' is inactive`, 所以在重置时，需要将消费组下的consumer 都关闭。  

下面是关于`--reset-offsets`参数介绍：  
```bash
--reset-offsets                         Reset offsets of consumer group.
                                          Supports one consumer group at the
                                          time, and instances should be
                                          inactive
                                        Has 2 execution options: --dry-run
                                          (the default) to plan which offsets
                                          to reset, and --execute to update
                                          the offsets. Additionally, the --
                                          export option is used to export the
                                          results to a CSV format.
                                        You must choose one of the following
                                          reset specifications: --to-datetime,
                                          --by-duration, --to-earliest, --to-
                                          latest, --shift-by, --from-file, --
                                          to-current, --to-offset.
                                        To define the scope use --all-topics
                                          or --topic. One scope must be
                                          specified unless you use '--from-
                                          file'.

```

常见重置参数解释：          
```bash
--to-earliest：设置到最早位移处
--to-latest：设置到最新处，也就是主题分区HW的位置
--to-offset NUM：指定具体的位移位置
--shift-by NUM：根据偏移量数值相对移动（正数向前，负数向后）
--to-datetime：将偏移量重置到指定的日期时间  
--by-duration： 回退到多长时间   
```

>所以，成功 reset consumer offset的前提是停止当前消费组下的 consume 行为。    

那要是不停服务重置 consumer offset 有什么`便捷的`实现呢？下面介绍如何使用 `spring-kafka` 类库实现`不停服务重置 Consumer Offset`。           

## 使用 spring-kafka 类库实现 不停服务重置 Consumer Offset    
**项目结构:**   
![springkafkaresetoffset01](http://img.xinzhuxiansheng.com/blogimgs/kafka/springkafkaresetoffset01.png)      

### 添加项目依赖  
```xml
<dependency>
    <groupId>org.springframework.kafka</groupId>
    <artifactId>spring-kafka</artifactId>
    <version>3.1.10</version>
</dependency>
<dependency>
    <groupId>org.springframework</groupId>
    <artifactId>spring-context</artifactId>
    <version>6.1.15</version>
</dependency>
<dependency>
    <groupId>io.projectreactor.netty</groupId>
    <artifactId>reactor-netty</artifactId>
    <version>1.2.2</version>
</dependency>
```

### spring-kafka.properties 配置文件
```bash
spring.kafka.bootstrap-servers: 192.168.0.201:9092
spring.kafka.consumer.group-id: groupA
spring.kafka.consumer.enable-auto-commit: false
spring.kafka.consumer.auto-offset-reset: latest
spring.kafka.consumer.max-poll-records: 5
spring.kafka.consumer.listener.batch: true
spring.kafka.consumer.listener.missing-topics-fatal: false
spring.kafka.consumer.listener.concurrency: 1
```

### 自动加载配置参数 
**KafkaConfiguration**
```java
@Configuration
@PropertySource("classpath:spring-kafka.properties")
@EnableKafka
public class KafkaConfiguration {
    @Value("${spring.kafka.bootstrap-servers}")
    private String servers;

    @Value("${spring.kafka.consumer.group-id}")
    private String groupId;

    @Value("${spring.kafka.consumer.enable-auto-commit}")
    private boolean autoCommit;

    @Value("${spring.kafka.consumer.auto-offset-reset}")
    private String latest;

    @Value("${spring.kafka.consumer.max-poll-records}")
    private String maxPollRecords;

    @Value("${spring.kafka.consumer.listener.batch}")
    private boolean batch;

    @Value("${spring.kafka.consumer.listener.missing-topics-fatal}")
    private boolean missingTopicsFatal;

    @Value("${spring.kafka.consumer.listener.concurrency}")
    private int concurrency;

    /**
     * 自定义配置
     */
    public Map<String, Object> consumerConfigs() {
        Map<String, Object> props = new HashMap<>();
        props.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, servers);
        props.put(ConsumerConfig.GROUP_ID_CONFIG, groupId);
        props.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, autoCommit);
        props.put(ConsumerConfig.AUTO_OFFSET_RESET_CONFIG, latest);
        props.put(ConsumerConfig.MAX_POLL_RECORDS_CONFIG, maxPollRecords);
        props.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class);
        return props;
    }

    /**
     * 消费者工厂
     */
    @Bean
    public ConsumerFactory<String, Object> consumerFactory() {
        return new DefaultKafkaConsumerFactory<>(consumerConfigs());
    }

    /**
     * 注入KafkaListenerContainerFactory
     *
     * @return KafkaListenerContainerFactory
     */
    @Bean
    public KafkaListenerContainerFactory<ConcurrentMessageListenerContainer<String, Object>> kafkaListenerContainerFactory() {
        ConcurrentKafkaListenerContainerFactory<String, Object> factory =
                new ConcurrentKafkaListenerContainerFactory<>();
        factory.setConsumerFactory(consumerFactory());
        // 设置并发消费的线程数，不能超过partitions的大小
        factory.setConcurrency(concurrency);
        // 设置是否是批量消费
        factory.setBatchListener(batch);
        // 设置poll超时时间
        factory.getContainerProperties().setPollTimeout(30000);
        // 设置ACK模式
        factory.getContainerProperties().setAckMode(ContainerProperties.AckMode.MANUAL);
        // 设置缺少topic是否启动失败
        factory.setMissingTopicsFatal(missingTopicsFatal);
        return factory;
    }
}
```

### 使用 @KafkaListener 触发消费行为   
>注意：@KafkaListener 必须标注 id 参数，并且将它设置为唯一值。
**KafkaConsumer**
```java
@Component
public class KafkaConsumer {
    private static final Logger logger = LoggerFactory.getLogger(KafkaConsumer.class);

    @KafkaListener(
            id = "cdclistener",
            topics = {"yzhoutpjson01"},
            groupId = "gid012401"
    )
    public void consumer(String record, Acknowledgment ack) {
        String threadName = Thread.currentThread().getName();
        try {
            logger.info("线程: {} 接收到消息: {}", threadName, record);
        } finally {
            logger.info("线程: {} 提交 ack", threadName);
            ack.acknowledge();
        }
    }
}
```  

### 编写 HttpServer，触发 stop，start POST 请求   
示例中的HTTP 是由 React-Netty 类库构建的。 start(),stop() 方法，会先通过 `@KafkaListener`注解设置的id 值获取 `ListenerContainer`。 再分别调用 stop(),start() 完成其内部容器内动态创建 consumer 或者 关闭 consumer， 从而达到 Consumer 消费线程的启停。  
```java
@Component
public class HttpServer {
    private static final Logger logger = LoggerFactory.getLogger(HttpServer.class);

    @Autowired
    private KafkaListenerEndpointRegistry kafkaListenerEndpointRegistry;

    public void startHttpServer() {
        logger.info("Http Server Starting ...");
        DisposableServer server = reactor.netty.http.server.HttpServer.create()
                .route(routes ->
                        routes.get("/hello",
                                        (request, response) ->
                                                response.sendString(
                                                        Mono.just("Hello World!")
                                                ))
                                .get("/list/{listenerId}",
                                        (request, response) -> response.sendString(
                                                Mono.just(has(request))
                                        ))
                                .post("/stop/{listenerId}",
                                        (request, response) -> response.sendString(
                                                Mono.just(stop(request))))
                                .post("/start/{listenerId}",
                                        (request, response) -> response.sendString(
                                                Mono.just(start(request))))
                )
                .host("localhost")
                .port(8080)
                .bindNow();
        server.onDispose();
    }

    private String has(HttpServerRequest request) {
        String listenerId = request.param("listenerId");
        return "true";
    }

    private String start(HttpServerRequest request) {
        String listenerId = request.param("listenerId");
        kafkaListenerEndpointRegistry.getListenerContainer(listenerId).start();
        return "stop";
    }

    private String stop(HttpServerRequest request) {
        String listenerId = request.param("listenerId");
        kafkaListenerEndpointRegistry.getListenerContainer(listenerId).stop();
        return "stop";
    }
}
```

通过 postman 请求 `localhost:8080/stop/cdclistener`,完成不停服停止 consumer，注意，此时spring-kafka 会调用 KafkaConsumer#close()方法关闭 consumer 对象。而不是暂停 consumer ，这两者是有明显区别的。  请求 `localhost:8080/start/cdclistener`  是会重新创建 consumer 对象进行消费数据。 

当调用 stop 接口关闭 consumer后，再 reset consumer offset 即可。 从而达到不停服重置 consumer offset。   

>完整代码：https://github.com/xinzhuxiansheng/javamain-services/tree/c9345e740d45bde318dc918225f70a65eb9cb2d9/javamain-springkafka          

refer           
1.https://spring.io/projects/spring-kafka           