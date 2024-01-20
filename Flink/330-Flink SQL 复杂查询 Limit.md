## Flink SQL 复杂查询

 

### 建表语句
```sql
CREATE TABLE source_table_1 (
    user_id BIGINT NOT NULL,
    row_time AS cast(CURRENT_TIMESTAMP as timestamp(3)),
    WATERMARK FOR row_time AS row_time
) WITH (
  'connector' = 'datagen',
  'rows-per-second' = '10',
  'fields.user_id.min' = '1',
  'fields.user_id.max' = '10'
);


CREATE TABLE sink_table (
    user_id BIGINT
) WITH (
  'connector' = 'print'
);

``` 


### 执行 sql  

```sql
INSERT INTO sink_table
SELECT user_id
FROM source_table_1
Limit 3;
```     


