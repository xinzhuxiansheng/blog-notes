# Official - Algebra 扩展      

## 引言  
关系代数是 Calcite 的核心。每个查询都表示为关系运算符树。你可以将 SQL 转换为关系代数，也可以直接构建树。        

规划器规则使用保留语义的数学恒等式来转换表达式树。例如，如果过滤器不引用其他输入中的列，则将过滤器推入内部联接的输入是有效的。      

Calcite 通过将规划器规则重复应用于关系表达式来优化查询。成本模型指导该过程，规划器引擎生成具有与原始语义相同但成本更低的替代表达式。        

规划过程是可扩展的。你可以添加自己的关系运算符、规划器规则、成本模型和统计数据。         

## Algebra builder (代数生成器)    
构建关系表达式的最简单方法是使用代数构建器 RelBuilder。这是一个例子：       

### TableScan   
```java
final FrameworkConfig config;
final RelBuilder builder = RelBuilder.create(config);
final RelNode node = builder
  .scan("EMP")
  .build();
System.out.println(RelOptUtil.toString(node));      
```

（你可以在 `RelBuilderExample.java`(https://github.com/apache/calcite/blob/main/core/src/test/java/org/apache/calcite/examples/RelBuilderExample.java) 中找到此示例和其他示例的完整代码。）  
```java
LogicalTableScan(table=[[scott, EMP]])      
```

它已创建 EMP 表的扫描；相当于SQL        
```
SELECT * FROM scott.EMP;    
```

### Adding a Project (添加项目)        
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


### Adding a Filter and Aggregate (添加过滤器和聚合)   
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
```bash
LogicalFilter(condition=[>($1, 10)])
  LogicalAggregate(group=[{7}], C=[COUNT()], S=[SUM($5)])
    LogicalTableScan(table=[[scott, EMP]])
```

#### Push and pop (推送和弹出)
构建器使用堆栈来存储一个步骤生成的关系表达式，并将其作为输入传递到下一步。这允许生成关系表达式的方法生成构建器。        

大多数时候，你将使用的唯一堆栈方法是 build()，用于获取最后一个关系表达式，即树的根。            

有时，堆栈嵌套得如此之深，以至于令人困惑。为了让事情变得简单，你可以从堆栈中删除表达式。例如，我们在这里构建一个茂密的连接：            

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

### Switch Convention(切换约定)  
在默认情况下，`RelBuilder`创建不带约定的逻辑`RelNode`。但你可以通过`adoptConvention()` 切换到不同的约定：   

```java
final RelNode result = builder
  .push(input)
  .adoptConvention(EnumerableConvention.INSTANCE)
  .sort(toCollation)
  .build();
```

在这个例子中，我们在输入 RelNode 的顶部创建了一个`EnumerableSort`。  

### Field names and ordinals(字段名称和)
你可以按名称或序号引用字段。   

序数是从零开始的。每个运算符保证其输出字段出现的顺序。例如，`Project` 返回每个标量表达式生成的字段。
运算符的字段名称保证是唯一的，但有时这意味着名称不完全是你所期望的。例如，当你将 EMP 连接到 DEPT 时，其中一个输出字段将称为 DEPTNO，另一个输出字段将称为 DEPTNO_1。    

一些关系表达式方法使你可以更好地控制字段名称：   
* project 允许你使用 alias(expr, fieldName) 包装表达式。它会删除包装器，但保留建议的名称（只要它是唯一的）。  

* value(String[] fieldNames, Object...values) 接受字段名称数组。如果数组的任何元素为空，构建器将生成一个唯一的名称。  

如果表达式投影输入字段或输入字段的转换，它将使用该输入字段的名称。  

一旦分配了唯一的字段名称，这些名称就是不可变的。如果你有一个特定的 RelNode 实例，你可以相信字段名称不会改变。事实上，整个关系表达式是不可变的。  

但是，如果关系表达式已经通过了多个重写规则（请参阅 RelOptRule`https://calcite.apache.org/javadocAggregate/org/apache/calcite/plan/RelOptRule.html`），则结果表达式的字段名称可能看起来不太像原始表达式。此时最好按序号引用字段。    

当你构建接受多个输入的关系表达式时，你需要构建考虑到这一点的字段引用。这种情况在构建连接条件时最常发生。  

假设你正在 EMP（有 8 个字段 [EMPNO、ENAME、JOB、MGR、HIREDATE、SAL、COMM、DEPTNO]）和 DEPT（有 3 个字段 [DEPTNO、DNAME、LOC]）上构建联接。在内部，Calcite 将这些字段表示为具有 11 个字段的组合输入行的偏移量：左侧输入的第一个字段是字段 #0（从 0 开始，记住），右侧输入的第一个字段是字段 #8。    

但通过构建器 API，你可以指定哪个输入的哪个字段。要引用“SAL”、内部字段 #5，请编写 builder.field(2, 0, "SAL")、builder.field(2, "EMP", "SAL") 或 builder.field(2, 0, 5)。这意味着“两个输入中输入#0 的字段#5”。 （为什么它需要知道有两个输入？因为它们存储在堆栈中；输入 #1 位于堆栈顶部，输入 #0 位于其下方。如果我们没有告诉构建器这是两个输入，它不知道输入 #0 的深度。）        

同样，要引用“DNAME”、内部字段 #9 (8 + 1)，`请编写 builder.field(2, 1, "DNAME")、builder.field(2, "DEPT", "DNAME") 或 builder。字段(2,1,1)`。     

**小结** 
builder.field(), 的第一个形参表示的输入个数，第二个形参表达的是第几个输入，第三个参数表达的是引用的字段名称。      

### Recursive Queries(递归查询)  
>警告：当前 API 处于实验阶段，如有更改，恕不另行通知。 SQL 递归查询，例如这个生成序列 1, 2, 3, …10：   
```sql
WITH RECURSIVE aux(i) AS (
  VALUES (1)
  UNION ALL
  SELECT i+1 FROM aux WHERE i < 10
)
SELECT * FROM aux
```  

可以使用 TransientTable 和 RepeatUnion 上的扫描来生成：   
```java
final RelNode node = builder
  .values(new String[] { "i" }, 1)
  .transientScan("aux")
  .filter(
      builder.call(
          SqlStdOperatorTable.LESS_THAN,
          builder.field(0),
          builder.literal(10)))
  .project(
      builder.call(
          SqlStdOperatorTable.PLUS,
          builder.field(0),
          builder.literal(1)))
  .repeatUnion("aux", true)
  .build();
System.out.println(RelOptUtil.toString(node));
```  

其产生：  
```bash
LogicalRepeatUnion(all=[true])
  LogicalTableSpool(readType=[LAZY], writeType=[LAZY], tableName=[aux])
    LogicalValues(tuples=[[{ 1 }]])
  LogicalTableSpool(readType=[LAZY], writeType=[LAZY], tableName=[aux])
    LogicalProject($f0=[+($0, 1)])
      LogicalFilter(condition=[<($0, 10)])
        LogicalTableScan(table=[[aux]])
```    

## API summary(API 摘要)   
**关系运算符**     
以下方法创建一个关系表达式（RelNode），将其压入堆栈，并返回 RelBuilder。   

| METHOD      |    DESCRIPTION |
| :-------- | --------:|
| scan(tableName)  | Creates a TableScan. |
| functionScan(operator, n, expr...) functionScan(operator, n, exprList)      | Creates a TableFunctionScan of the n most recent relational expressions. |  
| transientScan(tableName [, rowType])     | Creates a TableScan on a TransientTable with the given type (if not specified, the most recent relational expression’s type will be used). |
| values(fieldNames, value...)  ;  values(rowType, tupleList)  | Creates a Values. |
| filter([variablesSet, ] exprList)
filter([variablesSet, ] expr...)      |  Creates a Filter over the AND of the given predicates; if variablesSet is specified, the predicates may reference those variables. |
| project(expr...) ; project(exprList [, fieldNames])      | Creates a Project. To override the default name, wrap expressions using alias, or specify the fieldNames argument. |
| projectPlus(expr...) ; projectPlus(exprList)      | Variant of project that keeps original fields and appends the given expressions. |
| projectExcept(expr...) ; projectExcept(exprList)      | Variant of project that keeps original fields and removes the given expressions. |
| permute(mapping)      | Creates a Project that permutes the fields using mapping. |
| convert(rowType [, rename])      | Creates a Project that converts the fields to the given types, optionally also renaming them. |  
| aggregate(groupKey, aggCall...) ; aggregate(groupKey, aggCallList)  | Creates an Aggregate. |  
| distinct() | Creates an Aggregate that eliminates duplicate records. |   
| pivot(groupKey, aggCalls, axes, values) | Adds a pivot operation, implemented by generating an Aggregate with a column for each combination of measures and values |    
| unpivot(includeNulls, measureNames, axisNames, axisMap) | Adds an unpivot operation, implemented by generating a Join to a Values that converts each row to several rows |   
| sort(fieldOrdinal...) ; sort(expr...) ; sort(exprList) | Creates a Sort. In the first form, field ordinals are 0-based, and a negative ordinal indicates descending; for example, -2 means field 1 descending. In the other forms, you can wrap expressions in as, nullsFirst or nullsLast. |  
| sortLimit(offset, fetch, expr...) ; sortLimit(offset, fetch, exprList) | Creates a Sort with offset and limit. |    
| limit(offset, fetch) | Creates a Sort that does not sort, only applies with offset and limit. |   
| exchange(distribution) | Creates an Exchange. |   
| sortExchange(distribution, collation) | Creates a SortExchange. |   
| correlate(joinType, correlationId, requiredField...) ; correlate(joinType, correlationId, requiredFieldList) | Creates a Correlate of the two most recent relational expressions, with a variable name and required field expressions for the left relation. |    
| join(joinType, expr...) ; join(joinType, exprList) ; join(joinType, fieldName...) | Creates a Join of the two most recent relational expressions.    The first form joins on a boolean expression (multiple conditions are combined using AND).   The last form joins on named fields; each side must have a field of each name. |   
| semiJoin(expr) | Creates a Join with SEMI join type of the two most recent relational expressions. |   
| antiJoin(expr) | Creates a Join with ANTI join type of the two most recent relational expressions. |    
| union(all [, n]) | Creates a Union of the n (default two) most recent relational expressions. |   
| intersect(all [, n]) | Creates an Intersect of the n (default two) most recent relational expressions. |  
| minus(all) | Creates a Minus of the two most recent relational expressions. |    
| repeatUnion(tableName, all [, n]) | Creates a RepeatUnion associated to a TransientTable of the two most recent relational expressions, with n maximum number of iterations (default -1, i.e. no limit). |   
| sample(bernoulli, rate [, repeatableSeed]) | Creates a sample of at given sampling rate. |   
| snapshot(period) | Creates a Snapshot of the given snapshot period. |   
| match(pattern, strictStart, strictEnd, patterns, measures, after, subsets, allRows, partitionKeys, orderKeys, interval) | Creates a Match. |    

## 参数类型:  
- `expr, interval RexNode`: 表达式, 间隔 RexNode
- `expr...`, `requiredField... Array of RexNode`: 表达式..., 必需字段... RexNode 数组
- `exprList`, `measureList`, `partitionKeys`, `orderKeys`, `requiredFieldList Iterable of RexNode`: 表达式列表, 度量列表, 分区键, 排序键, 必需字段列表 RexNode 可迭代
- `fieldOrdinal`: 行中字段的序数(从0开始)
- `fieldName`: 行内字段名称, 唯一
- `fieldName... Array of String`: 字段名称... 字符串数组
- `fieldNames Iterable of String`: 字段名称 字符串可迭代
- `rowType RelDataType`: 行类型 RelDataType
- `groupKey RelBuilder.GroupKey`: 分组键 RelBuilder.GroupKey
- `aggCall... Array of RelBuilder.AggCall`: 聚合调用... RelBuilder.AggCall 数组
- `aggCallList Iterable of RelBuilder.AggCall`: 聚合调用列表 RelBuilder.AggCall 可迭代
- `value... Array of Object`: 值... 对象数组
- `value Object`: 值 对象
- `tupleList Iterable of List of RexLiteral`: 元组列表 RexLiteral 列表可迭代
- `all`, `distinct`, `strictStart`, `strictEnd`, `allRows boolean`: 全部, 去重, 严格开始, 严格结束, 全部行 布尔值
- `alias String`: 别名 字符串
- `correlationId CorrelationId`: 相关性ID CorrelationId
- `variablesSet Iterable of CorrelationId`: 变量集 CorrelationId 可迭代
- `varHolder Holder of RexCorrelVariable`: 变量持有者 RexCorrelVariable 持有者
- `patterns Map whose key is String, value is RexNode`: 模式 键为字符串, 值为 RexNode 的映射
- `subsets Map whose key is String, value is a sorted set of String`: 子集 键为字符串, 值为已排序字符串集的映射
- `distribution RelDistribution`: 分布 RelDistribution
- `collation RelCollation`: 排序 RelCollation
- `operator SqlOperator`: 操作符 SqlOperator
- `joinType JoinRelType`: 连接类型 JoinRelType

构建器方法执行各种优化，包括:

- `project` 如果被要求投影所有列，则返回其输入
- `filter` 平坦化条件（因此 `AND` 和 `OR` 可能有超过两个子节点），简化（例如将 `x = 1 AND TRUE` 转换为 `x = 1`）
- 如果你先应用排序然后限制，效果就像你调用了 `sortLimit`    

有一些注释方法可以将信息添加到堆栈顶部的关系表达式中：     
| Method      |    Description |
| :-------- | --------:|
| as(alias)  | Assigns a table alias to the top relational expression on the stack |
| variable(varHolder)     | Creates a correlation variable referencing the top relational expression |    

## Stack methods(堆栈方法)   
| Method      |    Description |
| :-------- | --------:|
| build() | Pops the most recently created relational expression off the stack |   
| push(rel) | Pushes a relational expression onto the stack. Relational methods such as scan, above, call this method, but user code generally does not |   
| pushAll(collection) | Pushes a collection of relational expressions onto the stack |   
| peek() | Returns the relational expression most recently put onto the stack, but does not remove it |   

## Scalar expression methods(标量表达式方法)
以下方法返回标量表达式 (RexNode)。   
其中许多都使用堆栈的内容。例如，`field("DEPTNO")` 返回对刚刚添加到堆栈的关系表达式的“DEPTNO”字段的引用。      
| Method      |    Description |
| :-------- | --------:|
| literal(value) | Constant |   
| field(fieldName) | Reference, by ordinal, to a field of the top-most relational expression |  
| field(inputCount, inputOrdinal, fieldName) | Reference, by name, to a field of the (inputCount - inputOrdinal)th relational expression |  
| field(inputCount, inputOrdinal, fieldOrdinal) | Reference, by ordinal, to a field of the (inputCount - inputOrdinal)th relational expression |  
| field(inputCount, alias, fieldName) | Reference, by table alias and field name, to a field at most inputCount - 1 elements from the top of the stack |  
| field(alias, fieldName) | Reference, by table alias and field name, to a field of the top-most relational expressions |  
| field(expr, fieldName) | Reference, by name, to a field of a record-valued expression |  
| field(expr, fieldOrdinal) | Reference, by ordinal, to a field of a record-valued expression |  
| fields(fieldOrdinalList) | List of expressions referencing input fields by ordinal |   
| fields(mapping) | List of expressions referencing input fields by a given mapping |   
| fields(collation) | List of expressions, exprList, such that sort(exprList) would replicate collation|  
| call(op, expr...) ; call(op, exprList) | Call to a function or operator |   
| and(expr...) ; and(exprList) | Logical AND. Flattens nested ANDs, and optimizes cases involving TRUE and FALSE. |   
| or(expr...) ; or(exprList) | Logical OR. Flattens nested ORs, and optimizes cases involving TRUE and FALSE. |  
| not(expr) | Logical NOT |   
| equals(expr, expr) | Equals |   
| isNull(expr) | Checks whether an expression is null |   
| isNotNull(expr) | Checks whether an expression is not null |  
| alias(expr, fieldName) | Renames an expression (only valid as an argument to project) |   
| cast(expr, typeName) ; cast(expr, typeName, precision) ; cast(expr, typeName, precision, scale) | Converts an expression to a given type |  
| desc(expr) | Changes sort direction to descending (only valid as an argument to sort or sortLimit) |  
| nullsFirst(expr) | Changes sort order to nulls first (only valid as an argument to sort or sortLimit) |  
| nullsLast(expr) | Changes sort order to nulls last (only valid as an argument to sort or sortLimit) |  
| cursor(n, input) | Reference to inputth (0-based) relational input of a TableFunctionScan with n inputs (see functionScan) |  

## Sub-query methods(子查询方法)
以下方法将子查询转换为标量值（在 in、exists、some、all、unique 的情况下为 BOOLEAN；对于 scalarQuery 为任何标量类型）。 arrayQuery 的 ARRAY、mapQuery 的 MAP 和 multisetQuery 的 MULTISET）。   

在下文中，relFn 是一个接受 RelBuilder 参数并返回 RelNode 的函数。你通常将其实现为 lambda；该方法使用具有正确上下文的 RelBuilder 调用你的代码，并且你的代码返回将成为子查询的 RelNode。       

>具体 API 可访问： https://calcite.apache.org/docs/algebra.html#sub-query-methods    

## Pattern methods(模式方法)
以下方法返回用于匹配的模式。     

>具体 API 可访问：https://calcite.apache.org/docs/algebra.html#pattern-methods     

## Group key methods(分组关键方法)   
以下方法返回 RelBuilder.GroupKey。    

>具体 API 可访问： https://calcite.apache.org/docs/algebra.html#group-key-methods   

## Aggregate call methods(聚合调用方法)   
以下方法返回 RelBuilder.AggCall。     

>具体 API 可访问： https://calcite.apache.org/docs/algebra.html#aggregate-call-methods  

## Windowed aggregate call methods(窗口聚合调用方法)  
要创建表示对窗口聚合函数的调用的 RelBuilder.OverCall，请创建聚合调用，然后调用其 over() 方法，例如 count().over()。     
要进一步修改 OverCall，请调用其方法：   

>具体 API 可访问： https://calcite.apache.org/docs/algebra.html#windowed-aggregate-call-methods   

