# Calcite - 实践 Calcite SqlParser，SqlValidator，自定义 UDF     

## 引言      
在之前发表的关于 Calcite 的微信公众号文章中，**文章列表如下所示**，这些文章的大部分篇幅都在介绍如何通过实现 Calcite 的 `自定义表元数据` 来实践 SQL 查询数据的功能。因此，`元数据的定义` 就像一个 `驱动器`，桥接了 Calcite SQL 与存储介质之间的关系 ... 但就目前实现结果来看，博主仍然可以思考一些不足点。    

![ddl01](http://img.xinzhuxiansheng.com/blogimgs/calcite/ddl01.png)      

文章列表：      
* "Calcite - 探索让集合可以像表一样查询" (https://mp.weixin.qq.com/s/Zy8SOCSCRRIBaLjVG4AEKQ)    
* "Calcite - Official - Tutorial CSV 案例说明补充"(https://mp.weixin.qq.com/s/YdKV4V4ToRy0h42brg4PSg)        
* "Calcite - 探索 Kafka Eagle KSQL 数据查询实现" (https://mp.weixin.qq.com/s/55QMA0EI-Lbr3PSj60mQtg)      

>后面的章节是以公众号文章 "Calcite - Official - Tutorial CSV 案例说明补充"(`https://mp.weixin.qq.com/s/YdKV4V4ToRy0h42brg4PSg`)中的 `YzhouCsvTest` 示例代码为基础，进行优化。遗忘或者未阅读过的小伙伴，建议重新阅读文章。      

## 不足点    

>这里的不足点偏向于在实践过程中还可以继续思考，继续补充的地方。       

### 1. 去掉 model.json 文件引用     
在公众号文章 "Calcite - Official - Tutorial CSV 案例说明补充"(`https://mp.weixin.qq.com/s/YdKV4V4ToRy0h42brg4PSg`) 介绍了使用 Calcite 查询 CSV 数据，但 `YzhouCsvTest` 示例仅是一个 Demo，因为我们很少会通过 `model.json` 文件来定义元数据，这种声明方式在使用过程中非常不方便。所以，我们使用 `RootSchema` 添加元数据，去掉 `model.json` 文件的引用。代码如下所示：           

**YzhouCsvTest_withoutjson.java**  
```java
public class YzhouCsvTest_withoutjson {

  public static void main(String[] args) throws SQLException {
    Properties props = new Properties();
    props.setProperty("caseSensitive", "false");
    props.setProperty("lex", "JAVA");
    try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
         CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
      SchemaPlus rootSchema = calciteConnection.getRootSchema();
      File csvDir = new File("javamain-calcite\\src\\main\\resources\\sales");
      CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);
      // 动态添加表到模式
      rootSchema.add("depts", csvSchema.getTable("DEPTS"));
      rootSchema.add("sdepts", csvSchema.getTable("SDEPTS"));

      String sql = "select deptno,name from depts";
      try (Statement statement = connection.createStatement();
           ResultSet resultSet = statement.executeQuery(sql)) {
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

执行结果：   
```bash
10, Sales
20, Marketing
30, Accounts
```

### 2. 通过 DDL 对元数据操作   
像 model.json 文件定义元数据 或者自定义表元数据，在大多数时候比较偏 `静态`，当程序启动时就决定了元数据的结构信息，这也导致了后期无法对元数据信息做扩展。 例如增加表，视图等操作，都需要重新启动程序。大家可以参考像 Flink SQL Cli 中的 `Catalog`，我们可以像在 MySQL 那样，通过 DDL 语句对元数据进行操作。       

**DdlExample.java**
```java
public class DdlExample {
  public static void main(String[] args) throws SQLException {
    // 1. 配置属性，指定支持 DDL 的解析器
    Properties props = new Properties();
    props.setProperty("lex", "JAVA");
    props.put(
            CalciteConnectionProperty.PARSER_FACTORY.camelName(),
            ServerDdlExecutor.class.getName() + "#PARSER_FACTORY"
    );

    // 2. 创建连接
    try (Connection conn = DriverManager.getConnection("jdbc:calcite:", props);
         Statement stmt = conn.createStatement()) {

      // === 用 SQL 来定义元数据结构 ===
      stmt.execute("CREATE SCHEMA sales");
      stmt.execute("CREATE TABLE sales.depts(deptno int, name varchar(20))");
      stmt.execute("INSERT INTO sales.depts VALUES (1, 'IT'), (2, 'HR')");

      // === 查询数据 ===
      try (ResultSet rs = stmt.executeQuery("SELECT * FROM sales.depts")) {
        print(rs);
      }
    }
  }

  private static void print(ResultSet rs) throws SQLException {
    ResultSetMetaData meta = rs.getMetaData();
    int cols = meta.getColumnCount();
    while (rs.next()) {
      for (int i = 1; i <= cols; i++) {
        System.out.print(rs.getString(i));
        if (i < cols) System.out.print(", ");
      }
      System.out.println();
    }
  }
}
```

执行结果：      
```bash
1, IT
2, HR  
```

### 3. 添加 UDF，校验 SQL & 执行 UDF                                                                                              
Apache Calcite 学习成本对于博主来说是比较高的，这里面包括的原因是它的官方文档比较简洁、学习书籍比较少，网上资料偏少，我目前只找到 《Calcite 数据管理实战》 这本书，其实最让我心累的是，不管是 Google 出来或者 AI 告诉我的概念、API 方法等，我并不能在 Calcite 官方文档和《Calcite 数据管理实战》中检索到它们，所以获取信息的手段比较少。无法体系的了解它们，更多的是去`试错` API， 摸索出来实现细节。      

有的时候，我们调用 API，SDK 或者某些方法, 那真的就是在处理业务逻辑，我们关注的是形参是啥，有没有某种逻辑判断语句会跳过这些 API，SDK 或者某些方法，我们偏少的机会会遇到方法的触发会涉及 `方法的生命周期`，意思是，别人已经帮我们规定好了，哪些方法是在前，或者在后，等等才能执行。下面展示了 Calcite 流程图，博主有了少许的 Calcite开发经验后发现，程序调用的 Calcite API 不能光是调用，它也同样具备`生命周期`，**什么阶段执行什么方法似的**。  
![ddl05](http://img.xinzhuxiansheng.com/blogimgs/calcite/ddl05.png)     

>但别想太多，当下是将程序跑起来再说 ......          

现在我们可以举一个示例，**自定义 UDF**          
我们在构造 Calcite connection 时会传入一个 Properties 对象，它可以通过设置 `lex` 来控制 SQL 的词法规则，这样我们编写的 SQL 就可以灵活的适配多个数据库引擎。    
```java
Properties info = new Properties();
info.setProperty("lex", "JAVA");
Connection connection = DriverManager.getConnection("jdbc:calcite:", info);
```

在大数据领域中自定义 UDF 在很多数据库引擎中都是很常见的功能，Calcite 亦是如此，但与我的理解又有所不同。它定义的UDF 的方式不同，而 UDF执行的生命周期也有所不同，（博主花了不少时间试错）。       

#### 3.1 UDF 介绍  
在 `YzhouCsvTest_withoutjson` 示例代码中，执行 `select deptno,name from depts` SQL 查询 csv 文件数据， 现在我需要添加一个 UDF 函数 `addStr`, 它支持字符串类型形参，将传入的数据，添加一个固定字符串后缀，例如：name 为 "yzhou", addStr(name) 为 "yzhou_addStr"。     

>以上面的 `YzhouCsvTest_withoutjson` 为基础，添加 UDF “功能”。

#### 3.1 通过 SqlFunction 定义 UDF，SqlOperatorTable 方式注册 
>我知道下面的做法是有问题的，但实践可以帮助我们总结经验。     

**代码结构如下**     
![ddl02](http://img.xinzhuxiansheng.com/blogimgs/calcite/ddl02.png)    

`UAddStr` 继承 `SqlFunction`，而 `SqlFunction` 继承抽象类 `SqlOperator`，因为它们的内部字段都是用 `final`定义，所以 `UAddStr.java` 需要在构造方法强制给它们赋值，如下所示，UAddStr 的形参具体含义除了 `SqlOperandTypeInference`,目前博主还没有使用到，其他参数的含义如下所示，具体参数描述可查看 `SqlOperator`的注释：        
* String name: UDF 函数名称    
* @Nullable SqlReturnTypeInference returnTypeInference：UDF 函数返回值类型，注意此时的类型是 Calcite DB 类型，例如字符串指的是 VARCHAR     
* @Nullable SqlOperandTypeInference operandTypeInference：先忽略，暂时没有用到    
* @Nullable SqlOperandTypeChecker operandTypeChecker：形参类型，注意此时的类型是 Calcite JAVA 类型，例如字符串指的是 JAVA   
* SqlFunctionCategory category： UDF 函数分类   

**UAddStr 类图**  
![ddl03](http://img.xinzhuxiansheng.com/blogimgs/calcite/ddl03.png)  

**UAddStr.java**
```java
public class UAddStr extends SqlFunction {

  public UAddStr(String name, SqlKind kind, @Nullable SqlReturnTypeInference returnTypeInference, @Nullable SqlOperandTypeInference operandTypeInference, @Nullable SqlOperandTypeChecker operandTypeChecker, SqlFunctionCategory category) {
    super(name, kind, returnTypeInference, operandTypeInference, operandTypeChecker, category);
  }
}
```

`ExtendedSqlOperatorTable` 继承 抽象类 `ReflectiveSqlOperatorTable`, 而 `ReflectiveSqlOperatorTable` 实现 `SqlOperatorTable` 接口， 在 `ExtendedSqlOperatorTable` 有几个比较重要的核心点：  
* SqlOperatorTable: 是用来定义查找 SQL 算子（SqlOperator）和函数的接口       
* ReflectiveSqlOperatorTable: `它是什么？暂时搁置`, 首先它的派生类 `SqlStdOperatorTable` 是一个曝光量非常高的类，经常用于 SQL 解析和校验阶段，简单来说，Calcite 在分析 SQL 语句时需要知道各种操作符（AND，OR，LIKE, COUNT 等等）的语义和功能，而这些操作符的标准定义就集中在 `SqlStdOperatorTable`。 这可以帮助我们理解 ReflectiveSqlOperatorTable    
* 我们需要在它内部定义 addStr() 方法，并且需要在 instance() 注册 addStr() 方法        
* instance() 方法是什么鬼？ `ExtendedSqlOperatorTable` 继承 抽象类 `ReflectiveSqlOperatorTable`时，并没有提示需要创建 instance() 方法。 而我怎么知晓的呢？  是 Google 的结果，我看到别人是这么用，了解到 `SqlStdOperatorTable` 类也是这么封装的（包括 instance 字段， instance() 方法） 。   

>上面逻辑，博主解释不够专业，建议大家把代码跑起来后，再带入思考。   

**ExtendedSqlOperatorTable 类图** 
![ddl04](http://img.xinzhuxiansheng.com/blogimgs/calcite/ddl04.png)   

**ExtendedSqlOperatorTable.java**    
```bash
public class ExtendedSqlOperatorTable extends ReflectiveSqlOperatorTable {
  private static ExtendedSqlOperatorTable instance;

  public static synchronized ExtendedSqlOperatorTable instance() {
    if (instance == null) {
      instance = new ExtendedSqlOperatorTable();
      instance.init();
    }
    instance.register(addStr());
    return instance;
  }

  private static SqlFunction addStr() {
    return new UAddStr("addStr",
            SqlKind.OTHER_FUNCTION,
            ReturnTypes.VARCHAR,
            null,
            OperandTypes.STRING,
            SqlFunctionCategory.USER_DEFINED_FUNCTION);
  }
}
```

我在 `YzhouCsvTest_withoutjson.java` 示例基础上中添加了 `validateSql()` 方法用于 **解析，校验 SQL 功能**，在业务开发中，我们肯定不希望 SQL 在执行后再报错，希望它在运行前就完成了初步的 "校验" 功能， `YzhouCsvTest_withvalidator_withsqlfunction` 会先校验 SQL，再执行 SQL 读取 CSV数据。这里需要特别注意的是 `SqlValidatorUtil.newValidator()` 方法的第一个形参，这里使用的是我们定义的 `ExtendedSqlOperatorTable.instance()`, 若没有定义 UDF 情况下，`SqlStdOperatorTable.instance()` 是足够使用的。      

>Tips：YzhouCsvTest_withvalidator_withsqlfunction.java 示例中的 `parser.parseQuery()` 方法解析 SQL 时， 并不会对 SQL 的字段，表名，UDF 做校验，你可以将校验部分代码注释掉，列举错误的SQL (`select deptno02,addstr(name) from depts02`) 对 parseQuery() 进行测试,你会发现可以正常打印出解析后的 SqlNode。      

>这部分可参考 《Calcite 数据管理实战》的 7.3 章节介绍了代码中的核心类的解释说明。（凑合看）   

**YzhouCsvTest_withvalidator_withsqlfunction.java**
```java
public class YzhouCsvTest_withvalidator_withsqlfunction {

  public static void main(String[] args) throws SQLException {
    Properties props = new Properties();
    props.setProperty("caseSensitive", "false");
    props.setProperty("lex", Lex.JAVA.toString());
    try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
         CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
      SchemaPlus rootSchema = calciteConnection.getRootSchema();
      File csvDir = new File("javamain-calcite\\src\\main\\resources\\sales");
      CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);
      // 动态添加表到模式
      rootSchema.add("depts", csvSchema.getTable("DEPTS"));
      rootSchema.add("sdepts", csvSchema.getTable("SDEPTS"));


      String sql = "select deptno,addstr(name) from depts";

      // 校验 SQL
      validateSql(props,sql,rootSchema);

      try (Statement statement = connection.createStatement();
           ResultSet resultSet = statement.executeQuery(sql)) {
        print(resultSet);
      }
    } catch (SqlParseException e) {
      throw new RuntimeException(e);
    }
  }

  /**
   * 校验 SQL
   */
  private static void validateSql(Properties props,String sql, SchemaPlus rootSchema) throws SqlParseException { 
    // 解析 SQL
    SqlParser.Config config = SqlParser.configBuilder()
            .setCaseSensitive(false)
            .setLex(Lex.JAVA)
            .build();
    SqlParser parser = SqlParser.create(sql, config);
    SqlNode sqlNodeParsed = parser.parseQuery();
    System.out.println("[parsed sqlNode]");
    System.out.println(sqlNodeParsed);

    // 校验 SQL
    JavaTypeFactoryImpl sqlTypeFactory = new JavaTypeFactoryImpl();
    CalciteCatalogReader catalogReader = new CalciteCatalogReader(
            CalciteSchema.from(rootSchema),
            CalciteSchema.from(rootSchema).path(null),
            sqlTypeFactory,
            new CalciteConnectionConfigImpl(props));
    SqlValidator validator = SqlValidatorUtil.newValidator(
            ExtendedSqlOperatorTable.instance(),
            catalogReader,
            sqlTypeFactory,
            SqlValidator.Config.DEFAULT);
    // validate: SqlNode -> SqlNode
    SqlNode sqlNodeValidated = validator.validate(sqlNodeParsed);
    System.out.println();
    System.out.println("[validated sqlNode]");
    System.out.println(sqlNodeValidated);
  }

  /**
   * 打印数据结果
   */
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

看似还挺符合逻辑，但的确是有问题的。细心的读者应该发现，我们还没有定义 `addStr` UDF 函数的执行逻辑，那我们先说执行结果，运行报错，异常信息如下所示, 发现解析，校验 SQL 是成功的，但是最后在 查询数据时报错，说匹配不到 `addstr` 方法。 `但为什么又能解析和校验通过呢?`    

这是因为 SqlFunction 直接定义的 UDF 负责定义语法、参数类型、校验规则，但不涉及到真正的执行。若需要在执行阶段运行 UDF，需要使用 `SqlUserDefinedFunction 或 自定义执行器`。  

```bash
[parsed sqlNode]
SELECT `deptno`, `addstr`(`name`)
FROM `depts`

[validated sqlNode]
SELECT `deptno`, ADDSTR(`name`)
FROM `depts`
Exception in thread "main" java.sql.SQLException: Error while executing SQL "select deptno,addstr(name) from depts": From line 1, column 15 to line 1, column 26: No match found for function signature addstr(<CHARACTER>)
	at org.apache.calcite.avatica.Helper.createException(Helper.java:56)
	at org.apache.calcite.avatica.Helper.createException(Helper.java:41)
	at org.apache.calcite.avatica.AvaticaStatement.executeInternal(AvaticaStatement.java:164)
	at org.apache.calcite.avatica.AvaticaStatement.executeQuery(AvaticaStatement.java:228)
	at com.javamain.calcite.csv03.withsqlfunction.YzhouCsvTest_withvalidator_withsqlfunction.main(YzhouCsvTest_withvalidator_withsqlfunction.java:45)
Caused by: org.apache.calcite.runtime.CalciteContextException: From line 1, column 15 to line 1, column 26: No match found for function signature addstr(<CHARACTER>)
	at java.base/jdk.internal.reflect.NativeConstructorAccessorImpl.newInstance0(Native Method)
	at java.base/jdk.internal.reflect.NativeConstructorAccessorImpl.newInstance(NativeConstructorAccessorImpl.java:62)
	at java.base/jdk.internal.reflect.DelegatingConstructorAccessorImpl.newInstance(DelegatingConstructorAccessorImpl.java:45)
	at java.base/java.lang.reflect.Constructor.newInstance(Constructor.java:490)
	at org.apache.calcite.runtime.Resources$ExInstWithCause.ex(Resources.java:505)
	at org.apache.calcite.sql.SqlUtil.newContextException(SqlUtil.java:948)
	at org.apache.calcite.sql.SqlUtil.newContextException(SqlUtil.java:933)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.newValidationError(SqlValidatorImpl.java:5469)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.handleUnresolvedFunction(SqlValidatorImpl.java:2002)
	at org.apache.calcite.sql.SqlFunction.deriveType(SqlFunction.java:340)
	at org.apache.calcite.sql.SqlFunction.deriveType(SqlFunction.java:231)
	at org.apache.calcite.sql.validate.SqlValidatorImpl$DeriveTypeVisitor.visit(SqlValidatorImpl.java:6534)
	at org.apache.calcite.sql.validate.SqlValidatorImpl$DeriveTypeVisitor.visit(SqlValidatorImpl.java:6521)
	at org.apache.calcite.sql.SqlCall.accept(SqlCall.java:166)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.deriveTypeImpl(SqlValidatorImpl.java:1916)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.deriveType(SqlValidatorImpl.java:1901)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.expandSelectItem(SqlValidatorImpl.java:484)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.validateSelectList(SqlValidatorImpl.java:4624)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.validateSelect(SqlValidatorImpl.java:3806)
	at org.apache.calcite.sql.validate.SelectNamespace.validateImpl(SelectNamespace.java:61)
	at org.apache.calcite.sql.validate.AbstractNamespace.validate(AbstractNamespace.java:88)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.validateNamespace(SqlValidatorImpl.java:1144)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.validateQuery(SqlValidatorImpl.java:1115)
	at org.apache.calcite.sql.SqlSelect.validate(SqlSelect.java:282)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.validateScopedExpression(SqlValidatorImpl.java:1090)
	at org.apache.calcite.sql.validate.SqlValidatorImpl.validate(SqlValidatorImpl.java:796)
	at org.apache.calcite.sql2rel.SqlToRelConverter.convertQuery(SqlToRelConverter.java:598)
	at org.apache.calcite.prepare.Prepare.prepareSql(Prepare.java:257)
	at org.apache.calcite.prepare.Prepare.prepareSql(Prepare.java:220)
	at org.apache.calcite.prepare.CalcitePrepareImpl.prepare2_(CalcitePrepareImpl.java:665)
	at org.apache.calcite.prepare.CalcitePrepareImpl.prepare_(CalcitePrepareImpl.java:519)
	at org.apache.calcite.prepare.CalcitePrepareImpl.prepareSql(CalcitePrepareImpl.java:487)
	at org.apache.calcite.jdbc.CalciteConnectionImpl.parseQuery(CalciteConnectionImpl.java:236)
	at org.apache.calcite.jdbc.CalciteMetaImpl.prepareAndExecute(CalciteMetaImpl.java:630)
	at org.apache.calcite.avatica.AvaticaConnection.prepareAndExecuteInternal(AvaticaConnection.java:677)
	at org.apache.calcite.avatica.AvaticaStatement.executeInternal(AvaticaStatement.java:157)
	... 2 more
Caused by: org.apache.calcite.sql.validate.SqlValidatorException: No match found for function signature addstr(<CHARACTER>)
	at java.base/jdk.internal.reflect.NativeConstructorAccessorImpl.newInstance0(Native Method)
	at java.base/jdk.internal.reflect.NativeConstructorAccessorImpl.newInstance(NativeConstructorAccessorImpl.java:62)
	at java.base/jdk.internal.reflect.DelegatingConstructorAccessorImpl.newInstance(DelegatingConstructorAccessorImpl.java:45)
	at java.base/java.lang.reflect.Constructor.newInstance(Constructor.java:490)
	at org.apache.calcite.runtime.Resources$ExInstWithCause.ex(Resources.java:505)
	at org.apache.calcite.runtime.Resources$ExInst.ex(Resources.java:599)
	... 33 more
```

#### 3.2 通过 Schema 注册 UDF    
`UAddStr`类中定义了一个名为 `addStr`的方法并且包含它的执行逻辑，`YzhouCsvTest_withvalidator_withschema` 类调用 `rootSchema.add()` 方法接收两个参数，第一个参数是函数名称，第二个参数是利用ScalarFunctionImpl类的create方法创建Linq4j当中的Function，因此需要传入具体的类路径和该类中的方法。 利用 SchemaPlus 类，我们可以把 Schema 理解成元数据信息，我们可以往元数据中添加函数、数据库、表等。因此我们只要把函数注册到 Schema 当中即可。    

当把 `validateSql(props,sql,rootSchema);` 代码注释掉，不打印校验结果时，通过打印数据的结果来看， addstr() udf 函数的确起了作用。  
`但`把 validateSql(props,sql,rootSchema); 去掉注释，先打印校验结果时，我们会遇到以下异常信息,该问题我们在上面章节遇到过，当时理解是 SqlFunction UDF并不会在`执行阶段`运行。看来通过 Schema 注册的 UDF 也同样遇到这样的问题。      

>这事因为：    
在执行阶段，Calcite 的 SQL 引擎（RelRoot、Interpreter 等）会查找 schema 里的函数；    
在校验阶段，SqlValidatorUtil.newValidator(...) 默认只认识标准函数库 SqlStdOperatorTable。     

```bash
Caused by: org.apache.calcite.sql.validate.SqlValidatorException: No match found for function signature addstr(<CHARACTER>)
	at java.base/jdk.internal.reflect.NativeConstructorAccessorImpl.newInstance0(Native Method)
	at java.base/jdk.internal.reflect.NativeConstructorAccessorImpl.newInstance(NativeConstructorAccessorImpl.java:62)
	at java.base/jdk.internal.reflect.DelegatingConstructorAccessorImpl.newInstance(DelegatingConstructorAccessorImpl.java:45)
	at java.base/java.lang.reflect.Constructor.newInstance(Constructor.java:490)
	at org.apache.calcite.runtime.Resources$ExInstWithCause.ex(Resources.java:505)
	at org.apache.calcite.runtime.Resources$ExInst.ex(Resources.java:599)
	... 23 more
```

**UAddStr.java**  
```java
public class UAddStr {
  public static String addStr(String str) {
    return str + "_addStr";
  }
}
```  

**YzhouCsvTest_withvalidator_withschema.java**
```java
public class YzhouCsvTest_withvalidator_withschema {

  public static void main(String[] args) throws SQLException {
    Properties props = new Properties();
    props.setProperty("caseSensitive", "false");
    props.setProperty("lex", Lex.JAVA.toString());
    try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
         CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
      SchemaPlus rootSchema = calciteConnection.getRootSchema();
      File csvDir = new File("javamain-calcite\\src\\main\\resources\\sales");
      CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);
      // 动态添加表到模式
      rootSchema.add("depts", csvSchema.getTable("DEPTS"));
      rootSchema.add("sdepts", csvSchema.getTable("SDEPTS"));

      // 注册 UDF 方法 
      rootSchema.add("addstr", ScalarFunctionImpl.create(UAddStr.class, "addStr"));

      String sql = "select deptno,addstr(name) from depts";

      // 校验 SQL
      // validateSql(props,sql,rootSchema);

      try (Statement statement = connection.createStatement();
           ResultSet resultSet = statement.executeQuery(sql)) {
        print(resultSet);
      }
    }
  }

  /**
   * 校验 SQL
   */
  private static void validateSql(Properties props,String sql, SchemaPlus rootSchema) throws SqlParseException {
    SqlParser.Config config = SqlParser.configBuilder()
            .setCaseSensitive(false)
            .setLex(Lex.JAVA)
            .build();
    SqlParser parser = SqlParser.create(sql, config);
    SqlNode sqlNodeParsed = parser.parseQuery();
    System.out.println("[parsed sqlNode]");
    System.out.println(sqlNodeParsed);

    JavaTypeFactoryImpl sqlTypeFactory = new JavaTypeFactoryImpl();
    CalciteCatalogReader catalogReader = new CalciteCatalogReader(
            CalciteSchema.from(rootSchema),
            CalciteSchema.from(rootSchema).path(null),
            sqlTypeFactory,
            new CalciteConnectionConfigImpl(props));
    SqlValidator validator = SqlValidatorUtil.newValidator(
            SqlStdOperatorTable.instance(),
            catalogReader,
            sqlTypeFactory,
            SqlValidator.Config.DEFAULT);
    // validate: SqlNode -> SqlNode
    SqlNode sqlNodeValidated = validator.validate(sqlNodeParsed);
    System.out.println();
    System.out.println("[validated sqlNode]");
    System.out.println(sqlNodeValidated);
  }

  /**
   * 打印数据结果
   */
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

输出结果:       
```bash
10, Sales_addStr
20, Marketing_addStr
30, Accounts_addStr
```

## UDF 到底怎么注册    
这里的确让我引发思考，SqlFucntion 定义，Schema 定义。 各自展现的形式不同。 差异是什么？ Calcite 在解析 SQL，校验 SQL，执行 SQL 等，应该会有一个 Context吧。 那这又是什么呢 ？          
![ddl06](http://img.xinzhuxiansheng.com/blogimgs/calcite/ddl06.png)  

咱们可以试试发个邮件问问老外，试试有人回复么？   

继续探索 ......