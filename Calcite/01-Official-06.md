# Calcite -   





这句话就目前文章的上下文来说`暂时有些无厘头`，它原文中的 `using mathematical identities that preserve semantics`, 


`Planner rules` 先暂时不做过多解释，会在后面的文章的介绍 `如何自定义规则`，这里希望大家别像我一样忽略一个单词 `expression`，准确的说是 `Relational-algebra expression`。   

并且它给出一个 `谓词下推`的示例。 




我该如何理解 Apache Calcite 中的 RelRoot, RelNode, RexNode   






## RelNode    

常见的 RelNode 类型：      
- TableScan - 扫描表     
- Filter - 过滤行     
- Project - 投影列      
- Join - 连接表      
- Aggregate - 聚合（GROUP BY）      
- Sort - 排序       

## RexNode 





RelBuilder.create(config) 方法传递的 config，它定义了 `SqlParser.config()`中设置了 `CaseSensitive` 为 false，也就是忽略大小写，可实际执行结果并不符合我的预期，这里确实有个`很大的疑惑?`  

**YzhouCsvTest_withoutjson_relbuilder.java**
```bash
public class YzhouCsvTest_withoutjson_relbuilder {

  public static void main(String[] args) throws SQLException {
    Properties props = new Properties();
    props.setProperty("caseSensitive", "false");
    props.setProperty("lex", Lex.JAVA.toString());
    try (Connection connection = DriverManager.getConnection("jdbc:calcite:", props);
         CalciteConnection calciteConnection = connection.unwrap(CalciteConnection.class)) {
      SchemaPlus rootSchema = calciteConnection.getRootSchema();
      File csvDir = new File("javamain-calcite\\src\\test\\resources\\sales");
      CsvSchema csvSchema = new CsvSchema(csvDir, CsvTable.Flavor.SCANNABLE);
      // 动态添加表到模式
      rootSchema.add("depts", csvSchema.getTable("DEPTS"));
      rootSchema.add("sdepts", csvSchema.getTable("SDEPTS"));

      FrameworkConfig config = Frameworks.newConfigBuilder()
              .parserConfig(SqlParser.config()
                              .withLex(Lex.JAVA)
                              .withCaseSensitive(false))
              .defaultSchema(rootSchema)
              .build();
      RelBuilder builder = RelBuilder.create(config);
      RelNode rel = builder
              .scan("depts")
              .project(builder.field("deptno"), builder.field("name"))  // 0=DEPTNO, 1=NAME
              .build();

      System.out.println("RelNode tree:");
      System.out.println(org.apache.calcite.plan.RelOptUtil.toString(rel));

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

输出结果： 
```bash
Exception in thread "main" java.lang.IllegalArgumentException: field [deptno] not found; input fields are: [DEPTNO, NAME]
	at org.apache.calcite.tools.RelBuilder.field(RelBuilder.java:539)
	at org.apache.calcite.tools.RelBuilder.field(RelBuilder.java:522)
	at com.javamain.calcite.csv.YzhouCsvTest_withoutjson_relbuilder.main(YzhouCsvTest_withoutjson_relbuilder.java:46)
```

**resources\sales\DEPTS.csv**
```bash
DEPTNO:int,NAME:string
10,"Sales"
20,"Marketing"
30,"Accounts"
```

 .project(builder.field(0), builder.field(1)) 

Calcite源码中 RelBuilderExample.java   


添加 Project 时，要说明 `.project(builder.field("DEPTNO"), builder.field("NAME"))` 与 `.project(builder.field("NAME"), builder.field("DEPTNO"))` 在打印 RelNode tree 区别，例如： 
```bash
# DEPTNO 在前，NAME 在后
[RelNode tree]
LogicalTableScan(table=[[depts]])

# NAME 在前，DEPTNO 在后
[RelNode tree]
LogicalProject(NAME=[$1], DEPTNO=[$0])
  LogicalTableScan(table=[[depts]])
```

列裁剪（Project 下沉）需要知道“父节点/更上层到底用哪些列”。比如根节点只输出列 A，那就应尽量让下游节点只产出 A；这个信息是自顶向下的，单看某个节点及其子节点无法判断哪些列可以安全丢弃。






refer     
1.https://github.com/HamaWhiteGG/flink-sql-lineage       
2.https://github.com/apache/calcite      
