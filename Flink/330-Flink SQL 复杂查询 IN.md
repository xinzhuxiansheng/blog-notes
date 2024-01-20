## Flink SQL 复杂查询

### 案例背景  
假设有两个 Kafka 主题：一个用于存储产品信息，另一个用于记录销售数据。我们的目标是找出在特定地区（例如，北美）有销售记录的产品。 

### 建表语句
```sql
-- 产品数据
CREATE TABLE Products (
  id INT,
  name STRING
) WITH (
  'connector' = 'kafka',
  'properties.bootstrap.servers' = 'dn-kafka3:9092',
  'json.ignore-parse-errors' = 'false',
  'format' = 'json',
  'topic' = 'yzhoujsontp01',
  'properties.group.id' = 'yzhougid011602',
  'scan.startup.mode' = 'latest-offset',
  'json.fail-on-missing-field' = 'false'
);



CREATE TABLE Sales (
  product_id INT,
  amount INT,
  region STRING
) WITH (
  'connector' = 'kafka',
  'properties.bootstrap.servers' = 'dn-kafka3:9092',
  'json.ignore-parse-errors' = 'false',
  'format' = 'json',
  'topic' = 'yzhoujsontp02',
  'properties.group.id' = 'yzhougid011602',
  'scan.startup.mode' = 'latest-offset',
  'json.fail-on-missing-field' = 'false'
);
``` 




### 测试数据  
```json
//产品信息  
{ "id": 1, "name": "Laptop" }
{ "id": 2, "name": "Smartphone" }
{ "id": 3, "name": "Tablet" }


// 销售数据
{ "product_id": 2, "amount": 300, "region": "North America" }
{ "product_id": 1, "amount": 1000, "region": "Europe" }
{ "product_id": 3, "amount": 500, "region": "North America" }


```

### 执行 sql  
我们执行一个 IN 子查询来查找在北美地区有销售记录的产品。
```sql
SELECT name 
FROM Products 
WHERE id IN (SELECT product_id FROM Sales WHERE region = 'North America');
```

这个查询首先在 Sales 表中找出所有在北美地区有销售记录的产品 ID，然后使用这些 ID 在 Products 表中查询对应的产品名称。    

