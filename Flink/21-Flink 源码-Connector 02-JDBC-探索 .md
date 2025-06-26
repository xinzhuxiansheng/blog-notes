# Flink 源码 - Connector JDBC - 探索   

>Flink version: 1.15.4，Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8           


当 source 是 MySQL 时，env.createInput() 会统一创建 `InputFormatSourceFunction`

```java
env.createInput(JdbcInputFormat.buildJdbcInputFormat()
// 省略部分代码  
```

`StreamExecutionEnvironment#createInput()` 
```java
private <OUT> DataStreamSource<OUT> createInput(
        InputFormat<OUT, ?> inputFormat, TypeInformation<OUT> typeInfo, String sourceName) {

    InputFormatSourceFunction<OUT> function =
            new InputFormatSourceFunction<>(inputFormat, typeInfo);
    return addSource(function, sourceName, typeInfo);
}
```