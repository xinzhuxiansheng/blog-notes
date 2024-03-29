## Streaming    

Calcite 扩展了 SQL 和关系代数以支持流式查询。   

### Introduction (简介) 
流是持续、永久流动的记录的集合。与表不同，它们通常不存储在磁盘上，而是通过网络流动并在内存中短暂保存。          

流是对表的补充，因为它们代表企业现在和未来正在发生的事情，而表则代表过去。将流归档到表中是很常见的。        

与表一样，你通常希望使用基于关系代数的高级语言来查询流，根据模式进行验证，并进行优化以利用可用的资源和算法。    

Calcite 的 SQL 是标准 SQL 的扩展，而不是另一种“类 SQL”语言。这种区别很重要，原因如下：      

* 对于任何了解常规 SQL 的人来说，流式 SQL 都很容易学习。        
* 语义很清晰，因为我们的目标是在流上产生相同的结果，就像表中存在相同的数据一样。        
* 你可以编写组合流和表（或流的历史记录，基本上是内存中的表）的查询。        
* 许多现有工具可以生成标准 SQL。        

如果不使用 STREAM 关键字，您将回到常规标准 SQL。            

### An example schema （示例）  
我们的流式 SQL 示例使用以下架构：       
* 订单（rowtime、productId、orderId、units）- 流和表        
* 产品（rowtime、productId、名称）- 一个表      
* Shipments (rowtime, orderId) - 一个流         

### A simple query (一个简单的查询)     
让我们从最简单的流式查询开始：          
