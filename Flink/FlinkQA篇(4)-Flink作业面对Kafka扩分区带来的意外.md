--In Blog
--Tags: Flink

# Flink作业面对Kafka Topic扩分区带来的意外

## 意外背景
1. 有个每秒300M字节写入的Kafka Topic （Topic名称：TPA）
2. 有一个自定义Jar的Flink作业（没有开启checkpoint）在消费 TPA
3. 开发人员发现Flink消费延迟增加，需要增加TPA分区数
4. Kafka管理员增加TPA分区数后，xxx发现新增加的分区 只有写入，而Flink作业没有在消费

>TPA增加分区后，Flink作业没有消费到"新增加"的分区？？


## 思考
### 检查配置项是否配置

xxx检查了一遍 FlinkKafkaConsumer的构造方法，有没有添加"flink.partition-discovery.interval-millis"参数项。
```java
public static FlinkKafkaConsumer<ConsumerRecord<byte[], byte[]>> createConsumer(String topicName, String bootstrapServers, String token, String groupId, Map<String, Object> pro) {
    Properties properties = getProperties(bootstrapServers, token);
    properties.put("group.id", groupId);
    properties.put("flink.partition-discovery.interval-millis", 60000);
    if (pro != null) {
        properties.putAll(pro);
    }
    return new FlinkKafkaConsumer<ConsumerRecord<byte[], byte[]>>(topicName, new SourceKafkaDeserializationSchema(), properties);
}
```

发现是配置了呀。 `properties.put("flink.partition-discovery.interval-millis", 60000);`


### 检查参数值是否生效

Flink作业开发挺大的好处就是"本地debug"不费事。

FlinkKafkaConsumer.java
```java
private FlinkKafkaConsumer(
	List<String> topics,
	Pattern subscriptionPattern,
	KafkaDeserializationSchema<T> deserializer,
	Properties props) {

	//发现 getLong(...) 方法返回的是 null, 这逻辑不合理呀，命名配置 60000
	super(
		topics,
		subscriptionPattern,
		deserializer,
		getLong(
			checkNotNull(props, "props"),
			KEY_PARTITION_DISCOVERY_INTERVAL_MILLIS, PARTITION_DISCOVERY_DISABLED),
		!getBoolean(props, KEY_DISABLE_METRICS, false));

	props = KafkaSdkSupport.addKafkaSdkSpeicalizedProperty(props);

	this.properties = props;
	setDeserializer(this.properties);

	// configure the polling timeout
	try {
		if (properties.containsKey(KEY_POLL_TIMEOUT)) {
			this.pollTimeout = Long.parseLong(properties.getProperty(KEY_POLL_TIMEOUT));
		} else {
			this.pollTimeout = DEFAULT_POLL_TIMEOUT;
		}
	}
	catch (Exception e) {
		throw new IllegalArgumentException("Cannot parse poll timeout for '" + KEY_POLL_TIMEOUT + '\'', e);
	}
}
```

### debug  getLong(checkNotNull(props, "props"),KEY_PARTITION_DISCOVERY_INTERVAL_MILLIS, PARTITION_DISCOVERY_DISABLED)方法

```java
public static long getLong(Properties config, String key, long defaultValue) {
    //config.getProperty(key) 返回为 null
    String val = config.getProperty(key);
    if (val == null) {
        return defaultValue;
    } else {
        try {
            return Long.parseLong(val);
        } catch (NumberFormatException var6) {
            throw new IllegalArgumentException("Value for configuration key='" + key + "' is not set correctly. Entered value='" + val + "'. Default value='" + defaultValue + "'");
        }
    }
}
```

debug发现: 
* `flink.partition-discovery.interval-millis`=数值型，返回为null
```java 
properties.put("flink.partition-discovery.interval-millis", 60000);
String val = config.getProperty(key);
val = null
```

* `flink.partition-discovery.interval-millis`=字符串，返回为60000
```java 
properties.put("flink.partition-discovery.interval-millis", "60000");
String val = config.getProperty(key);
val = 60000
```

### debug config.getProperty(key)
getProperty()方法只会对字符串类型的value值，取值
```java
public String getProperty(String key) {
    Object oval = super.get(key);
    String sval = (oval instanceof String) ? (String)oval : null;
    return ((sval == null) && (defaults != null)) ? defaults.getProperty(key) : sval;
}
```


