## Flink SQL 复杂查询 (该案例没有测试成功)

### 案例背景  
假设你有一个实时事件流，比如网站的点击流数据，你想计算每5分钟内每个页面的点击次数。

### 建表语句
```sql

CREATE TABLE ClickStream (
  page_id STRING,
  user_id STRING,
  event_time TIMESTAMP(3),
  WATERMARK FOR event_time AS event_time - INTERVAL '5' SECOND
) WITH (
  'connector' = 'kafka',
  'properties.bootstrap.servers' = 'dn-kafka3:9092',
  'json.ignore-parse-errors' = 'false',
  'format' = 'json',
  'topic' = 'yzhoujsontp01',
  'properties.group.id' = 'yzhougid011603',
  'scan.startup.mode' = 'latest-offset',
  'json.fail-on-missing-field' = 'false'
);
``` 


### 测试数据  
```json
{ "page_id": "home", "user_id": "u123", "timestamp": "2024-01-16T12:00:00Z" }
{ "page_id": "contact", "user_id": "u124", "timestamp": "2024-01-16T12:01:00Z" }
```

### 执行 sql  

```sql
SELECT 
  page_id, 
  TUMBLE_START(event_time, INTERVAL '1' MINUTE) as window_start,
  COUNT(*) as click_count
FROM ClickStream
GROUP BY page_id, TUMBLE(event_time, INTERVAL '1' MINUTE);
```    
