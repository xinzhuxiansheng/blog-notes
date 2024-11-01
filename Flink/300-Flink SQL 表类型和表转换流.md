
##
* Static Table  

* Dynamic Table 
    - Versioned Table(Temporal Table)   
    - Temporal Table Function   


## Static Table 
静态表，表中的数据不随着时间实时变化,例如，在 BatchMode 模式下使用 filesystem 定义的表。注意： 静态表的数据也可能会发生变化，只不过不会随着时间实时变化。    



## Dynamic Table  

### 版本表    
* 通过 PRIMARY KEY 定义主键    
* 通过 Watermark 来定义事件时间字段，但前提是需要在表中先定义一个普通的时间字段      

建表语句：    
```bash
CREATE TABLE products (
    update_time TIMESTAMP(3),
    product_id STRING,
    product_name STRING,
    price DECIMAL(32,2),
    -- 定义主键
    PRIMARY KEY(product_id) NOT ENFORCED,
    WATERMARK FOR update_time AS update_time
)
WITH (...)
```

### 时态表    
* 时态表函数支持访问动态表（仅追加）中指定时间点的数据版本      
* 与版本表不同，时态表函数只能在仅追加表上定义      
* 时态表函数不能通过 SQL DDL 定义，需要通过 Table API 注册   

## 动态表 转换 流     
动态表转换为输出表：     
* Append-only 流，追加流 (新增) INSERT +I   
* Retract 流，回撤流 (新增，更新，删除) INSERT +I, UPDATE: UPDATE_BEFORE -U, UPDATE_AFTER +U, DELETE -D    
* Upsert 流，插入流 (新增，更新，删除)  INSERT +I, UPDATE: UPDATE_AFTER +U, DELETE -D        

```bash
INSERT +I
UPDATE: UPDATE_BEFORE -U, UPDATE_AFTER +U    
DELETE -D    
``` 

### 动态表 转 Append-Only 流  
```java
public class AppendOnlySQL {
    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        StreamTableEnvironment tEnv = StreamTableEnvironment.create(env);
        env.setParallelism(1);

        /*
        CREATE TABLE orders(
            order_id    BIGINT,
            price       DECIMAL(10,2),
            order_time  TIMESTAMP
        ) WITH (
            'connector' = 'datagen',
            'rows-per-second' = '1'
        )
         */
        String inTableSql = "CREATE TABLE orders(\n" +
                "    order_id    BIGINT,\n" +
                "    price       DECIMAL(10,2),\n" +
                "    order_time  TIMESTAMP\n" +
                ") WITH (\n" +
                "    'connector' = 'datagen',\n" +
                "    'rows-per-second' = '1'\n" +
                ")";
        tEnv.executeSql(inTableSql);

        Table resTable = tEnv.sqlQuery("SELECT * FROM orders");
        DataStream<Row> resStream = tEnv.toChangelogStream(resTable,
                Schema.newBuilder().build(),
                ChangelogMode.insertOnly());

        resStream.print();
        env.execute("AppendOnlySQL");
    }
}
```  

Output:  (注意，此时时间缺少东八区时间定义)              
```bash
+I[-583137642846923259, 88621255.86, 2024-10-31T07:07:12.581]
+I[-6278723994698920731, 86888662.13, 2024-10-31T07:07:13.580]
+I[-888239117413540249, 21801583.99, 2024-10-31T07:07:14.590]
+I[-4771107177037268490, 63971978.43, 2024-10-31T07:07:15.595]
+I[945202507492865167, 32654136.76, 2024-10-31T07:07:16.580]
+I[592302051806283212, 96217380.84, 2024-10-31T07:07:17.584]
+I[6968183462417157051, 59042776.55, 2024-10-31T07:07:18.580]
+I[3390975286494898013, 27969745.28, 2024-10-31T07:07:19.590]
+I[725517364192562324, 80806505.73, 2024-10-31T07:07:20.581]
......   
```


### 动态流 转 Retract 流  
```java
public class RetractSQL {
    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        StreamTableEnvironment tEnv = StreamTableEnvironment.create(env);
        env.setParallelism(1);

        String inTableSql = "CREATE TABLE orders(\n" +
                "    order_id    BIGINT,\n" +
                "    price       DECIMAL(10,2),\n" +
                "    order_time  TIMESTAMP\n" +
                ") WITH (\n" +
                "    'connector' = 'datagen',\n" +
                "    'rows-per-second' = '1',\n" +
                "    'fields.order_id.min' = '100',\n" +
                "    'fields.order_id.max' = '105'\n" +
                ")";
        tEnv.executeSql(inTableSql);

        Table resTable = tEnv.sqlQuery("SELECT order_id,COUNT(*) AS cnt FROM orders GROUP BY order_id");
        DataStream<Row> resStream = tEnv.toChangelogStream(resTable,
                Schema.newBuilder().build(),
                ChangelogMode.all());

//        private static final ChangelogMode ALL =
//                ChangelogMode.newBuilder()
//                        .addContainedKind(RowKind.INSERT)
//                        .addContainedKind(RowKind.UPDATE_BEFORE)
//                        .addContainedKind(RowKind.UPDATE_AFTER)
//                        .addContainedKind(RowKind.DELETE)
//                        .build();

        resStream.print();
        env.execute("RetractSQL");
    }
}
```

Output: 
```bash
+I[102, 1]
+I[103, 1]
+I[105, 1]
+I[101, 1]
-U[101, 1]
+U[101, 2]
+I[104, 1]
-U[102, 1]
+U[102, 2]
-U[105, 1]
+U[105, 2]
-U[105, 2]
+U[105, 3]
-U[105, 3]
+U[105, 4]
+I[100, 1]
```

### 动态表 转 Upsert 流   
必须有主键（唯一键）  

```java
public class UpsertSQL {
    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        StreamTableEnvironment tEnv = StreamTableEnvironment.create(env);
        env.setParallelism(1);

        String inTableSql = "CREATE TABLE orders(\n" +
                "    order_id    BIGINT NOT NULL,\n" +
                "    price       DECIMAL(10,2),\n" +
                "    order_time  TIMESTAMP\n" +
                ") WITH (\n" +
                "    'connector' = 'datagen',\n" +
                "    'rows-per-second' = '1',\n" +
                "    'fields.order_id.min' = '100',\n" +
                "    'fields.order_id.max' = '105'\n" +
                ")";
        tEnv.executeSql(inTableSql);

        Table resTable = tEnv.sqlQuery("SELECT order_id,COUNT(*) AS cnt FROM orders GROUP BY order_id");
        DataStream<Row> resStream = tEnv.toChangelogStream(resTable,
                Schema.newBuilder().primaryKey("order_id").build(),   // 必须指定主键
                ChangelogMode.upsert());

        resStream.print();
        env.execute("UpsertSQL");
    }
}
```

Output:     
```bash
+I[102, 1]
+I[105, 1]
+U[102, 2]
+U[102, 3]
+I[104, 1]
+I[103, 1]
+U[104, 2]
+U[104, 3]
+U[105, 2]
+U[102, 4]
+U[103, 2]
+U[102, 5]
+U[104, 4]
```

refer  
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/table/concepts/overview/     