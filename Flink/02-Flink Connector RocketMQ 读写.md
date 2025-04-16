# Flink JAR - RocketMQ 读写  

>Flink version: 1.15.4, RocketMQ version: 5.3.2  

## 打包 rocketmq-flink 项目  
在 `maven repository` (https://mvnrepository.com) 仓库中，并没有找到 rocketmq 与 flink 集成的 connector jar，就像 `flink-connector-kafka` 那样，所以我们需要访问 `https://github.com/apache/rocketmq-flink` 项目对其手动打包，克隆下来 执行 `mvn clean install` 打出的jar名称是 `flink-connector-rocketmq`。 看名称这就是我想要的。  

## RocketMQ 读写  

添加 RocketMQ 相关依赖（包括上面手动编译打包的 flink-connector-rocketmq）  
```xml
<!-- RocketMQ Flink Connector -->
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-rocketmq</artifactId>
    <version>1.15.0</version>
</dependency>

<dependency>
    <groupId>org.apache.rocketmq</groupId>
    <artifactId>rocketmq-common</artifactId>
    <version>5.3.2</version>
</dependency>

<!-- RocketMQ Client -->
<dependency>
    <groupId>org.apache.rocketmq</groupId>
    <artifactId>rocketmq-client</artifactId>
    <version>5.3.2</version>
</dependency>
```

**JobMain.java** 
代码逻辑并不复杂，该案例是 `https://github.com/apache/rocketmq-flink?tab=readme-ov-file#examples` 提供的，不过需要注意的是 `Message` 类 是由 `rocketmq-common` jar 提供的。 
```java
import com.alibaba.fastjson.JSONObject;
import org.apache.flink.connector.rocketmq.legacy.RocketMQConfig;
import org.apache.flink.connector.rocketmq.legacy.RocketMQSink;
import org.apache.flink.connector.rocketmq.legacy.RocketMQSourceFunction;
import org.apache.flink.connector.rocketmq.legacy.common.config.OffsetResetStrategy;
import org.apache.flink.connector.rocketmq.legacy.common.serialization.SimpleKeyValueDeserializationSchema;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.streaming.api.functions.ProcessFunction;
import org.apache.flink.util.Collector;
import org.apache.rocketmq.common.message.Message;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class JobMain {
  public static void main(String[] args) {
    StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();

    // enable checkpoint
    env.enableCheckpointing(3000);

    Properties consumerProps = new Properties();
    consumerProps.setProperty(RocketMQConfig.NAME_SERVER_ADDR, "192.168.0.201:9876");
    consumerProps.setProperty(RocketMQConfig.CONSUMER_GROUP, "c002");
    consumerProps.setProperty(RocketMQConfig.CONSUMER_TOPIC, "yzhoutpjson01");

    Properties producerProps = new Properties();
    producerProps.setProperty(RocketMQConfig.NAME_SERVER_ADDR, "192.168.0.201:9876");

    RocketMQSourceFunction<Map<Object,Object>> source = new RocketMQSourceFunction(
            new SimpleKeyValueDeserializationSchema("id", "address"), consumerProps);
    // use group offsets.
    // If there is no committed offset,consumer would start from the latest offset.
    source.setStartFromGroupOffsets(OffsetResetStrategy.LATEST);
    env.addSource(source)
            .name("rocketmq-source")
            .setParallelism(2)
            .process(new ProcessFunction<Map<Object, Object>, Map<Object, Object>>() {
              @Override
              public void processElement(
                      Map<Object, Object> in,
                      Context ctx,
                      Collector<Map<Object, Object>> out) {
                HashMap result = new HashMap();
                result.put("id", in.get("id"));
                String[] arr = in.get("address").toString().split("\\s+");
                result.put("province", arr[arr.length - 1]);
                out.collect(result);
              }
            })
            .name("upper-processor")
            .setParallelism(2)
            .process(new ProcessFunction<Map<Object, Object>, Message>() {
              @Override
              public void processElement(Map<Object, Object> value, Context ctx, Collector<Message> out) {
                String jsonString = JSONObject.toJSONString(value);
                Message message =
                        new Message(
                                "yzhoutpjson02",
                                "",
                                jsonString.getBytes(StandardCharsets.UTF_8));
                out.collect(message);
              }
            })
            .addSink(new RocketMQSink(producerProps).withBatchFlushOnCheckpoint(true))
            .name("rocketmq-sink")
            .setParallelism(2);

    try {
      env.execute("rocketmq-flink-example");
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
}
```

refer       
1.https://github.com/apache/rocketmq-flink          
