## Flink SQL 维表 Join 案例实战 

### 案例一：直播平台开播记录 （关联MySQL中的国家 -> 大区映射关系）  
直播平台中的主播开播之后, 会产生一条开播记录数据，对应的会记录一条日志数据, 我们会通过日志采集工具实时采集这份数据。 主播的开播记录中有一个字段是country, 代表的是主播所在的国家，但是后续统计数据指标的时侯, 业务部门一般会以大区这个维度进行统计，因为我们的直播平台是在很多个国家运营, 运营部门为了便于管理将这些国家划分到了一些大区里面, 后期下发运营策略的，同一个大区里面的国家使用相同的运营策略，国家和大区之间的关系是多对一, 那为什么当时不直接存储大区信息呢 ?      

因为主播所在的国家是固定的, 但是这个国家对应的大区是运营人员人为划分的 后期可能还会调整,那就是说国家和大区的关系可能会发生改变, 但是也不会频繁变化，可能隔几个月会调整一次, 国家和大区的映射关系存储在MySOL数据库中, 这份映射关系运营人员可以在控制平台进行修改，那我们在实时处理开播记录数据的时候, 如果想要关联最新的国家和大区的映射关系， 就需要使用维表Join来实现了。      

1.下面我们提前在本地MySQL中去初始化国家和大区的映射关系(sql语句)。        

```sql  
// 建表 table
create table country_area
(
    country varchar(100) null,
    area    varchar(100) null
);

// 初始化 sql
INSERT INTO 'country_area' VALUES ('US', 'A US');
INSERT INTO 'country_area' VALUES ('PR', 'A AR');
INSERT INTO 'country_area' VALUES ('KW', 'A AR');
INSERT INTO 'country_area' VALUES ('SA', 'A AR');
INSERT INTO 'country_area' VALUES ('IN', 'A IN');   
```

2.添加依赖
```xml
<dependency>
    <groupId>com.ververica</groupId>
    <artifactId>flink-connector-mysql-cdc</artifactId>
    <version>2.3.0</version>
</dependency>
<dependency>
    <groupId>mysql</groupId>
    <artifactId>mysql-connector-java</artifactId>
    <version>8.0.33</version>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-jdbc_${scala.binary.version}</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
```


3.代码编写


>注意: 针对MySQL 维表中的jdbc url参数，针对8.x MySQL一定要添加该参数 `serverTimezone=Asia/Shanghai`         

```
'url' = 'jdbc:mysql://localhost:3306/yzhou_test?serverTimezone=Asia/Shanghai', -- mysql8.x中需要指定时区
```

>注意: 针对MySQL 维表参数中 `lookup.cache.max-rows`,`lookup.cache.ttl`,`lookup.max-retries` 这三个参数在实际作业开发中，是一定要加上的，尽可能优化。        

```
'lookup.cache.max-rows' = '100', -- 控制lookup缓存中最多存储的数据条数
'lookup.cache.ttl' = '3600000', -- 控制lookup缓存中数据的生命周期(毫秒)，太大或者太小都不合适
'lookup.max-retries' = '1' -- 查询数据库失败后重试的次数
```

