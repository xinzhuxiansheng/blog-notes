# Official - Streaming - Overview 扩展     

>Calcite 扩展了 SQL 和关系代数以支持流式查询。      

## 引言
流是不断流动且永不停歇的记录集合。`与表不同，流通常不会存储在磁盘上，而是在网络上传输，并在内存中短时间保存`。      

流补充了表，因为它们代表了企业的当前和未来动态，而表则代表了过去。将流归档到表中是非常常见的做法。       

像表一样，你经常希望用基于关系代数的高级语言来查询流，这些查询根据模式进行验证，并经过优化以充分利用可用资源和算法。      

Calcite 的 SQL 是对标准 SQL 的扩展，而不是另一种‘类似 SQL’的语言。这个区别很重要，原因有以下几点：      
- 对于了解常规 SQL 的人来说，学习流 SQL 很容易。      
- 语义是清晰的，因为我们旨在对流生成与表中相同数据相同的结果。     
- 你可以编写结合流和表（或流的历史，基本上是内存表）的查询。     
- 许多现有工具可以生成标准 SQL。       

如果你不使用 `STREAM 关键字`，你将回到常规的标准 SQL。     

>Introduction (https://calcite.apache.org/docs/stream.html#introduction)     

## schema 示例     
我们的流式 SQL 示例使用以下架构：     
Orders (rowtime, productId, orderId, units) - a stream and a table   
Products (rowtime, productId, name) - a table    
Shipments (rowtime, orderId) - a stream         




为了在实践中学习，我们先在 MySQL中，创建 3个 测试表，Orders、Products、Shipments,         

### Orders 数据准备  
```sql
create table Orders
(
    rowtime   varchar(50) null,
    productId int         null,
    orderId   int         null,
    units     int         null
);

INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('10:17:00', 30, 5, 4);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('10:17:05', 10, 6, 1);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('10:18:05', 20, 7, 2);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('10:18:07', 30, 8, 20);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('11:02:00', 10, 9, 6);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('11:04:00', 10, 10, 1);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('11:09:30', 40, 11, 12);
INSERT INTO yzhou_test.Orders (rowtime, productId, orderId, units) VALUES ('11:24:11', 10, 12, 4);

```




>An example schema (https://calcite.apache.org/docs/stream.html#an-example-schema)    

## 查询示例   
让我们从最简单的流式查询开始：      



>A simple query (https://calcite.apache.org/docs/stream.html#a-simple-query)   

* Filtering rows (https://calcite.apache.org/docs/stream.html#filtering-rows)   

* Projecting expressions (https://calcite.apache.org/docs/stream.html#projecting-expressions)   

* Tumbling windows (https://calcite.apache.org/docs/stream.html#tumbling-windows)    

* Tumbling windows, improved (https://calcite.apache.org/docs/stream.html#tumbling-windows-improved)    

* Hopping windows (https://calcite.apache.org/docs/stream.html#hopping-windows)    

* GROUPING SETS (https://calcite.apache.org/docs/stream.html#grouping-sets)    

* Filtering after aggregation (https://calcite.apache.org/docs/stream.html#filtering-after-aggregation)   

* Sub-queries, views and SQL’s closure property (https://calcite.apache.org/docs/stream.html#sub-queries-views-and-sqls-closure-property)  

* Converting between streams and relations (https://calcite.apache.org/docs/stream.html#converting-between-streams-and-relations)    

* The “pie chart” problem: Relational queries on streams (The “pie chart” problem: Relational queries on streams)   

* Sorting (https://calcite.apache.org/docs/stream.html#sorting)   

* Table constructor (https://calcite.apache.org/docs/stream.html#table-constructor)   

* Sliding windows (https://calcite.apache.org/docs/stream.html#sliding-windows)   

* Cascading windows (https://calcite.apache.org/docs/stream.html#cascading-windows)   

* Joining streams to tables (https://calcite.apache.org/docs/stream.html#joining-streams-to-tables)   

* Joining streams to streams (https://calcite.apache.org/docs/stream.html#joining-streams-to-streams)    

* DML (https://calcite.apache.org/docs/stream.html#dml)    

* Punctuation (https://calcite.apache.org/docs/stream.html#punctuation)    

* State of the stream (https://calcite.apache.org/docs/stream.html#state-of-the-stream)   
    Implemented (https://calcite.apache.org/docs/stream.html#implemented)   

    Not implemented (https://calcite.apache.org/docs/stream.html#not-implemented)   

    To do in this document (https://calcite.apache.org/docs/stream.html#to-do-in-this-document)    

* Functions (https://calcite.apache.org/docs/stream.html#functions)   

* References (https://calcite.apache.org/docs/stream.html#references)    


refer   
1.https://calcite.apache.org/docs/stream.html   
