## Idea 本地调试状态恢复    

>本篇Blog 主要是通过 Idea 测试 Flink Job 状态恢复（Checkpoint机制）     

### 测试案例    

```sql
CREATE TABLE source_table (
    dim BIGINT,
    user_id BIGINT,
    price BIGINT,
    row_time AS cast(CURRENT_TIMESTAMP as timestamp(3)),
    WATERMARK FOR row_time AS row_time - INTERVAL '5' SECOND
) WITH (
  'connector' = 'datagen',
  'rows-per-second' = '1',
  'fields.dim.min' = '1',
  'fields.dim.max' = '2',
  'fields.user_id.min' = '1',
  'fields.user_id.max' = '100000',
  'fields.price.min' = '1',
  'fields.price.max' = '100000'
);
                
CREATE TABLE sink_table (
    dim BIGINT,
    pv BIGINT,
    sum_price BIGINT,
    max_price BIGINT,
    min_price BIGINT,
    uv BIGINT,
    window_start bigint
) WITH (
  'connector' = 'print'
);
                
insert into sink_table
select dim,
       sum(bucket_pv) as pv,
       sum(bucket_sum_price) as sum_price,
       max(bucket_max_price) as max_price,
       min(bucket_min_price) as min_price,
       sum(bucket_uv) as uv,
       max(window_start) as window_start
from (
     select dim,
            count(*) as bucket_pv,
            sum(price) as bucket_sum_price,
            max(price) as bucket_max_price,
            min(price) as bucket_min_price,
            count(distinct user_id) as bucket_uv,
            UNIX_TIMESTAMP(CAST(tumble_start(row_time, interval '1' DAY) AS STRING)) * 1000 as window_start
     from source_table
     group by
            mod(user_id, 1024),
            dim,
            tumble(row_time, interval '1' DAY)
)
group by dim,window_start;
```


refer   
1.https://mp.weixin.qq.com/s/rLeKY_49q8rR9C_RmlTmhg     

