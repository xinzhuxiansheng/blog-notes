# Flink SQL Connectors 读写 Hbase 入门实践        

>Flink version: 1.17.2, Hbase version: 2.3.7     

## 准备数据   
```shell 
# 启动 hbase 终端   
./hbase shell  

# 创建 namespace
create_namespace 'yzhou'

# 创建一个表 person，并包含一个列族 info
create 'yzhou:person', 'info'

# 插入数据
put 'yzhou:person', 'row1', 'info:id', '1'
put 'yzhou:person', 'row1', 'info:name', 'Alice'
put 'yzhou:person', 'row1', 'info:address', '123 Main St'

# 查询命名空间中的数据
get 'yzhou:person', 'row1'

#查看命名空间中的表
list_namespace_tables 'yzhou'   
```

## 添加 Hbase 相关依赖（其他依赖不过多介绍）    
在 `lib/`目录下添加 `flink-sql-connector-hbase-2.2-1.17.2.jar`, 或者添加 
```
flink-connector-hbase-2.2-1.17.2.jar
flink-connector-hbase-base-1.17.2.jar
hbase-shaded-client-2.2.7.jar
```

## Flink SQL Cli 读 Hbase  
```bash  
CREATE TABLE hbase_person (
  rowkey STRING,
  info ROW<id STRING, name STRING, address STRING>,
  PRIMARY KEY (rowkey) NOT ENFORCED
) WITH (
  'connector' = 'hbase-2.2',
  'table-name' = 'yzhou:person',
  'zookeeper.quorum' = 'bigdata01:2181',
  'zookeeper.znode.parent' = '/hbase'
);
```

执行 `select * from hbase_person;` 得到以下结果：    
![hbase01.png](http://img.xinzhuxiansheng.com/blogimgs/flink/hbase01.png)      


## Kafka join Hbase      

### Kafka Source 
```bash 
CREATE TABLE kafka_source (
  id STRING,
  event_time STRING,
  event_type STRING,
  amount DECIMAL(10, 2),
  currency STRING,
  proctime AS PROCTIME()
) WITH (
  'connector' = 'kafka',
  'topic' = 'yzhoujsontp01',
  'properties.bootstrap.servers' = '192.168.0.201:9092',
  'properties.group.id' = 'gid111801',
  'format' = 'json',
  'scan.startup.mode' = 'latest-offset'
);
```   

kafka 示例数据： 
```json
{"id":"row1","event_time":"2024-10-30T12:00:00.000Z","event_type":"purchase","amount":99.99,"currency":"USD"}      
```

### Hbase lookup  
```bash 
CREATE TABLE hbase_lookup (
  rowkey STRING,
  info ROW<id STRING, name STRING, address STRING>,
  PRIMARY KEY (rowkey) NOT ENFORCED
) WITH (
  'connector' = 'hbase-2.2',
  'table-name' = 'yzhou:person',
  'zookeeper.quorum' = 'bigdata01:2181',
  'zookeeper.znode.parent' = '/hbase'
);
```
 
### 执行语句  
```bash
SELECT
    k.id,
    k.event_time,
    h.info.name AS person_name,
    h.info.address AS person_address
FROM
    kafka_source AS k
JOIN
    hbase_lookup FOR SYSTEM_TIME AS OF k.proctime AS h
ON
    k.id = h.rowkey;
```

结果如下：   
![hbase02](http://img.xinzhuxiansheng.com/blogimgs/flink/hbase02.png)   

## Kafka 2 Hbase    

### Kafka Source 
```bash 
CREATE TABLE kafka_source (
  rowkey STRING,
  info ROW<id STRING, name STRING, address STRING>
) WITH (
  'connector' = 'kafka',
  'topic' = 'yzhoujsontp02',
  'properties.bootstrap.servers' = '192.168.0.201:9092',
  'properties.group.id' = 'gid111801',
  'format' = 'json',
  'scan.startup.mode' = 'latest-offset'
);
```

Kafka 测试数据：    
```json
{"rowkey": "row2", "info": {"id": "2", "name": "Bob", "address": "456 High St"}}
```

### Hbase Sink     
```bash
CREATE TABLE hbase_sink (
  rowkey STRING,
  info ROW<id STRING, name STRING, address STRING>,
  PRIMARY KEY (rowkey) NOT ENFORCED
) WITH (
  'connector' = 'hbase-2.2',
  'table-name' = 'yzhou:person',
  'zookeeper.quorum' = 'bigdata01:2181',
  'zookeeper.znode.parent' = '/hbase'
);
```   

### 执行语句  
```bash
INSERT INTO hbase_sink
SELECT * FROM kafka_source; 
```

Output log:   
```bash 
hbase(main):005:0* get 'yzhou:person', 'row2'
COLUMN                                     CELL
 info:address                              timestamp=1731901590792, value=456 High St
 info:id                                   timestamp=1731901590792, value=2
 info:name                                 timestamp=1731901590792, value=Bob
1 row(s)
Took 0.1890 seconds
hbase(main):006:0>
```

## Hbase With Kerberos  

### Flink Create Table SQL 
```bash
CREATE TABLE hbase_lookup (
  rowkey STRING,
  info ROW<id STRING, name STRING, address STRING>,
  PRIMARY KEY (rowkey) NOT ENFORCED
) WITH (
  'connector' = 'hbase-2.2',
  'table-name' = 'yzhou:person',
  'zookeeper.quorum' = 'bidata01:2181',
  'zookeeper.znode.parent' = '/hbase',
  'properties.hbase.client.authentication.type' = 'kerberos',
  'properties.hbase.client.keytab.file' = '/root/hdfsconfig/hbase.keytab',
  'properties.hbase.client.principal' = 'hbase/bigdata01@CDH5.COM'
);
```   

### 多个 keytab 指定    
修改`flink-conf.yml`，通过`security.kerberos.login.contexts` 参数指定组件使用的 kerberos 信息，下面提供一个示例：   

```bash
classloader.resolve-order: parent-first
#fs.hdfs.hadoopconf: /root
state.checkpoints.num-retained: 3
state.checkpoints.dir: hdfs:///user/flink/drcdockercompose
execution.checkpointing.interval: 60000
execution.checkpointing.externalized-checkpoint-retention: RETAIN_ON_CANCELLATION
state.savepoints.dir: hdfs:///user/flink/drcdockercompose
state.backend: filesystem
state.backend.incremental: false
security.kerberos.login.use-ticket-cache: false
env.hadoop.conf.dir: /root/hdfsconfig
security.kerberos.access.hadoopFileSystems: hdfs://bigdata01:8020
 
security.kerberos.login.keytab: /root/hdfsconfig/hdfs.keytab
security.kerberos.login.principal: hdfs/bigdata01@CDH5.COM
security.kerberos.login.contexts: HDFS
 
security.kerberos.login.keytab: /root/hdfsconfig/hbase.keytab
security.kerberos.login.principal: hbase/bigdata01@CDH5.COM
security.kerberos.login.contexts: Client
 
env.java.opts.jobmanager: -Djava.security.krb5.conf=/root/hdfsconfig/krb5.conf
env.java.opts.taskmanager: -Djava.security.krb5.conf=/root/hdfsconfig/krb5.conf
```