**`正文`**
[TOC]

## Log4j2的onMatch和onMismatch属性值讲解
>onMatch和onMismatch都有三个属性值，分别为Accept、DENY和NEUTRAL

分别介绍这两个配置项的三个属性值：

onMatch="ACCEPT" 表示匹配该级别及以上
onMatch="DENY" 表示不匹配该级别及以上
onMatch="NEUTRAL" 表示该级别及以上的，由下一个filter处理，如果当前是最后一个，则表示匹配该级别及以上
onMismatch="ACCEPT" 表示匹配该级别以下
onMismatch="NEUTRAL" 表示该级别及以下的，由下一个filter处理，如果当前是最后一个，则不匹配该级别以下的
onMismatch="DENY" 表示不匹配该级别以下的



## 关闭Kafka在log4j的 不必要的输出

```xml
<logger name="org.apache.kafka.clients.consumer.ConsumerConfig" level="off" />

```





## Log4j2使用

refer: https://logging.apache.org/log4j/2.x/articles.html


### Example Code

**添加`log4j-core`依赖**
```xml
<dependency>
    <groupId>org.apache.logging.log4j</groupId>
    <artifactId>log4j-core</artifactId>
    <version>2.18.0</version>
</dependency>
```

**Code**    
```java
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author yzhou
 * @date 2022/8/9
 */
public class ModuleA {
    private static Logger logger = LogManager.getLogger();

    public static void main(String [] args){
        logger.debug("debug");
        logger.error("error");
        logger.fatal("fatal");
        logger.info("info");
        logger.trace("trace");
        logger.warn("warn");
    }
}
```

**Output**  
```
20:12:35.333 [main] ERROR com.yzhou.log4j.ModuleA - error
20:12:35.338 [main] FATAL com.yzhou.log4j.ModuleA - fatal
```

我们Code中添加所有级别log输出，但在控制台中，我们只看到两个级别。实际上，当我们不提供任何配置文件（尚未涵盖）时，默认情况下 Log4j使用默认配置，且输出级别是`ERROR`。 
默认配置提供了配置信息: 
1.默认输入到控制台（ConsoleAppender）
2.默认PatternLayout应用在ConsoleAppender (%d{HH:mm:ss.SSS} [%t] %-5level %logger{36} – %msg%n)


log输入级别遵循以下：
ALL < TRACE < DEBUG < INFO < WARN < ERROR < FATAL   



### 添加log4j2 config

refer： https://logging.apache.org/log4j/2.x/manual/configuration.html 