## 演示 Table API 转  Retract 和 Upsert 流  

### 读取 Kafka 数据 示例
```java
/*
  以 Kafka 作为源，按时 Table API 转 Retract 和 Upsert 流
  示例数据：{"name":"zs","age":19}
 */
object KafkaSourceSinkSQL1FixToDataStream {
  def main(args: Array[String]): Unit = {
    //由于需要将Table转为DataStream，所以需要使用StreamTableEnviroment
    val env = StreamExecutionEnvironment.getExecutionEnvironment
    val tEnv = StreamTableEnvironment.create(env)

    //创建输入表
    val inTableSql = FileUtil.readFile("/Users/a/Code/Java/flink-tutorial/flink-learn/src/main/resources/upsertkafka/kafka_source.sql");
    tEnv.executeSql(inTableSql)

    //业务逻辑
    val execSql =
      """
        |SELECT
        |  age,
        |  COUNT(*) AS cnt
        |FROM kafka_source
        |GROUP BY age
        |""".stripMargin

    //执行SQL查询操作
    val resTable = tEnv.sqlQuery(execSql)

    //将结果转换为DataStream数据流-Retract  回撤流
    val resStream = tEnv.toChangelogStream(resTable,
      Schema.newBuilder().build(),
      ChangelogMode.all()
    )

    //将结果转换为DataStream数据流-Upsert   Changelog流
//    val resStream = tEnv.toChangelogStream(resTable,
//      Schema.newBuilder().primaryKey("age").build(),
//      ChangelogMode.upsert()
//    )

    //打印DataStream数据流中的数据
    resStream.print()

    //执行
    env.execute("KafkaSourceSinkSQL1FixToDataStream")
  }

}
``` 

**SQL 示例**    
```sql
CREATE TABLE kafka_source(
     name STRING,
     age INT NOT NULL
)WITH
(
 'connector' = 'kafka',
 'topic' = 'dt001',
 'properties.bootstrap.servers' = 'kafka:9092',
 'properties.group.id' = 'gid-sql-1',
 'scan.startup.mode' = 'latest-offset',
 'format' = 'json',
 'json.fail-on-missing-field' = 'false',
 'json.ignore-parse-errors' = 'true'
 )
```

### 转成 Retract 回撤流 
```java
//将结果转换为DataStream数据流-Retract  回撤流
val resStream = tEnv.toChangelogStream(resTable,
    Schema.newBuilder().build(),
    ChangelogMode.all()
)
```

output: 
```shell
# 发送第一条 {"name":"zs","age":19}
6> +I[19, 1]    

# 发送第二条 {"name":"zs","age":19}，此时会 发送2条数据，1是 -U，2是 +U
6> -U[19, 1]
6> +U[19, 2]
```

删除老数据使用的是 `-U`,添加新数据时使用的是 `+U` 

### 转成 Upsert 流  
```java
//将结果转换为DataStream数据流-Upsert   Changelog流
val resStream = tEnv.toChangelogStream(resTable,
    Schema.newBuilder().primaryKey("age").build(),
    ChangelogMode.upsert()
)
```

output: 
```shell
# 发送第一条 {"name":"zs","age":19}
6> +I[19, 1]

# 发送第二条 {"name":"zs","age":19}，此时会 发送2条数据，1是 -U，2是 +U
6> +U[19, 2]
```

