## Flink Doris Connector 支持删除操作   

### 背景    
在`Kafka 2 Doris`数据同步场景下，Kafka TopicA 的数据为以下内容:    
```json 
{
    "TRANS_DATE": 1,
    "PRD_CODE": "1",
    "PRD_NAME": "产品名称",
    "TOTAL_AMOUNT": 100,
    "TURNOVER_AMOUNT": 100,
    "op_type": "I"
}
``` 
将 Kafka Topic 数据写入到 Doris时，根据`op_type`做相应的增删改操作。例如：  
* 当 op_type : I 时，则表示插入   
* 当 op_type : U 时，则表示更新，注意因没有before字段，所以是全字段更新（覆盖）   
* 当 op_type : D 时，则表示删除     

此时将 TopicA 中的 `PRD_CODE` 定义为 Doris 的 `Unique Key`。 此处请参考`https://doris.apache.org/zh-CN/docs/ecosystem/flink-doris-connector`      

### 环境准备    

#### 前提准备   
1.启动 standalone Flink Seesion 集群
2.搭建测试 Kafka && 创建 kafka topic `yzhoutp01`  
请参考`https://kafka.apache.org/quickstart` 单节点 测试搭建。  
**yzhoutp01 topic创建**         
```shell
./kafka-topics.sh --create --bootstrap-server xxx.xxx.xxx.xxx:9292 --replication-factor 1 --partitions 1 --topic yzhoutp01      
```

3.搭建测试 Doris && 创建 doris table `yzhou_test01` 
请参考`https://doris.apache.org/zh-CN/docs/install/construct-docker/run-docker-cluster` 搭建 1fe1be 测试集群。    

**yzhou_test01 建表语句**           
```sql
CREATE TABLE yzhou_test01
(
    PRD_CODE               varchar(1000) null,
    TRANS_DATE             varchar(1000) null,
    TOTAL_AMOUNT           varchar(1000) null,
    TURNOVER_AMOUNT        varchar(1000) null,
    op_type             varchar(1000) null
)
 ENGINE=OLAP
 UNIQUE KEY(`PRD_CODE`)
 COMMENT "OLAP"
 DISTRIBUTED BY HASH(`PRD_CODE`) BUCKETS 1
 PROPERTIES (
    "replication_num" = "1",
    "in_memory" = "false",
     "storage_format" = "V2"
 );
```

### Flink SQL Cli 提交 Kafka 2 Doris Job    
**kafka source create table sql**
```
CREATE TABLE `kafka_source` (
    `TRANS_DATE` STRING COMMENT '',
    `PRD_CODE` STRING COMMENT '',
    `TOTAL_AMOUNT` STRING COMMENT '',
    `TURNOVER_AMOUNT` STRING COMMENT '',
    `op_type` STRING COMMENT ''
)
WITH
(
    'connector' = 'kafka',
    'properties.bootstrap.servers' = '192.168.0.201:9092',
    'format' = 'json',
    'topic' = 'yzhoutp01',
    'properties.group.id' = 'yzhougid01',
    'scan.startup.mode' = 'earliest-offset'
);
```

**doris sink create table sql**
```
CREATE TABLE `doris_sink` (
     `TRANS_DATE` STRING COMMENT '',
     `PRD_CODE` STRING NOT NULL COMMENT '',
     `TOTAL_AMOUNT` STRING COMMENT '',
     `TURNOVER_AMOUNT` STRING COMMENT '',
     `op_type` STRING COMMENT '',
     PRIMARY KEY (PRD_CODE) NOT ENFORCED
)WITH (
    'connector' = 'doris',
    'fenodes' = '192.168.0.201:8030',
    'table.identifier' = 'yzhou_test.yzhou_test01',
    'username' = 'root',
    'password' = '',
    'sink.max-retries' = '3',
    'sink.properties.format' = 'json',
    'sink.enable-delete'='true',
    'sink.label-prefix' = 'doris_label_yzhou_103',
    'sink.properties.format' = 'json', -- 别漏掉该参数
    'sink.properties.read_json_by_line' = 'true' -- 别漏掉该参数
);
```

当通过 Flink Sql Cli 执行 `INSERT INTO doris_sink SELECT * FROM kafka_source;`, 只会对数据进行插入、修改，并`不会删除`。 

>需特别注意，务必开启 Checkpoint。          

### 改造计划 

#### 摸索
根据 Doris 的官网介绍 `https://doris.apache.org/zh-CN/docs/ecosystem/flink-doris-connector`，"目前的删除是支持 Flink CDC 的方式接入数据实现自动删除"，所以我拿 `MySQL CDC 2 Doris` Flink SQL Job 调试`Flink Doris Connector`(https://github.com/apache/doris-flink-connector)。 

>若对Flink 自定义 Connector不够了解的同学，可先通过该篇 Blog "Flink用户自定义连接器（Table API Connectors）学习总结"（https://www.modb.pro/db/634537） 了解如何自定义Connector，等有了足够的了解，doris connector 调试起来就会显得得心应手。        

根据 Doris Connector 源码的 `RowDataSerializer#serialize()`方法，首先会解析数据，通过 buildJsonString()方法，将 `RowData` -> Map<String,String> valueMap -> String -> `DorisRecord`。   

```java
public String buildJsonString(RowData record, int maxIndex) throws IOException {
    int fieldIndex = 0;
    Map<String, String> valueMap = new HashMap<>();
    while (fieldIndex < maxIndex) {
        Object field = rowConverter.convertExternal(record, fieldIndex);
        String value = field != null ? field.toString() : null;
        valueMap.put(fieldNames[fieldIndex], value);
        fieldIndex++;
    }
    if (enableDelete) {
        valueMap.put(DORIS_DELETE_SIGN, parseDeleteSign(record.getRowKind()));
    }
    return objectMapper.writeValueAsString(valueMap);
}
```

若 enableDelete = true，会执行RowKind()获取对应的 CDC op值，当等于 DELETE 时，会在 valueMap放入一个key为`__DORIS_DELETE_SIGN__`,value = 1。 

>所以这一切都来的太突然了 ...

#### 验证
基于 `__DORIS_DELETE_SIGN__` = 1，就代表删除的策略 且 采用了 Doris Sink 配置项中的扩展参数`sink.properties.*` , 定义了2个参数，如下所示：    
* sink.properties.customdelete_name: 表示 自定义删除列
* sink.properties.customdelete_value: 表示 当删除列等于某值时标记删除 

所以，`DorisDynamicTableSink#getSinkRuntimeProvider()` 获取参数并将参数传入 `RowDataSerializer`对象 
```java
@Override
public SinkRuntimeProvider getSinkRuntimeProvider(Context context) {
    Properties loadProperties = executionOptions.getStreamLoadProp();
    boolean deletable = executionOptions.getDeletable() && RestService.isUniqueKeyType(options, readOptions, LOG);
    if (!loadProperties.containsKey(COLUMNS_KEY)) {
        String[] fieldNames = tableSchema.getFieldNames();
        Preconditions.checkState(fieldNames != null && fieldNames.length > 0);
        String columns = String.join(",", Arrays.stream(fieldNames).map(item -> String.format("`%s`", item.trim().replace("`", ""))).collect(Collectors.toList()));
        if (deletable) {
            columns = String.format("%s,%s", columns, DORIS_DELETE_SIGN);
        }
        loadProperties.put(COLUMNS_KEY, columns);
    }

    RowDataSerializer.Builder serializerBuilder = RowDataSerializer.builder();
    serializerBuilder.setFieldNames(tableSchema.getFieldNames())
            .setFieldType(tableSchema.getFieldDataTypes())
            .setType(loadProperties.getProperty(FORMAT_KEY, CSV))
            .enableDelete(deletable)
            .setCustomDeleteName(loadProperties.getProperty("customdelete_name"))
            .setCustomDeleteValue(loadProperties.getProperty("customdelete_value"))
            .setFieldDelimiter(loadProperties.getProperty(FIELD_DELIMITER_KEY, FIELD_DELIMITER_DEFAULT));
    DorisSink.Builder<RowData> dorisSinkBuilder = DorisSink.builder();
    dorisSinkBuilder.setDorisOptions(options)
            .setDorisReadOptions(readOptions)
            .setDorisExecutionOptions(executionOptions)
            .setSerializer(serializerBuilder.build());
    return SinkProvider.of(dorisSinkBuilder.build(), sinkParallelism);
}
```

接下来还需在 RowDataSerializer 中增加以下两个字段和 Builder的设值方法。      
```
private String customDeleteName;
private String customDeleteValue;       
```


最后，在 `RowDataSerializer#buildJsonString()` 增加 customDeleteName和 customDeleteValue的校验 即可。 

```java
public String buildJsonString(RowData record, int maxIndex) throws IOException {
    int fieldIndex = 0;
    Map<String, String> valueMap = new HashMap<>();
    while (fieldIndex < maxIndex) {
        Object field = rowConverter.convertExternal(record, fieldIndex);
        String value = field != null ? field.toString() : null;
        valueMap.put(fieldNames[fieldIndex], value);
        fieldIndex++;
    }
    if (enableDelete) {
        valueMap.put(DORIS_DELETE_SIGN, parseDeleteSign(record.getRowKind()));
    }
    // 根据自定义来标记 删除
    if(!StringUtils.isNullOrWhitespaceOnly(customDeleteName) && valueMap.containsKey(customDeleteName)){
        if(valueMap.get(customDeleteName).equals(customDeleteValue)){
            valueMap.put(DORIS_DELETE_SIGN, "1"); // 标记删除
        }
    }
    return objectMapper.writeValueAsString(valueMap);
}
```

>注意，上面代码修改，并没有一一讲述，有思路，心不慌 ...

到这里， Kafka 2 Doris 支持删除操作的改造就已经完成了，剩下的就是编译打包，以及放入 flink lib/中，重新启动 Standalone Flink。           
