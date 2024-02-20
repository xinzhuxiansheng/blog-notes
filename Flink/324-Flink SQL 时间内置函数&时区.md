## Flink SQL 时间内置函数 & 时区       

>Flink version: 1.15.4 

### 引言 
本篇Blog 内容旨在帮助大家了解到以下两个场景的问题：    

* 在 1.13 之前，DDL create table 中使用 PROCTIME() 指定处理时间列时，返回值类型为 TIMESTAMP(3) 类型，而 TIMESTAMP(3) 是不带任何时区信息的，默认为 UTC 时间(0 时区)。    
* 使用 StreamTableEnvironment::createTemporaryView 将 DataStream 转为 Table 时，注册处理时间(proctime.proctime)、事件时间列(rowtime.rowtime)时，两列时间类型也为 TIMESTAMP(3) 类型，不带时区信息。        

而以上两个场景就会导致：  

* 在北京时区的用户使用 TIMESTAMP(3) 类型的时间列开最常用的 1 天的窗口时，划分出来的窗口范围是北京时间的 [2022-01-01 08:00:00, 2022-01-02 08:00:00]，而不是北京时间的 [2022-01-01 00:00:00, 2022-01-02 00:00:00]。因为 TIMESTAMP(3) 是默认的 UTC 时间，即 0 时区。           
* 北京时区的用户将 TIMESTAMP(3) 类型时间属性列转为 STRING 类型的数据展示时，也是 UTC 时区的，而不是北京时间的。             

因此充分了解本节的知识内容可以很好的帮你避免时区问题错误。    


### SQL 时间类型   
* Flink SQL 支持 TIMESTAMP(不带时区信息的时间)、TIMESTAMP_LTZ(带时区信息的时间)       

* TIMESTAMP(不带时区信息的时间)：是通过一个 年， 月， 日， 小时， 分钟， 秒 和 小数秒 的字符串来指定。举例：1970-01-01 00:00:04.001。    

* 为什么要使用字符串来指定呢?因为此种类型不带时区信息，所以直接用一个字符串指定就好了？    

* 那 TIMESTAMP 字符串的时间代表的是什么时区的时间呢?UTC 时区，也就是默认 0 时区，对应中国北京是东八区。       
    
* TIMESTAMP_LTZ(带时区信息的时间)：没有字符串来指定，而是通过 java 标准 epoch 时间 1970-01-01T00:00:00Z 开始计算的毫秒数。举例：1640966400000。         

* 其时区信息是怎么指定的呢?是通过本次任务中的时区配置参数 table.local-time-zone 设置的。        

* 时间戳本身也不带有时区信息，为什么要使用时间戳来指定呢?就是因为时间戳不带有时区信息，所以我们通过配置 table.local-time-zone 时区参数之后，就能将一个不带有时区信息的时间戳转换为带有时区信息的字符串了。举例：table.local-time-zone 为 Asia/Shanghai 时，4001 时间戳转化为字符串的效果是 1970-01-01 08:00:04.001。                


### 时区参数生效的 SQL 时间函数 
以下 SQL 中的时间函数都会受到时区参数的影响，从而做到最后显示给用户的时间、窗口的划分都按照用户设置时区之内的时间。             
* LOCALTIME；           
* LOCALTIMESTAMP；      
* CURRENT_DATE；          
* CURRENT_TIME；            
* CURRENT_TIMESTAMP；         
* CURRENT_ROW_TIMESTAMP()；                   
* NOW()；               
* PROCTIME()：`其中 PROCTIME() 在 1.13 版本及之后版本，返回值类型是 TIMESTAMP_LTZ(3)`。               

在 Flink SQL client 中执行结果如下：          

```shell
Flink SQL> SET sql-client.execution.result-mode=tableau;
[INFO] Session property has been set.

Flink SQL> CREATE VIEW MyView1 AS SELECT LOCALTIME, LOCALTIMESTAMP, CURRENT_DATE, CURRENT_TIME, CURRENT_TIMESTAMP, CURRENT_ROW_TIMESTAMP() AS `CURRENT_ROW_TIMESTAMP()`, NOW() AS `NOW()`, PROCTIME() AS `PROCTIME()`;
[INFO] Execute statement succeed.

Flink SQL> DESC MyView1;
+-------------------+-----------------------------+-------+-----+--------+-----------+
|              name |                        type |  null | key | extras | watermark |
+-------------------+-----------------------------+-------+-----+--------+-----------+
|         LOCALTIME |                     TIME(0) | FALSE |     |        |           |
|    LOCALTIMESTAMP |                TIMESTAMP(3) | FALSE |     |        |           |
|      CURRENT_DATE |                        DATE | FALSE |     |        |           |
|      CURRENT_TIME |                     TIME(0) | FALSE |     |        |           |
| CURRENT_TIMESTAMP |            TIMESTAMP_LTZ(3) | FALSE |     |        |           |
|            EXPR$5 |            TIMESTAMP_LTZ(3) | FALSE |     |        |           |
|            EXPR$6 |            TIMESTAMP_LTZ(3) | FALSE |     |        |           |
|            EXPR$7 | TIMESTAMP_LTZ(3) *PROCTIME* | FALSE |     |        |           |
+-------------------+-----------------------------+-------+-----+--------+-----------+
8 rows in set
```

>在 Flink SQL 中，CREATE VIEW 语句用于创建一个视图（view），它是基于 SQL 查询的虚拟表。视图本身不存储数据，而是在每次查询时动态生成数据。这使得用户能够使用简单的 SQL 查询来访问更复杂的查询结果。      


```shell
Flink SQL> SET table.local-time-zone=UTC;
[INFO] Session property has been set.

Flink SQL> SELECT * FROM MyView1;
WARNING: An illegal reflective access operation has occurred
WARNING: Illegal reflective access by org.apache.flink.api.java.ClosureCleaner (file:/root/flink/flink-1.15.4/lib/flink_udf-1.0-SNAPSHOT.jar) to field java.lang.Class.ANNOTATION
WARNING: Please consider reporting this to the maintainers of org.apache.flink.api.java.ClosureCleaner
WARNING: Use --illegal-access=warn to enable warnings of further illegal reflective access operations
WARNING: All illegal access operations will be denied in a future release
+----+-----------+-------------------------+--------------+--------------+-------------------------+-------------------------+-------------------------+-------------------------+
| op | LOCALTIME |          LOCALTIMESTAMP | CURRENT_DATE | CURRENT_TIME |       CURRENT_TIMESTAMP | CURRENT_ROW_TIMESTAMP() |                   NOW() |              PROCTIME() |
+----+-----------+-------------------------+--------------+--------------+-------------------------+-------------------------+-------------------------+-------------------------+
| +I |  12:10:57 | 2024-02-20 12:10:57.173 |   2024-02-20 |     12:10:57 | 2024-02-20 12:10:57.173 | 2024-02-20 12:10:57.173 | 2024-02-20 12:10:57.173 | 2024-02-20 12:10:57.173 |
+----+-----------+-------------------------+--------------+--------------+-------------------------+-------------------------+-------------------------+-------------------------+
Received a total of 1 row
```



refer   
1.https://nightlies.apache.org/flink/flink-docs-release-1.18/zh/docs/dev/table/functions/systemfunctions/#%e6%97%b6%e9%97%b4%e5%87%bd%e6%95%b0                 
2.https://www.51cto.com/article/709280.html   


