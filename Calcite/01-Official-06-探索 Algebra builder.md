# Calcite - 探索 Algebra builder      

## 引言   
在上一篇 `Calcite - 探索 Relation Algebra (关系代数)` 公众号文章中介绍了 `Algebra` 官方文档的 `Relation Algebra` 部分，该篇内容会继续探索 `Algebra builder` 部分。       

## Algebra builder      
`Apache Calcite 的 Relation Algebra` 既支持使用 SQL 来构造，也支持使用 `RelBuilder` 来构造。但在使用 SQL 查询数据时，我们并不关心它是如何转换成 `Relation Algebra`，但我们又愿意花时间执行 `EXPLAIN` 或者 `EXPLAIN ANALYZE` 来查看 SQL 的执行计划；至少在了解 SQL 是否命名中索引的场景下，它是非常好用的。  

在当前开发工作中，SQL 引擎相关内容涉及不多，但用上 `Calcite RelBuilder` 构建查询功能时，有种让我更贴近 `relational-algebra expression`、 `optimizer`、`execution plan` 的感受。下面回到 `Algebra builder` 实践部分。    

>官方文档的伪代码可以在 `RelBuilderExample`(https://github.com/apache/calcite/blob/main/core/src/test/java/org/apache/calcite/examples/RelBuilderExample.java) 单测中寻找到。 但为了示例的完整性，博主会将代码会将拆分成一个个完整示例。             

>对于 `relational expression` 术语比较陌生的同学可以阅读我的上一篇文章 `Calcite - 探索 Relation Algebra (关系代数)`，我们大量谈论到该术语。   

## TableScan         
文档中介绍 `RelBuilder` 是构建 Relational Expression 最简单的方法, 在 `YzhouCsvTest_withoutjson_withrelbuilder_tablescan.java` 示例中  RelBuilder 的 scan("scott", "dept") 用于扫描 `avamain-calcite\\src\\main\\resources\\scott` 目录下 dept.csv 获取数据, 它等同的语义 SQL 是 `SELECT * FROM scott.dept`，最后通过 build() 方法生成一个 `RelNode` 对象, 然后通过 `RelOptUtil.toString(rel)`方法将 RelNode 对象打印出来。最后输出的结果为 `LogicalTableScan(table=[[scott, dept]])`。这是一个最简单的示例，我不知道大家是否跟我有一样的疑问？          

**1.** `RelBuilder` 构造生成的 `RelNode` 对象是什么？它是一个 `Relational Expression`么？ 至少就像文档中描述的那样：`The simplest way to build a relational expression is to use the algebra builder, RelBuilder.` 我看到 `RelOptUtil.toString()` 它的方法注释是 `Converts a relational expression to a string, showing just basic attributes.`     
```bash
/**
  * Converts a relational expression to a string, showing just basic
  * attributes.
  */
public static String toString(final RelNode rel) {
  return toString(rel, SqlExplainLevel.EXPPLAN_ATTRIBUTES);
}
```

也可以看到 RelBuilder 类的注释描述，也同样介绍它是用于构造 `relational expressions`。                 
```bash
/**
 * Builder for relational expressions.
 *
 * <p>{@code RelBuilder} does not make possible anything that you could not
 * also accomplish by calling the factory methods of the particular relational
 * expression. But it makes common tasks more straightforward and concise.
 *
 * <p>{@code RelBuilder} uses factories to create relational expressions.
 * By default, it uses the default factories, which create logical relational
 * expressions ({@link LogicalFilter},
 * {@link LogicalProject} and so forth).
 * But you could override those factories so that, say, {@code filter} creates
 * instead a {@code HiveFilter}.
 *
 * <p>It is not thread-safe.
 */
@Value.Enclosing
public class RelBuilder {
... 省略部分代码
```

所以，看到这些，我理解 RelNode 对象它就代表的是 `relational algebra expression`。       

**2.** 该不该继续让 `逻辑计划（Logical Plan）` 术语出现在文章？ 我非常很质疑这个词了，示例中输出的 `LogicalTableScan`, 它包含 `Logical`，它让我一开始有些弄不清 `逻辑计划` 与 `relational algebra expression`。      

我们可以看 `《Calcite数据管理实战》的 8.2.1 优化器介绍`章节中介绍一条 SQL 语句大概会经历以下5个步骤，如下描述；可这部分与我阅读 `《Database System Concepts 7》 CHAPTER 15 Query Processing` 内容介绍 SQL 经历的过程之间，它们存在术语上的差异，但流程步骤上是一致的。你在《Database System Concepts 7》整片书中都找不到 `逻辑计划（Logical Plan）`或者 `物理计划（Physical Plan）`。可是这两个中文名词在我的知识体系里面早已扎根，我几乎在大数据 SQL 相关的书籍中，都能找到这两个词，可我又去百度百科，维基百科（中英）都没有检索到逻辑计划和物理计划的词条。有些惶恐不定，我之所以提出该疑问，目标是了解 Calcite 的 RelBuilder 构造生成的 `RelNode` 对象是属于哪个处理阶段？ 有了以上的这些了解，后续的 Calcite 的文章中便不会再出现逻辑计划、逻辑计划优化、物理计划，而相对应的是 relational algebra expression， optimizer，execution plan。这样会统一我表述 SQL 引擎处理流程，也可以更好的理解 Apache Calcite 的 API。    

到这里，`YzhouCsvTest_withoutjson_withrelbuilder_tablescan.java` 示例输出的 `LogicalTableScan(table=[[scott, emp]])` ,可以很明确的表达它是一个 `relational algebra expression`。    

>维基百科词条有:Relational algebra, Query optimization, Query plan 等。  

**《Calcite数据管理实战》的 8.2.1 优化器介绍 内容片段**      
```bash
一条SQL语句大概会经历以下5个步骤。
（1）解析：把SQL语句解析成抽象语法树。
（2）校验：根据元数据信息校验字段、表等信息是否合法。
（3）逻辑计划：生成最初版本的逻辑计划。
（4）逻辑计划优化：对前一阶段生成的逻辑计划进行进一步优化。
（5）物理执行：生成物理计划，执行具体物理计划。  

逻辑计划优化刚好位于逻辑计划和物理执行之间。生成的未经优化的逻辑计划完全是根据输入的SQL语句直接转换而来的。也就是说，输入的SQL语句是什么样子，逻辑计划便会一对一地将其转换为关系代数模型。
```

**《Database System Concepts 7》 CHAPTER 15 Query Processing 内容片段**    
```bash
The steps involved in processing a query appear in Figure 15.1. The basic steps are:           
1. Parsing and translation.              
2. Optimization.              
3. Evaluation.            

Before query processing can begin, the system must translate the query into a usable form. A language such as SQL is suitable for human use, but it is ill suited to be the system’s internal representation of a query. A more useful internal representation
is one based on the ex  tended relational algebra.

Thus, the first action the system must take in query processing is to translate a given query into its internal form. This translation process is similar to the work performed by the parser of a compiler. In generating the internal form of the query, the parser checks the syntax of the user’s query, verifies that the relation names appearing in the query are names of the relations in the database, and so on. The system constructs a parse-tree representation of the query, which it then translates into a relational-algebra expression. If the query was expressed in terms of a view, the translation phase also replaces all uses of the view by the relational-algebra expression that defines the view. Most compiler texts cover parsing in detail.      
```  
**图02 SQL处理流程图**    
![relbuilder02](http://img.xinzhuxiansheng.com/blogimgs/calcite/relbuilder02.png)  

**YzhouCsvTest_withoutjson_withrelbuilder_tablescan.java 示例代码， csv file 内容如下所示:**                  

![relbuilder01](http://img.xinzhuxiansheng.com/blogimgs/calcite/relbuilder01.png)      

**dept.csv**        
```bash
DEPTNO,DNAME,LOC
10,ACCOUNTING,NEW YORK
20,RESEARCH,DALLAS
30,SALES,CHICAGO
40,OPERATIONS,BOSTON
```

**emp.csv**         
```bash
EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO
7369,SMITH,CLERK,7902,1980-12-17,800,,20
7499,ALLEN,SALESMAN,7698,1981-02-20,1600,300,30
7521,WARD,SALESMAN,7698,1981-02-22,1250,500,30
7566,JONES,MANAGER,7839,1981-04-02,2975,,20
7654,MARTIN,SALESMAN,7698,1981-09-28,1250,1400,30
7698,BLAKE,MANAGER,7839,1981-05-01,2850,,30
7782,CLARK,MANAGER,7839,1981-06-09,2450,,10
7788,SCOTT,ANALYST,7566,1982-12-09,3000,,20
7839,KING,PRESIDENT,,1981-11-17,5000,,10
7844,TURNER,SALESMAN,7698,1981-09-08,1500,0,30
7876,ADAMS,CLERK,7788,1983-01-12,1100,,20
7900,JAMES,CLERK,7698,1981-12-03,950,,30
7902,FORD,ANALYST,7566,1981-12-03,3000,,20
7934,MILLER,CLERK,7782,1982-01-23,1300,,10
```

**YzhouCsvTest_withoutjson_withrelbuilder_tablescan.java**        
```java
public class YzhouCsvTest_withoutjson_withrelbuilder_tablescan {

    public static void main(String[] args) throws SQLException {
        Properties props = new Properties();
        props.setProperty("caseSensitive", "false");
        props.setProperty("lex", Lex.JAVA.toString());
        try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
             CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
            SchemaPlus rootSchema = calciteConnection.getRootSchema();
            File csvDir = new File("javamain-calcite\\src\\main\\resources\\scott");
            CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);

            SchemaPlus scottSchema = rootSchema.add("scott", new AbstractSchema());
            scottSchema.add("dept", csvSchema.getTable("dept"));
            scottSchema.add("emp", csvSchema.getTable("emp"));

            FrameworkConfig config = Frameworks.newConfigBuilder()
                    .parserConfig(
                            SqlParser.config()
                                    .withLex(Lex.JAVA)
                                    .withCaseSensitive(false)
                    )
                    .defaultSchema(rootSchema)
                    .build();
            RelBuilder builder = RelBuilder.create(config);
            final RelNode rel = builder
                    .scan("scott", "emp")
                    .build();
            System.out.println(RelOptUtil.toString(rel));

            RelRunner runner = connection.unwrap(RelRunner.class);
            try (PreparedStatement preparedStatement = runner.prepareStatement(rel);
                 ResultSet resultSet = preparedStatement.executeQuery()) {
                print(resultSet);
            }
        }
    }

    private static void print(ResultSet resultSet) throws SQLException {
        final ResultSetMetaData metaData = resultSet.getMetaData();
        final int columnCount = metaData.getColumnCount();
        while (resultSet.next()) {
            for (int i = 1; ; i++) {
                System.out.print(resultSet.getString(i));
                if (i < columnCount) {
                    System.out.print(", ");
                } else {
                    System.out.println();
                    break;
                }
            }
        }
    }
}
```
 
**输出结果：**    
```bash
LogicalTableScan(table=[[scott, emp]])

7369, SMITH, CLERK, 7902, 1980-12-17, 800, , 20
7499, ALLEN, SALESMAN, 7698, 1981-02-20, 1600, 300, 30
7521, WARD, SALESMAN, 7698, 1981-02-22, 1250, 500, 30
7566, JONES, MANAGER, 7839, 1981-04-02, 2975, , 20
7654, MARTIN, SALESMAN, 7698, 1981-09-28, 1250, 1400, 30
... 省略部分数据
```

我们也可以利用 `RelToSqlConverter` 将 `YzhouCsvTest_withoutjson_withrelbuilder_tablescan` 示例中的 rel 对象转成 SQL 语句。示例代码如下：   
```java
private static void relToSql(RelNode rel){
    SqlDialect dialect = CalciteSqlDialect.DEFAULT;
    RelToSqlConverter converter = new RelToSqlConverter(dialect);
    SqlNode sqlNode = converter.visitRoot(rel).asStatement();
    String generatedSql = sqlNode.toSqlString(dialect).getSql();
    System.out.println("\n=== Generated SQL ===");
    System.out.println(generatedSql);
}
```

### 小结
通过对 `TableScan` 示例的实践，我想你应该有所体会，`RelBuilder` 让你有种深入 SQL 引擎流程的快感，我想这就是 Apache Calicte `厉害之处`，随着不断深入，我相信会对 SQL 引擎的实现有较深的理解。**千万别忽略 RelOptUtil.toString() 输出的 relational algebra expression**,读懂它们也是我们重中之重。           

## Adding a Project  
有了上一篇 `Calcite - 探索 Relation Algebra (关系代数)`内容的理解，Project 你再也不会翻译成 "项目"，而它的含义是 `投影`，它对应关系代数中的 `Project Operation`。     

在 `YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project`示例代码中的 ReBuilder 调用 project() 方法添加了 Project Operation, 通过 RelOptUtil.toString() 方法打印出的 `relational algebra expression` 多了 `LogicalProject(DEPTNO=[$7], ENAME=[$1])` 部分。而 $7,$1 各自代表 `emp.DEPTNO` 和 `emp.ENAME` 字段的序号，默认从 0 开始。其次 `relational algebra expression` 的输出结果呈现层级结构，它对应的是 `expression tree`的结构。   

这里需要额外注意的是 `.project(builder.field("DEPTNO"), builder.field("ENAME"))` 方法的形参类型是 RexNode, 未来我们会有很长的一段路需要跟它打交道，就像 RelNode 一样。       

![relbuilder03](http://img.xinzhuxiansheng.com/blogimgs/calcite/relbuilder03.png)   

**YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project.java**     
```java
public class YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project {

    public static void main(String[] args) throws SQLException {
        Properties props = new Properties();
        props.setProperty("caseSensitive", "false");
        props.setProperty("lex", Lex.JAVA.toString());
        try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
             CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
            SchemaPlus rootSchema = calciteConnection.getRootSchema();
            File csvDir = new File("javamain-calcite\\src\\main\\resources\\scott");
            CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);

            SchemaPlus scottSchema = rootSchema.add("scott", new AbstractSchema());
            scottSchema.add("dept", csvSchema.getTable("dept"));
            scottSchema.add("emp", csvSchema.getTable("emp"));

            FrameworkConfig config = Frameworks.newConfigBuilder()
                    .parserConfig(
                            SqlParser.config()
                                    .withLex(Lex.JAVA)
                                    .withCaseSensitive(false)
                    )
                    .defaultSchema(rootSchema)
                    .build();
            RelBuilder builder = RelBuilder.create(config);

            final RelNode rel = builder
                    .scan("scott", "emp")
                    .project(builder.field("DEPTNO"), builder.field("ENAME"))
                    .build();
            System.out.println(RelOptUtil.toString(rel));
            relToSql(rel);
            System.out.println();

            RelRunner runner = connection.unwrap(RelRunner.class);
            try (PreparedStatement preparedStatement = runner.prepareStatement(rel);
                 ResultSet resultSet = preparedStatement.executeQuery()) {
                print(resultSet);
            }
        }
    }

    private static void print(ResultSet resultSet) throws SQLException {
        final ResultSetMetaData metaData = resultSet.getMetaData();
        final int columnCount = metaData.getColumnCount();
        while (resultSet.next()) {
            for (int i = 1; ; i++) {
                System.out.print(resultSet.getString(i));
                if (i < columnCount) {
                    System.out.print(", ");
                } else {
                    System.out.println();
                    break;
                }
            }
        }
    }

    private static void relToSql(RelNode rel){
        SqlDialect dialect = CalciteSqlDialect.DEFAULT;
        RelToSqlConverter converter = new RelToSqlConverter(dialect);
        SqlNode sqlNode = converter.visitRoot(rel).asStatement();
        String generatedSql = sqlNode.toSqlString(dialect).getSql();
        System.out.println("\n=== Generated SQL ===");
        System.out.println(generatedSql);
    }
}
```

**输出结果：**      
```bash
LogicalProject(DEPTNO=[$7], ENAME=[$1])
  LogicalTableScan(table=[[scott, emp]])


=== Generated SQL ===
SELECT "DEPTNO", "ENAME"
FROM "scott"."emp"

20, SMITH
30, ALLEN
30, WARD
20, JONES
30, MARTIN
... 省略部分数据
```

## Adding a Filter and Aggregate        
`YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project_filter_aggregate.java` 示例通过 RelBuilder 构造 `SELECT deptno, count(*) AS c, sum(sal) AS s FROM emp GROUP BY deptno HAVING count(*) > 2` 查询，因为数据有限，所以将官网示例 HAVING COUNT(*) > 10 改为 HAVING COUNT(*) > 2， 这里需要特别注意的是：在 emp.csv 文件中，定义的 field 默认类型是 String，当前示例中，我们要构建的查询包含计算了 SUM("SAL") AS s。所以需要显示将 SAL 的类型设置为 int，这样才能满足计算要求。    
![relbuilder04](http://img.xinzhuxiansheng.com/blogimgs/calcite/relbuilder04.png)      

**YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project_filter_aggregate.java**    
```java
public class YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project_filter_aggregate {

    public static void main(String[] args) throws SQLException {
        Properties props = new Properties();
        props.setProperty("caseSensitive", "false");
        props.setProperty("lex", Lex.JAVA.toString());
        try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
             CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
            SchemaPlus rootSchema = calciteConnection.getRootSchema();
            File csvDir = new File("javamain-calcite\\src\\main\\resources\\scott_fieldtype");
            CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);

            SchemaPlus scottSchema = rootSchema.add("scott", new AbstractSchema());
            scottSchema.add("dept", csvSchema.getTable("dept"));
            scottSchema.add("emp", csvSchema.getTable("emp"));

            FrameworkConfig config = Frameworks.newConfigBuilder()
                    .parserConfig(
                            SqlParser.config()
                                    .withLex(Lex.JAVA)
                                    .withCaseSensitive(false)
                    )
                    .defaultSchema(rootSchema)
                    .build();
            RelBuilder builder = RelBuilder.create(config);

            final RelNode rel = builder
                    .scan("scott", "emp")
                    .aggregate(builder.groupKey("DEPTNO"),
                            builder.count(false, "C"),
                            builder.sum(false, "S", builder.field("SAL")))
                    .filter(
                            builder.call(SqlStdOperatorTable.GREATER_THAN,
                                    builder.field("C"),
                                    builder.literal(2)))
                    .build();
            System.out.println(RelOptUtil.toString(rel));
            relToSql(rel);
            System.out.println();

            RelRunner runner = connection.unwrap(RelRunner.class);
            try (PreparedStatement preparedStatement = runner.prepareStatement(rel);
                 ResultSet resultSet = preparedStatement.executeQuery()) {
                print(resultSet);
            }
        }
    }

    private static void print(ResultSet resultSet) throws SQLException {
        final ResultSetMetaData metaData = resultSet.getMetaData();
        final int columnCount = metaData.getColumnCount();
        while (resultSet.next()) {
            for (int i = 1; ; i++) {
                System.out.print(resultSet.getString(i));
                if (i < columnCount) {
                    System.out.print(", ");
                } else {
                    System.out.println();
                    break;
                }
            }
        }
    }

    private static void relToSql(RelNode rel) {
        SqlDialect dialect = CalciteSqlDialect.DEFAULT;
        RelToSqlConverter converter = new RelToSqlConverter(dialect);
        SqlNode sqlNode = converter.visitRoot(rel).asStatement();
        String generatedSql = sqlNode.toSqlString(dialect).getSql();
        System.out.println("\n=== Generated SQL ===");
        System.out.println(generatedSql);
    }
}
``` 

**输出结果：**    
```bash
LogicalFilter(condition=[>($1, 2)])
  LogicalAggregate(group=[{7}], C=[COUNT()], S=[SUM($5)])
    LogicalTableScan(table=[[scott, emp]])


=== Generated SQL ===
SELECT "DEPTNO", COUNT(*) AS "C", SUM("SAL") AS "S"
FROM "scott"."emp"
GROUP BY "DEPTNO"
HAVING COUNT(*) > 2

30, 6, 9400
20, 5, 10875
10, 3, 8750
```

## Push and pop     
The builder uses a stack to store the relational expression produced by one step and pass it as an input to the next step. This allows the methods that produce relational expressions to produce a builder.          

Most of the time, the only stack method you will use is build(), to get the last relational expression, namely the root of the tree.          

Sometimes the stack becomes so deeply nested it gets confusing. To keep things straight, you can remove expressions from the stack.             
（来自于官方文档）       

这段内容有点小抽象，push / pop 指的 stack 的入栈和出栈，stack 是一种先入后出（FILO）的数据结构，可以将数据入栈和出栈，入栈数据会在栈顶，出栈是将栈顶的数据取出。在原文中 `The builder uses a stack to store the relational expression produced by one step and pass it as an input to the next step.` 介绍 RelBuilder 是使用栈来存储每一步生成的关系表达式，并将其作为输入传递给下一步，哈哈，大伙，早在 YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project_filter_aggregate.java 示例中就有受到启发,你们看 filter() 方法，它在构造 Having COUNT(*) > 2时的形参顺序：>,COUNT(*),2。        
```java
.filter(
  builder.call(SqlStdOperatorTable.GREATER_THAN,
          builder.field("C"),
          builder.literal(2)))
```

两者的联动,如果用过 `波兰表达式算法`或者`逆波兰表达式算法`的同学，差不多就可以理解上面的话了，**存储每一步生成的关系表达式，并将其作为输入传递给下一步**。       

### 波兰表达式算法/逆波兰表达式算法      
>内容来自网上文章摘要。为了保证文章的完整性，添加了以下内容。           
```bash    
逆波兰表达式，英文为 Reverse Polish notation，跟波兰表达式（Polish notation）相对应。之所以叫波兰表达式和逆波兰表达式，是为了纪念波兰的数理科学家 Jan Łukasiewicz 其在著作中提到：
我在1924年突然有了一个无需括号的表达方法，我在文章第一次使用了这种表示法。      

* 平时我们习惯将表达式写成 (1 + 2) * (3 + 4)，加减乘除等运算符写在中间，因此称呼为中缀表达式。            
* 而波兰表达式的写法为 (* (+ 1 2) (+ 3 4))，将运算符写在前面，因而也称为前缀表达式。          
* 逆波兰表达式的写法为 ((1 2 +) (3 4 +) *)，将运算符写在后面，因而也称为后缀表达式。          

波兰表达式和逆波兰表达式有个好处，就算将圆括号去掉也没有歧义。上述的波兰表达式去掉圆括号，变为 * + 1 2 + 3 4。逆波兰表达式去掉圆括号，变成 1 2 + 3 4 + * 也是无歧义并可以计算的。事实上我们通常说的波兰表达式和逆波兰表达式就是去掉圆括号的。而中缀表达式，假如去掉圆括号，将 (1 + 2) * (3 + 4) 写成 1 + 2 * 3 + 4，就改变原来意思了。      

现实中，波兰表达式和逆波兰表达式，具体用于什么地方呢？
波兰表达式（前缀表达式），实际是抽象语法树的表示方式，比如中缀 (1 + 2) * (3 + 4) 编译时转成的抽象语法树为:              

     *
  /    \
 +      + 
/ \    / \
1  2  3   4 

这个操作符就是根节点，操作数为左右子节点。我们将这棵树用符号表达出来，可以写成 (* (+ 1 2) (+ 3 4))。这实际就是 Lisp 的 S-表达式。S-表达式可看成将整棵抽象语法树都写出来，每层节点都加上圆括号。           

至于逆波兰表示式，可用栈进行计算，天生适合于基于栈的语言。遇到数字就将数字压栈，遇到操作符，就将栈顶的两个元素取出计算，将计算结果再压入栈。          
```

**1.中缀表达式 演算示例**    
```bash
对 “2+3*4-2/2” 表达式来说，我们将数字压入栈，如果遇到 * 和 / 这两个优先级高的操作，我们将栈中的数据出栈，和操作符后面的数据进行 * 和 / 操作之后，将结果入栈，遇到 + 则不做操作，遇到 - 则对后一个数据进行取负，将数据压入栈即可。

就对 “2+3*4-2/2” 举例子：

轮询到第一个数字 2 压入栈中，当前栈 [2] 
操作符 +， 抛弃 
数字 3 压入栈，当前栈：[2, 3] 
操作符 *,将栈顶取出一个值 3，当前栈 [2] 
将下一个数字 4 和刚刚取出的值 3 做 * 操作得到 12，压入栈，目前栈：[2, 12] 
操作符 -, 对下一个数取负 
数字 2，取负后压入栈，目前栈 [2, 12, -2] 
操作符 /, 取出栈顶值 -2,目前栈 [2, 12] 
将下一个数字 2 和刚刚取出的值 -2 做 / 操作得到 1,压入栈

，目前栈 [2, 12, -1] 
最后对栈里的数据进行相加，得到 2+12-1 = 13 
```

**2.前缀表达式（波兰表达式）**           
2+3*4-2/2 转成前缀表达式后的结果： - + 2 * 3 4 / 2 2      
```bash
- + 2 * 3 4 / 2 2 演算过程：

轮询到第一个数字 2（最右边），压入栈中，当前栈 [2]
轮询到数字 2，压入栈中，当前栈 [2, 2]
操作符 /，取出栈顶值 2，再取出下一个值 2，做 2 / 2 操作得到 1，压入栈，目前栈 [1]
轮询到数字 4，压入栈中，当前栈 [1, 4]
轮询到数字 3，压入栈中，当前栈 [1, 4, 3]
操作符 *，取出栈顶值 3，再取出下一个值 4，做 3 * 4 操作得到 12，压入栈，目前栈 [1, 12]
轮询到数字 2，压入栈中，当前栈 [1, 12, 2]
操作符 +，取出栈顶值 2，再取出下一个值 12，做 2 + 12 操作得到 14，压入栈，目前栈 [1, 14]
操作符 -，取出栈顶值 14，再取出下一个值 1，做 14 - 1 操作得到 13，压入栈，目前栈 [13]

最终结果：栈中仅剩的 13。
```
  
### 栈存储每一步生成的关系表达式，并将其作为输入传递给下一步     
有了波兰表达式算法逻辑的理解，我想你对原文中这句话也理解的非常深刻了，`The builder uses a stack to store the relational expression produced by one step and pass it as an input to the next step.`;              

原文中`Most of the time, the only stack method you will use is build(), to get the last relational expression, namely the root of the tree.` 描述了通过 build() 方法获取最后一个 the last relational expression，也就是 the root of the tree，这点也很关键。 

### bushy join 示例    
**1.测试数据**          
![relbuilder05](http://img.xinzhuxiansheng.com/blogimgs/calcite/relbuilder05.png)     

1.1 consumers.csv  
```bash
id,firstname,lastname,birth,order_id
1,Zhang,San,1985-03-15,1001
2,Li,Si,1990-07-20,1002
3,Wang,Wu,1988-11-10,1004
4,Zhao,Liu,1995-02-28,
```

1.2 line_items.csv      
```bash
order_id,line_item_id,product_id,quantity,price
1001,1,101,1,999.99
1001,2,102,2,29.99
1001,3,103,1,79.99
1002,1,102,1,29.99
1002,2,105,2,2.00
1003,1,101,1,500.00
1004,1,103,1,79.99
1004,2,104,1,199.99
1004,3,102,2,29.99
```

1.3 orders.csv          
```bash
order_id,order_date,total_amount,consumer_id
1001,2024-01-15,1520.50,1
1002,2024-01-16,85.00,2
1003,2024-01-17,500.00,1
1004,2024-01-18,329.98,3
```

1.4 products.csv
```bash
product_id,product_name,price,category
101,Laptop,999.99,Electronics
102,Mouse,29.99,Electronics
103,Keyboard,79.99,Electronics
104,Chair,199.99,Furniture
105,USB Cable,2.00,Electronics
```

**2.YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project_bushyjoin.java**          
```java
public class YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project_bushyjoin {

    public static void main(String[] args) throws SQLException {
        Properties props = new Properties();
        props.setProperty("caseSensitive", "false");
        props.setProperty("lex", Lex.JAVA.toString());
        try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
             CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
            SchemaPlus rootSchema = calciteConnection.getRootSchema();
            File csvDir = new File("javamain-calcite\\src\\main\\resources\\scott_join");
            CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);

            SchemaPlus scottSchema = rootSchema.add("scott", new AbstractSchema());
            scottSchema.add("consumers", csvSchema.getTable("consumers"));
            scottSchema.add("line_items", csvSchema.getTable("line_items"));
            scottSchema.add("orders", csvSchema.getTable("orders"));
            scottSchema.add("products", csvSchema.getTable("products"));

            FrameworkConfig config = Frameworks.newConfigBuilder()
                    .parserConfig(
                            SqlParser.config()
                                    .withLex(Lex.JAVA)
                                    .withCaseSensitive(false)
                    )
                    .defaultSchema(rootSchema)
                    .build();
            RelBuilder builder = RelBuilder.create(config);

            final RelNode left = builder
                    .scan("scott","consumers")
                    .scan("scott","orders")
                    .join(JoinRelType.INNER, "order_id")
                    .build();

            final RelNode right = builder
                    .scan("scott","line_items")
                    .scan("scott","products")
                    .join(JoinRelType.INNER, "product_id")
                    .build();

            final RelNode rel = builder
                    .push(left)
                    .push(right)
                    .join(JoinRelType.INNER, "order_id")
                    .build();
            System.out.println(RelOptUtil.toString(rel));
            relToSql(rel);
            System.out.println();

            RelRunner runner = connection.unwrap(RelRunner.class);
            try (PreparedStatement preparedStatement = runner.prepareStatement(rel);
                 ResultSet resultSet = preparedStatement.executeQuery()) {
                print(resultSet);
            }
        }
    }

    private static void print(ResultSet resultSet) throws SQLException {
        final ResultSetMetaData metaData = resultSet.getMetaData();
        final int columnCount = metaData.getColumnCount();
        while (resultSet.next()) {
            for (int i = 1; ; i++) {
                System.out.print(resultSet.getString(i));
                if (i < columnCount) {
                    System.out.print(", ");
                } else {
                    System.out.println();
                    break;
                }
            }
        }
    }

    private static void relToSql(RelNode rel) {
        SqlDialect dialect = CalciteSqlDialect.DEFAULT;
        RelToSqlConverter converter = new RelToSqlConverter(dialect);
        SqlNode sqlNode = converter.visitRoot(rel).asStatement();
        String generatedSql = sqlNode.toSqlString(dialect).getSql();
        System.out.println("\n=== Generated SQL ===");
        System.out.println(generatedSql);
    }
}
```

**输出结果**            
```bash
LogicalJoin(condition=[=($4, $9)], joinType=[inner])
  LogicalJoin(condition=[=($4, $5)], joinType=[inner])
    LogicalTableScan(table=[[scott, consumers]])
    LogicalTableScan(table=[[scott, orders]])
  LogicalJoin(condition=[=($2, $5)], joinType=[inner])
    LogicalTableScan(table=[[scott, line_items]])
    LogicalTableScan(table=[[scott, products]])


=== Generated SQL ===
SELECT *
FROM "scott"."consumers"
INNER JOIN "scott"."orders" ON "consumers"."order_id" = "orders"."order_id"
INNER JOIN ("scott"."line_items" INNER JOIN "scott"."products" ON "line_items"."product_id" = "products"."product_id") ON "consumers"."order_id" = "line_items"."order_id"

1, Zhang, San, 1985-03-15, 1001, 1001, 2024-01-15, 1520.50, 1, 1001, 1, 101, 1, 999.99, 101, Laptop, 999.99, Electronics
1, Zhang, San, 1985-03-15, 1001, 1001, 2024-01-15, 1520.50, 1, 1001, 2, 102, 2, 29.99, 102, Mouse, 29.99, Electronics
1, Zhang, San, 1985-03-15, 1001, 1001, 2024-01-15, 1520.50, 1, 1001, 3, 103, 1, 79.99, 103, Keyboard, 79.99, Electronics
2, Li, Si, 1990-07-20, 1002, 1002, 2024-01-16, 85.00, 2, 1002, 1, 102, 1, 29.99, 102, Mouse, 29.99, Electronics
2, Li, Si, 1990-07-20, 1002, 1002, 2024-01-16, 85.00, 2, 1002, 2, 105, 2, 2.00, 105, USB Cable, 2.00, Electronics
3, Wang, Wu, 1988-11-10, 1004, 1004, 2024-01-18, 329.98, 3, 1004, 3, 102, 2, 29.99, 102, Mouse, 29.99, Electronics
3, Wang, Wu, 1988-11-10, 1004, 1004, 2024-01-18, 329.98, 3, 1004, 1, 103, 1, 79.99, 103, Keyboard, 79.99, Electronics
3, Wang, Wu, 1988-11-10, 1004, 1004, 2024-01-18, 329.98, 3, 1004, 2, 104, 1, 199.99, 104, Chair, 199.99, Furniture
```

### Switch Convention           
该章节可讨论的空间与它原文的篇幅大的多。 `Convention` 是第一次出现在我的 Calcite Blog 中，注意，它非常重要！我们可以通过上面的 `YzhouCsvTest_withoutjson_withrelbuilder_tablescan_project.java 示例` 来认识 Convention。 根据上面原文的介绍，RelBuilder 返回的 RelNode 是最后一个关系表达式，也就是树的根节点。它的 `RelTraitSet`集合中包含2个数据，一个是 `Convention`,另一个是 `RelCollation`。     

![relbuilder06](http://img.xinzhuxiansheng.com/blogimgs/calcite/relbuilder06.png)            

这特别让人一头雾水。其实 `RelTraitSet` 是 RelNode 的 `物理特征集合`，对应的书中的 `Physical Properties`，它描述这个 RelNode 以什么方式呈现数据，而不是这个 RelNode 做什么计算。在 ` Database System Concepts 7`书中 `22.7 Query Optimization for Parallel Execution` 章节有介绍:            
```bash
A more principled approach is to find the best plan, assuming that each operation is executed in parallel across all the nodes (operations with very small inputs may be executed on fewer nodes). Scheduling of independent operations in parallel on different nodes is not considered at this stage.                           

Partitioning of inputs and intermediate results is taken into consideration when estimating the cost of a query plan. Existing techniques for query optimization have been extended by considering partitioning as a physical property, in addition to physical properties such as sort orders that are already taken into account when choosing a sequential query plan. Just as sort operators are added to a query plan to get a desired sort order, exchange operators are added to get the desired partitioning property. The cost model used in practice is typically a resource consumption model, which we saw earlier. Although response-time cost models offer better estimates of query execution time, the cost of query optimization is higher when using a response-time cost model compared to the cost of optimization when using a resource-consumption cost model. References providing more information on the response-time cost model may be found in the
Further Reading section at the end of the chapter.
```

在 Calcite RelNode 中的 RelTraitSet Calcite 中有 3 种 RelTrait（关系表达式特征）：          
* Convention，它描述的是用什么方式执行计算，在 RelBuilder 返回的 RelNode，它的 Convention 默认是 NONE，仅代表是逻辑表达式，未绑定执行方式。             

还有其他的内置 Convention 如下所示：（但 Convention 是可以自定义），它更关注的是，计算在哪里发生，用什么技术实现。      
ENUMERABLE - 内存迭代执行（生成 Java 代码）                 
JDBC - 下推到数据库执行             
BINDABLE - 解释执行，不生成代码                     

* RelCollation，它描述的是数据以什么顺序进行物理排，示例：              
[]: 无序（随机顺序）            
[1 ASC]: 按第1列升序排列            
[1 ASC, 2 DESC]: 先按第1列升序，再按第2列降序           

* RelDistribution，它描述的数据在物理节点上的分布方式（分布式场景），关注点：数据在集群中的物理位置，示例：                    
SINGLETON: 所有数据在单个节点                   
HASH_DISTRIBUTED(col1): 按 col1 哈希分散到多节点                
BROADCAST: 每个节点都有完整副本         

下面是 Switch Convention 示例，你可以看到 sort 被转换成了`EnumerableSort`。              

**YzhouCsvTest_withoutjson_withrelbuilder_tablescan_convention.java**           
```java
public class YzhouCsvTest_withoutjson_withrelbuilder_tablescan_convention {

    public static void main(String[] args) throws SQLException {
        Properties props = new Properties();
        props.setProperty("caseSensitive", "false");
        props.setProperty("lex", Lex.JAVA.toString());
        try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
             CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
            SchemaPlus rootSchema = calciteConnection.getRootSchema();
            File csvDir = new File("javamain-calcite/src/main/resources/scott");
            CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);

            SchemaPlus scottSchema = rootSchema.add("scott", new AbstractSchema());
            scottSchema.add("dept", csvSchema.getTable("dept"));
            scottSchema.add("emp", csvSchema.getTable("emp"));

            FrameworkConfig config = Frameworks.newConfigBuilder()
                    .parserConfig(
                            SqlParser.config()
                                    .withLex(Lex.JAVA)
                                    .withCaseSensitive(false)
                    )
                    .defaultSchema(rootSchema)
                    .build();
            RelBuilder builder = RelBuilder.create(config);

            final RelNode rel = builder
                    .scan("scott", "emp")
                    .project(builder.field("ENAME"), builder.field("SAL"))
                    .adoptConvention(EnumerableConvention.INSTANCE)
                    .sort(builder.field("SAL"))
                    .build();
            System.out.println(RelOptUtil.toString(rel));
            relToSql(rel);
            System.out.println();

            RelRunner runner = connection.unwrap(RelRunner.class);
            try (PreparedStatement preparedStatement = runner.prepareStatement(rel);
                 ResultSet resultSet = preparedStatement.executeQuery()) {
                print(resultSet);
            }
        }
    }

    private static void print(ResultSet resultSet) throws SQLException {
        final ResultSetMetaData metaData = resultSet.getMetaData();
        final int columnCount = metaData.getColumnCount();
        while (resultSet.next()) {
            for (int i = 1; ; i++) {
                System.out.print(resultSet.getString(i));
                if (i < columnCount) {
                    System.out.print(", ");
                } else {
                    System.out.println();
                    break;
                }
            }
        }
    }

    private static void relToSql(RelNode rel) {
        SqlDialect dialect = CalciteSqlDialect.DEFAULT;
        RelToSqlConverter converter = new RelToSqlConverter(dialect);
        SqlNode sqlNode = converter.visitRoot(rel).asStatement();
        String generatedSql = sqlNode.toSqlString(dialect).getSql();
        System.out.println("\n=== Generated SQL ===");
        System.out.println(generatedSql);
    }
}
```

**输出的结果：**              
```bash
EnumerableSort(sort0=[$1], dir0=[ASC])
  LogicalProject(ENAME=[$1], SAL=[$5])
    LogicalTableScan(table=[[scott, emp]])


=== Generated SQL ===
SELECT "ENAME", "SAL"
FROM "scott"."emp"
ORDER BY "SAL"

ADAMS, 1100
WARD, 1250
MARTIN, 1250
MILLER, 1300
TURNER, 1500
ALLEN, 1600
CLARK, 2450
BLAKE, 2850
JONES, 2975
SCOTT, 3000
FORD, 3000
KING, 5000
SMITH, 800
JAMES, 950
```

大伙，Convention 觉不简单，在上面，我们介绍它描述的是用什么方式执行计算，Optimizer 的 VolcanoPlanner 使用的是  Volcano/Cascades 算法，它会根据 Convention（以及完整的 RelTraitSet）将等价的 RelNode 分组组织到不同的 RelSubset 中。每个 RelSubset 包含具有相同物理属性的等价关系表达式。           

要聊的地方可多了。但别陷进去了，我们的首要目标还是得先用起来。这非常重要 !!!     

## Field names and ordinals      
你可以通过名称或序号来引用一个字段。序号是从零开始的。每个运算符保证它输出字段出现的顺序。例如，`Project` 返回每个标量表达式生成的字段。            

运算符的字段名称需要保证是唯一的，但有时这也意味着，名称并不完全符合你的预期。例如，当你对 `EMP` 和 `DEPT` 进行关联时，其中一个输出字段会叫做 `DEPTNO`，而另一个输出字段则会叫做类似 `DEPTNO_1` 的名称。                

一些关系表达式方法让你能够更好地控制字段名称：              
* `project` 允许你使用 `alias(expr, fieldName)` 来包装表达式。它删除了包装器，但保留了建议的名称（只要它是唯一的）；
- `values(String[] fieldNames, Object... values)` 接受一个字段名称数组。如果数组中的任何元素为空，构建器将会生成一个唯一的名称；            

如果一个表达式投影成输入字段，或投影成输入字段的一个转换，那么它将使用输入字段的名称。一旦唯一的字段名称完成了分配，这些名称就是不可变的。如果你有一个特定的 `RelNode` 实例，你可以依赖字段名称的不变性。事实上，整个关系表达式也是不可变的。
但是，如果一个关系表达式已经通过了多个重写规则（参考 `RelOptRule`），结果表达式的字段名称可能看起来与原始表达式不太一样。这种情况下，最好按照序号来引用字段。

当你正在构建一个接受多个输入的关系表达式时，你需要考虑到那些点，从而构建字段引用。这在构建关联条件时经常出现。              

假设你正在 `EMP` 和 `DEPT` 上构建一个关联查询，`EMP` 有 8 个字段 `EMPNO`、`ENAME`、`JOB`、`MGR`、`HIREDATE`、`SAL`、`COMM`、`DEPTNO`; `DEPT` 有 3 个字段 `DEPTNO`、`DNAME`、`LOC`。在内部，Calcite 使用偏移量来表示这些字段，存储在一个包含 11 个字段的组合输入行中：左侧输入的第一个字段是 `#0`（请记住，序号从 0 开始），右侧输入的第一个字段是 `#8`。            

通过构建器 API，你可以指定哪个输入的哪个字段。要引用内部字段序号是 `#5` 的 `SAL`，可以写成 `builder.field(2, 0, "SAL")`，`builder.field(2, "EMP", "SAL")` 或 `builder.field(2, 0, 5)`。这个写法表示，在两个输入中，`#0` 输入的 `#5` 字段。为什么它需要知道有两个输入？因为它们存储在堆栈中，`#1` 输入位于堆栈顶部，`#0` 输入在其下方。如果我们不告诉构建器是两个输入，它不知道 `#0` 输入的深度。                        

类似地，要引用内部字段是 `#9 (8 + 1)` 的 `DNAME`，可以写成 `builder.field(2, 1, "DNAME")`，`builder.field(2, "DEPT", "DNAME")` 或 `builder.field(2, 1, 1)`。                        


refer       
1.https://calcite.apache.org/docs/algebra.html#algebra-builder        
2.https://github.com/apache/calcite/blob/main/core/src/test/java/org/apache/calcite/examples/RelBuilderExample.java       
3.https://zhuanlan.zhihu.com/p/410929199                
4.https://www.zhihu.com/question/41103160             