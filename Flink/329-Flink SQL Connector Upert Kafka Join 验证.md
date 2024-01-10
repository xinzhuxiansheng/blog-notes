## 演示 Flink SQL Left Join 转 DataStream API     

### 读取 Kafka 数据 双流 Join 示例    
```java
/*
  Kafka 双流 Join，转 DataStream API

  示例数据
  订单表：
  {"order_id":1001,"ts":1665367200000}
  {"order_id":1002,"ts":1665367262000}

  支付表
  {"order_id":1002,"pay_money":100}
  {"order_id":1001,"pay_money":80}
 */
def main(args: Array[String]): Unit = {
  //由于需要将Table转为DataStream，所以需要使用StreamTableEnviroment
  val env = StreamExecutionEnvironment.getExecutionEnvironment
  val tEnv = StreamTableEnvironment.create(env)

  //指定国内的时区
  tEnv.getConfig.setLocalTimeZone(ZoneId.of("Asia/Shanghai"))

  //订单表
  val UserOrderTableSql = FileUtil.readFile("/Users/a/Code/Java/flink-tutorial/flink-learn/src/main/resources/upsertkafka/user_order.sql");
  tEnv.executeSql(UserOrderTableSql)

  //支付表
  val PaymentFlowTableSql = FileUtil.readFile("/Users/a/Code/Java/flink-tutorial/flink-learn/src/main/resources/upsertkafka/payment_flow.sql")
  tEnv.executeSql(PaymentFlowTableSql)

  //关联订单表和支付表
  val joinSql =
    """
      |SELECT
      |  uo.order_id,
      |  uo.d_timestamp,
      |  pf.pay_money
      |FROM user_order AS uo
      |-- 这里使用LEFT JOIN 或者LEFT OUTER JOIN 是一样的效果
      |LEFT JOIN payment_flow AS pf ON uo.order_id = pf.order_id
      |""".stripMargin
  val resTable = tEnv.sqlQuery(joinSql)

  //将结果转换为DataStream数据流-Retract
  val resStream = tEnv.toChangelogStream(resTable,
    Schema.newBuilder().build(),
    ChangelogMode.all()
  )

  //将结果转换为DataStream数据流-Upsert
//  val resStream = tEnv.toChangelogStream(resTable,
//    Schema.newBuilder().primaryKey("order_id").build(),
//    ChangelogMode.upsert()
//  )

  //打印resStream数据流中的数据
  resStream.print()

  //执行
  env.execute("RegularJoin_LeftJoinToDataStream")
}
``` 
**user_order.sql**
```sql
CREATE TABLE user_order(
                           order_id BIGINT NOT NULL,
                           ts BIGINT,
                           d_timestamp AS TO_TIMESTAMP_LTZ(ts,3)
    -- 注意：d_timestamp的值可以从原始数据中取，原始数据中没有的话也可以从Kafka的元数据中取
    -- d_timestamp TIMESTAMP_LTZ(3) METADATA FROM 'timestamp'
)WITH(
     'connector' = 'kafka',
     'topic' = 'user_order',
     'properties.bootstrap.servers' = 'kafka:9092',
     'properties.group.id' = 'gid-sql-order',
     -- 为了便于演示，在这使用latest-offset，每次启动都使用最新的数据
     'scan.startup.mode' = 'latest-offset',
     'format' = 'json',
     'json.fail-on-missing-field' = 'false',
     'json.ignore-parse-errors' = 'true'
     )
```

**payment_flow.sql**
```sql
CREATE TABLE payment_flow(
                             order_id BIGINT  NOT NULL,
                             pay_money BIGINT
)WITH(
     'connector' = 'kafka',
     'topic' = 'payment_flow',
     'properties.bootstrap.servers' = 'kafka:9092',
     'properties.group.id' = 'gid-sql-payment',
     -- 为了便于演示，在这使用latest-offset，每次启动都使用最新的数据
     'scan.startup.mode' = 'latest-offset',
     'format' = 'json',
     'json.fail-on-missing-field' = 'false',
     'json.ignore-parse-errors' = 'true'
     )
```

### Left Join 转 Retract 流 
```java
//将结果转换为DataStream数据流-Retract
val resStream = tEnv.toChangelogStream(resTable,
  Schema.newBuilder().build(),
  ChangelogMode.all()
)
```

output: 
```shell
# 发送订单数据： {"order_id":1001,"ts":1665367200000}
2> +I[1001, 2022-10-10T02:00:00Z, null]

# 发送支付数据：{"order_id":1001,"pay_money":80}
2> -D[1001, 2022-10-10T02:00:00Z, null]
2> +I[1001, 2022-10-10T02:00:00Z, 80] 
```

Left Join在实现数据更新的时候，它是先通过 -D 回撤老数据, 再通过 +I 添加新数据, 它与 `group  by` 产生的效果不一样, group by在 回撤老数据的时候是 -U，添加新数据的时候是 +U。 

### Left Join 转 Upsert 流  
```java
//将结果转换为DataStream数据流-Upsert
val resStream = tEnv.toChangelogStream(resTable,
  Schema.newBuilder().primaryKey("order_id").build(),
  ChangelogMode.upsert()
)
```

output: 
```shell
# 发送订单数据： {"order_id":1001,"ts":1665367200000}
2> +I[1001, 2022-10-10T02:00:00Z, null]

# 发送支付数据：{"order_id":1001,"pay_money":80}
2> -D[1001, 2022-10-10T02:00:00Z, null]
2> +I[1001, 2022-10-10T02:00:00Z, 80]
```

将 Join的数据转换成 Retract 流 与 转换成 Upsert 流 效果是一样的, 没有使用 -U，+U 操作, 跟新数据时 是`先删除后新增`。  

