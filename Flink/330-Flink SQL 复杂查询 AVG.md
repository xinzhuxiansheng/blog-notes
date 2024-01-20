## Flink SQL 复杂查询

### 案例背景  
查询薪水高于公司平均薪水的员工名字。首先，我们需要从 Kafka 主题中读取数据，并将其转换为 Flink SQL 可以查询的表格式。

### 建表语句
```sql
CREATE TABLE Employees (
  name STRING,
  salary INT
) WITH (
  'connector' = 'kafka',
  'properties.bootstrap.servers' = 'dn-kafka3:9092',
  'json.ignore-parse-errors' = 'false',
  'format' = 'json',
  'topic' = 'yzhoujsontp01',
  'properties.group.id' = 'yzhougid011601',
  'scan.startup.mode' = 'latest-offset',
  'json.fail-on-missing-field' = 'false'
);
```

### 测试数据  
```json
{ "name": "John Doe", "salary": 50000 }
{ "name": "Jane Smith", "salary": 60000 }
{ "name": "Alice Johnson", "salary": 55000 }
```

### 执行 sql  
```sql
SELECT name 
FROM Employees 
WHERE salary > (SELECT AVG(salary) FROM Employees);
```