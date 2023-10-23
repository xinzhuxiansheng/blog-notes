## Algebra      

关系代数是 Calcite 的核心。每个查询都表示为关系运算符树。您可以将 SQL 转换为关系代数，也可以直接构建树。        

规划器规则使用保留语义的数学恒等式来转换表达式树。例如，如果过滤器不引用其他输入中的列，则将过滤器推入内部联接的输入是有效的。      

Calcite 通过将规划器规则重复应用于关系表达式来优化查询。成本模型指导该过程，规划器引擎生成具有与原始语义相同但成本更低的替代表达式。        

规划过程是可扩展的。您可以添加自己的关系运算符、规划器规则、成本模型和统计数据。        

### Algebra builder (代数生成器)    
构建关系表达式的最简单方法是使用代数构建器 RelBuilder。这是一个例子：       

**TableScan**   
```java
final FrameworkConfig config;
final RelBuilder builder = RelBuilder.create(config);
final RelNode node = builder
  .scan("EMP")
  .build();
System.out.println(RelOptUtil.toString(node));      
```

（您可以在 `RelBuilderExample.java` 中找到此示例和其他示例的完整代码。）代码打印出来:     
```java
LogicalTableScan(table=[[scott, EMP]])      
```
它已创建 EMP 表的扫描；相当于SQL        
```
SELECT * FROM scott.EMP;    
```

#### Adding a Project (添加项目)        
现在，让我们添加一个项目，相当于    
```sql
SELECT ename, deptno FROM scott.EMP;    
```  
我们只是在调用 build 之前添加对项目方法的调用： 
```java 
final RelNode node = builder
  .scan("EMP")
  .project(builder.field("DEPTNO"), builder.field("ENAME"))
  .build();
System.out.println(RelOptUtil.toString(node));
```
输出是:     
```
LogicalProject(DEPTNO=[$7], ENAME=[$1])
  LogicalTableScan(table=[[scott, EMP]])        
```     
对 builder.field 的两次调用创建简单的表达式，该表达式返回输入关系表达式中的字段，即由 scan 调用创建的 TableScan。       
Calcite 已将它们按序号、$7 和 $1 转换为字段引用。   


#### Adding a Filter and Aggregate (添加过滤器和聚合)   
带有聚合和过滤器的查询：        
```java
final RelNode node = builder
  .scan("EMP")
  .aggregate(builder.groupKey("DEPTNO"),
      builder.count(false, "C"),
      builder.sum(false, "S", builder.field("SAL")))
  .filter(
      builder.call(SqlStdOperatorTable.GREATER_THAN,
          builder.field("C"),
          builder.literal(10)))
  .build();
System.out.println(RelOptUtil.toString(node));      
```     
相当于SQL:  
```sql
SELECT deptno, count(*) AS c, sum(sal) AS s
FROM emp
GROUP BY deptno
HAVING count(*) > 10
```
并产生      

```
LogicalFilter(condition=[>($1, 10)])
  LogicalAggregate(group=[{7}], C=[COUNT()], S=[SUM($5)])
    LogicalTableScan(table=[[scott, EMP]])
```

#### Push and pop (推送和弹出)
构建器使用堆栈来存储一个步骤生成的关系表达式，并将其作为输入传递到下一步。这允许生成关系表达式的方法生成构建器。        

大多数时候，您将使用的唯一堆栈方法是 build()，用于获取最后一个关系表达式，即树的根。            

有时，堆栈嵌套得如此之深，以至于令人困惑。为了让事情变得简单，您可以从堆栈中删除表达式。例如，我们在这里构建一个茂密的连接：            

```
.
               join
             /      \
        join          join
      /      \      /      \
CUSTOMERS ORDERS LINE_ITEMS PRODUCTS
```

我们分三个阶段构建它。将中间结果存储在变量 left 和 right 中，并在创建最终 Join 时使用 push() 将它们放回到堆栈中：       
```java
final RelNode left = builder
  .scan("CUSTOMERS")
  .scan("ORDERS")
  .join(JoinRelType.INNER, "ORDER_ID")
  .build();

final RelNode right = builder
  .scan("LINE_ITEMS")
  .scan("PRODUCTS")
  .join(JoinRelType.INNER, "PRODUCT_ID")
  .build();

final RelNode result = builder
  .push(left)
  .push(right)
  .join(JoinRelType.INNER, "ORDER_ID")
  .build();
``` 

