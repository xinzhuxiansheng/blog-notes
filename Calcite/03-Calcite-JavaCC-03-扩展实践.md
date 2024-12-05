# Calcite - JavaCC - 扩展实践     

>javacc version: 7.0.13          

在之前 javacc example 介绍过程中，你应该对`JavaCC Grammar`有了一些了解，下面再做一些扩展实践        

## 示例01 - 解析 Select 1 + 1 查询语句      
需求：解析一条select 1+1查询语句。这里不会限定只能是1+1，数字可以改变。我们把它加起来并输出结果，select 1+1输出2，select 2+3输出5，select 1-1输出0。     

按照示例输入格式，首先要解析的是 `SELECT 固定字符串`(注意，此处可以忽略大小写)，那后续就是递归匹配 1 + 1 .....  等等。     
项目结构如下：    
![javaccpractice09](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice09.png)    

下面是 `SimpleSelectParser.jj`    
```bash
options {
    IGNORE_CASE = true;
    STATIC = false;
    OUTPUT_DIRECTORY = "../../src/main/java/extension/examples1";
}

PARSER_BEGIN(SimpleSelectParser)

package extension.examples1;

public class SimpleSelectParser {
    private String sql;

    public void parse() throws ParseException {
        SelectExpr(sql);
    }
    public SimpleSelectParser(String expr) {
        this((Reader)(new StringReader(expr)));
        this.sql = expr;
    }
    public static void main(String[] args) throws Exception{
        final SimpleSelectParser parser = new SimpleSelectParser(String.join(" ", args));
        parser.parse();
    }
}
PARSER_END(SimpleSelectParser)

/*
跳过的制表符
* */
SKIP :
{
    " "
|   "\t"
|   "\n"
|   "\r"
|   "\f"
}

TOKEN :
{
    < SELECT: "SELECT" >
|   < NUMBER: (["0"-"9"])+ >
|   < ADD: "+" >
|   < SUB: "-" >
}

void SelectExpr(String sql) :
{
    int res;
}
{
    <SELECT>
    res = Expression()
    {
        System.out.println(sql + "=" + res);
    }
}

int Expression() :
{
    int res = 0;
    int v;
}
{
    res = Number()
    (
        <ADD>
        v = Number()
        {res += v;}
    |
        <SUB>
        v = Number()
        {res -= v;}
    )*
    {return res;}
}

int Number() :
{
    Token t;
}
{
    t = <NUMBER>
    {
        return Integer.parseInt(t.image);
    }
}
```

>大伙看到`SimpleSelectParser.jj` 比较长的时候，我们可以利用 IDEA 的折叠快捷键来折叠 jj代码，只要我们心里谨记`JavaCC 的 File structure`,会有一定程度的宏观理解。 折叠示例如下：          
![javaccpractice08](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice08.png)    

### 分析  
>该篇 Blog 并不会对所有关键字做解析，在之前 Blog`Calcite - JavaCC - 官网 Example 入门实践`介绍过的，该篇就不过多解释了。      

#### options            
```bash
options {
    IGNORE_CASE = true;
    STATIC = false;
    OUTPUT_DIRECTORY = "../../src/main/java/extension/examples1";
}
```

* IGNORE_CASE = true; 匹配解析规则的时候`忽略大小写`,例如 select、Select 等等。   

* STATIC = false; 大家可以看`PARSER_BEGIN`部分，它包含一个`sql`变量，这部分就是 java 代码知识了，我们不希望 sql 是一个静态变量，每次实例化 SimpleSelectParser对象都创建一个 sql变量。      

#### PARSER DECLARATION   
`PARSER_BEGIN`与`PARSER_END`这部分完全是 java code，这部分`头一次`出现的是:             
```java
this((Reader)(new StringReader(expr)));
```
在 javacc中为了便利内容输入方式，会提供多样化的构造方法，这里不需要太多深究，可参考 https://javacc.github.io/javacc/tutorials/charstream.html

#### TOKEN          
需求是 select 加减表达式，所以它的 token 分别为 `SELECT`,`NUMBER`,`ADD`,`SUB`,以及他们的匹配规则。       

#### BNF SelectExpr() & BNF Expression() & BNF Number()       
![javaccpractice10](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice10.png)

首先在 SelectExpr() `匹配 SELECT`， 当满足条件时，继续`匹配数字`, 注意 Number()是解析 SQL 获取的 `NUMBER TOKEN`，并返回值(`t.image`);     


## 示例02 - Simple SQL Parser    
需求：解析 `Select a.id,a.name from a where a.id = 1` 查询语句, 得到 SQL 中的 查询列名，表名，where 条件。        

项目结构如下：  
![javaccpractice11](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice11.png)  

### 思路过程  
示例中的 SQL固定的几个关键字 SELECT、FROM、WHERE，那整体的 SQL 解析如下：  
```
SELECT ...
FROM ...
WHERE ...   
```

**示例 jj 文件内容:**    
```bash
options {
    IGNORE_CASE = true;
    STATIC = false;
    OUTPUT_DIRECTORY = "../../src/main/java/extension/examples2";
}

PARSER_BEGIN(SimpleSqlParser)
package extension.examples2;
public class SimpleSqlParser {

    public void parse() throws ParseException {
         SqlAttr sqlAttr = SFWStatement();
         System.out.println("OK");
    }
    public SimpleSqlParser(String expr) {
        this((Reader)(new StringReader(expr)));
    }
    public static void main(String[] args) throws Exception{
        final SimpleSqlParser parser = new SimpleSqlParser(String.join(" ", args));
        parser.parse();
    }
}
PARSER_END(SimpleSqlParser)

/*
跳过的制表符
* */
SKIP :
{
    " "
|   "\t"
|   "\n"
|   "\r"
}

TOKEN :
{
  < SELECT : "SELECT" >
| < FROM : "FROM" >
| < WHERE : "WHERE" >
| < AND : "AND" >
}
TOKEN : {< OPERATOR : ">" | "< " | "=" | ">=" | "<=" | "<>" | "!=" >}
TOKEN : {< NAME : ([ "a"-"z", "0"-"9" ])+ >}
TOKEN : {< DOT : "." >}
TOKEN : {< COMMA : "," >}
TOKEN : {< QUO : "" >}



SqlAttr SFWStatement() :
{
    SqlAttr sqlAttr = new SqlAttr();
    String tbName = "";
}
{
  SelectClause(sqlAttr.getSelectColumnList())
  tbName = FromClause()
  WhereClause(sqlAttr.getWhereColumList())
  {
    sqlAttr.setTableName(tbName);
    return sqlAttr;
  }
}

void SelectClause(List<Column> columnList) :
{
}
{
  < SELECT > Attr(columnList)
}

void Attr(List<Column> columnList) :
{
  Token relation;
  Token attr;
}
{
  relation = < NAME > < DOT > attr = < NAME >
  (
    < COMMA > Attr(columnList)
  )*
  {
    Column column =  new Column(relation.image, attr.image);
    columnList.add(column);
  }
}

String FromClause() :
{
  String from;
}
{
  < FROM > from = TableName()
  {
    return from;
  }
}

String TableName() :
{
  Token t;
}
{
  t = < NAME >
  {return t.image;}
}

void WhereClause(List<KV> kvList) :
{
}
{
  < WHERE > Expression(kvList)
}

void Expression(List<KV> whereColumList) :
{
}
{
   Factor(whereColumList)
   (
     < AND > Expression(whereColumList)
   )*
}

void Factor(List<KV> whereColumList) :
{
  Column left;
  String right = "";
  String operator;
}
{
  left = LeftAttr()
  operator = Operator()
  right = RightAttr()
  {
    KV kv = new KV();
    kv.setKey(left);
    kv.setOperator(operator);
    kv.setValue(right);
    whereColumList.add(kv);
  }
}

Column LeftAttr() :
{
  Token rel;
  Token attr;
}
{
  rel = < NAME > < DOT > attr = < NAME >
  {
    return new Column(rel.image, attr.image);
  }
| < QUO > rel = < NAME > < QUO >
  {
    return new Column(rel.image);
  }
}

String RightAttr() :
{
  Token rel;
}
{
  rel = < NAME >
  {return rel.image;}
}

String Operator() :
{
  Token operator;
}
{
  operator = < OPERATOR >
  {return operator.image;}
}
```


* 首先，jj 中的 options参数设置忽略大小写，STATIC 设置为 false 以及 OUTPUT_DIRECTORY 参数:          
```bash
options {
    IGNORE_CASE = true;
    STATIC = false;
    OUTPUT_DIRECTORY = "../../src/main/java/extension/examples2";
}
``` 

* PARSER_BEGIN\PARSER_END 部分，通过 main() 方法传入 SQL，调用解析方法，得到 `SqlAttr`对象。            
```java
PARSER_BEGIN(SimpleSqlParser)
package extension.examples2;
public class SimpleSqlParser {

    public void parse() throws ParseException {
         SqlAttr sqlAttr = SFWStatement();
         System.out.println("OK");
    }
    public SimpleSqlParser(String expr) {
        this((Reader)(new StringReader(expr)));
    }
    public static void main(String[] args) throws Exception{
        final SimpleSqlParser parser = new SimpleSqlParser(String.join(" ", args));
        parser.parse();
    }
}
PARSER_END(SimpleSqlParser)
```

* 因为输入SQL 中可能包含一些与解析无关的标记，所以我们需要跳过这些          
```bash   
SKIP :
{
    " "
|   "\t"
|   "\n"
|   "\r"
}
```   

* TOKEN 的定义，需要匹配的是 SELECT、FROM、WHERE、AND, (针对一些出现的内容都需要定义)     
```bash
TOKEN :
{
  < SELECT : "SELECT" >
| < FROM : "FROM" >
| < WHERE : "WHERE" >
| < AND : "AND" >
}
TOKEN : {< OPERATOR : ">" | "< " | "=" | ">=" | "<=" | "<>" | "!=" >}     # 匹配 where 条件的操作符 
TOKEN : {< NAME : ([ "a"-"z", "0"-"9" ])+ >}  # 匹配 列名 和 where 条件
TOKEN : {< DOT : "." >}     # 匹配 列名（a.id）
TOKEN : {< COMMA : "," >} 
TOKEN : {< QUO : "" >}
```

* BNF 语法部分，定义了 SqlAttr 对象 和 tbName, 通过`SelectClause(sqlAttr.getSelectColumnList())`、`tbName = FromClause()`、`WhereClause(sqlAttr.getWhereColumList())` 解析 SQL 中的3个部分
```bash
SqlAttr SFWStatement() :
{
    SqlAttr sqlAttr = new SqlAttr();
    String tbName = "";
}
{
  SelectClause(sqlAttr.getSelectColumnList())
  tbName = FromClause()
  WhereClause(sqlAttr.getWhereColumList())
  {
    sqlAttr.setTableName(tbName);
    return sqlAttr;
  }
}
```

3个不同的方法，他们都各自先匹配特定SQL 关键字：     
```bash
# 匹配 SELECT 
void SelectClause(List<Column> columnList) :
{
}
{
  < SELECT > Attr(columnList)
}


# 匹配 FROM
String FromClause() :
{
  String from;
}
{
  < FROM > from = TableName()
  {
    return from;
  }
}

# 匹配 WHERE   
void WhereClause(List<KV> kvList) :
{
}
{
  < WHERE > Expression(kvList)
}
```   

后续的方法调用，其实与 Java 代码编写思路都差不多，这里就不过多介绍了，  下面将 SqlAttr、Column、KV 列举出来：           

**SqlAttr**         
```java
package extension.examples2;

import java.util.ArrayList;
import java.util.List;

public class SqlAttr {
    private String tableName;
    private List<Column> selectColumnList;
    private List<KV> whereColumList;

    public SqlAttr() {
        this.selectColumnList = new ArrayList<>();
        this.whereColumList = new ArrayList<>();
    }

    public String getTableName() {
        return tableName;
    }

    public void setTableName(String tableName) {
        this.tableName = tableName;
    }

    public List<Column> getSelectColumnList() {
        return selectColumnList;
    }

    public void setSelectColumnList(List<Column> selectColumnList) {
        this.selectColumnList = selectColumnList;
    }

    public List<KV> getWhereColumList() {
        return whereColumList;
    }

    public void setWhereColumList(List<KV> whereColumList) {
        this.whereColumList = whereColumList;
    }
}
```

**Column**      
```java
package extension.examples2;

public class Column {
    private String tableName;
    private String fieldName;

    public Column(){}

    public Column(String fieldName){
        this.fieldName = fieldName;
    }

    public Column(String tableName, String fieldName) {
        this.tableName = tableName;
        this.fieldName = fieldName;
    }

    public String getTableName() {
        return tableName;
    }

    public void setTableName(String tableName) {
        this.tableName = tableName;
    }

    public String getFieldName() {
        return fieldName;
    }

    public void setFieldName(String fieldName) {
        this.fieldName = fieldName;
    }
}
```  

**KV**
```java
package extension.examples2;

public class KV {
    private Column key;
    private String operator;
    private String value;

    public KV() {
        this.key = new Column();
    }

    public Column getKey() {
        return key;
    }

    public void setKey(Column key) {
        this.key = key;
    }

    public String getValue() {
        return value;
    }

    public void setValue(String value) {
        this.value = value;
    }

    public String getOperator() {
        return operator;
    }

    public void setOperator(String operator) {
        this.operator = operator;
    }
}
```

得到 SqlAttr 对象的结果如下图：          
![javaccpractice12](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice12.png)      


## 总结   
若把 JavaCC 与 正则匹配做比较，那 JavaCC 做一些解析功能的开发确实比 正则表达式简单多了。在实践过程中，总是自上而下的思考解析的部分,从左到右顺序。      


refer   
1.《Calcite 数据管理实战》 - 6.3.2 JavaCC简单示例               
2.https://github.com/gtxistxgao/SQLParser                       

