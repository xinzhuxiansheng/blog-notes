# Calcite - 探索 Kafka Eagle KSQL 数据查询        

>Kafka Eagle version: v3.0.1   

## 背景       
LONG LONG AGO，我在开发 Kafka Cluster 管理平台时候，有个开源产品`Kafka Eagle`,它也提供了 Kafka Cluster 管理的功能，但在我心中`yahoo/CMAK`仍然是 top1, 但`Kafka Eagle`有个功能让我有些意外是`KSQL`,注意这并非`confluent ksql`。      

>注意：无特殊说明情况下，`KSQL` 代表的是 Kafka Eagle KSQL 功能，不是 confluent ksql。     

下面是我基于`v3.0.1`版本搭建的 Kafka Eagle `KSQL`功能的交互截图：        
![efak01](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak01.png)     

如果大家想要更好的了解 KSQL,可访问`Kafka Eagle`官网 `https://docs.kafka-eagle.org/3.quickstart/7.ksql`。             

开头引入`Kafka Eagle KSQL`，是因为它是由`Calcite`实现的。对于`Flink SQL`原理，博主也正在探索中，KSQL 相对于 Flink SQL相比，还是比较容易实现和理解的，这对于我们积累对`Calcite`了解再好不过了。            

>注意：该篇 Blog 并不会介绍`Kafka Eagle 服务搭建`，这块要是有任何问题，欢迎给我留言一起讨论。     

## 了解 KSQL      
让我们先从这张图开始：      
![efak02](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak02.png)        

## 了解支持输入的查询语句  
首先通过 KSQL 文档`https://docs.kafka-eagle.org/3.quickstart/7.ksql`，了解到它支持查询：       
* SELECT * ....   

* Parse JSONObject Or JSONArrays  

* Filter Query [where ... and]    

## KSQL 处理逻辑 
浏览器 F12 查看 KSQL 的查询请求接口 `/topic/logical/commit` GET (http://192.168.0.201:8048/topic/logical/commit/?sql=select * from yzhoutpjson01 where `partition` in (0) limit 10&jobId=job_id_1734760020607)：          
![efak03](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak03.png)       

有了调用入口，可以很快梳理到 `KafkaSqlParser#execute()`。    
![efak12](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak12.png)      

**KafkaSqlParser#execute()**   
```java
public static String execute(String clusterAlias, String sql) {
    JSONObject status = new JSONObject();
    try {
        // 01 解析 SQL
        KafkaSqlInfo kafkaSql = kafkaService.parseSql(clusterAlias, sql); 
        LOG.info("KafkaSqlParser - SQL[" + kafkaSql.getSql() + "]");
        if (kafkaSql.isStatus()) {
            if (!hasTopic(clusterAlias, kafkaSql)) {
                status.put("error", true);
                status.put("msg", "ERROR - Topic[" + kafkaSql.getTableName() + "] not exist.");
            } else {
                long start = System.currentTimeMillis();
                kafkaSql.setClusterAlias(clusterAlias);
                
                // 02 读取数据
                List<JSONArray> dataSets = KafkaConsumerAdapter.executor(kafkaSql);
                String results = "";
                int rows = 0;
                if (dataSets.size() > 0 && !dataSets.get(dataSets.size() - 1).isEmpty()) {

                    // 03 Calcite SQL 查询   
                    JSONObject object = KSqlUtils.query(kafkaSql.getSchema(), kafkaSql.getTableName(), dataSets, kafkaSql.getSql());
                    results = object.getString("result");
                    rows = object.getInteger("size");
                } else {
                    List<Map<String, Object>> schemas = new ArrayList<Map<String, Object>>();
                    Map<String, Object> map = new HashMap<>();
                    map.put("NULL", "");
                    schemas.add(map);
                    results = new Gson().toJson(schemas);
                }
                long end = System.currentTimeMillis();
                status.put("error", false);
                status.put("msg", results);
                status.put("status", "Time taken: " + (end - start) / 1000.0 + " seconds, Fetched: " + rows + " row(s)");
                status.put("spent", end - start);
            }
        } else {
            status.put("error", true);
            status.put("msg", "ERROR - SQL[" + kafkaSql.getSql() + "] has error,please start with select.");
        }
    } catch (Exception e) {
        status.put("error", true);
        status.put("msg", e.getMessage());
        e.printStackTrace();
        LOG.error("Execute sql to query kafka topic has error,msg is " + e.getMessage());
    }
    return status.toJSONString();
}
```

### kafkaService.parseSql() 解析 SQL   
下面是解析 SQL 的方法调用链路图：     
![](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak11.png)   

`kafkaService.parseSql()`方法其内部调用`KafkaServiceImpl#segments()`方法，使用 Calcite的 SqlParser类将 SQL 解析成 SqlNode，内部嵌套递归调用，将不同的 SQL 部分解析后赋值给 `TopicPartitionSchema tps`对象。 

**org.smartloli.kafka.eagle.common.constant.KSqlParser#parserTopic()**     
```java
/**
 * Parser sql mapper kafka tree.
 */
public static TopicPartitionSchema parserTopic(String sql) {
    TopicPartitionSchema tps = new TopicPartitionSchema();
    try {
        SqlParser.Config config = SqlParser.config().withLex(Lex.JAVA);
        SqlParser sqlParser = SqlParser.create(sql, config);
        SqlNode sqlNode = sqlParser.parseStmt();
        parseNode(sqlNode, tps);
    } catch (Exception e) {
        LoggerUtils.print(KSqlParser.class).error("Parser kafka sql has error, msg is ", e);
    }
    return tps;
}
```

这部分的 API 是`calcite-core` jar `SqlParser`提供的方法，这里使用 JavaCC 解析器对相关的 SQL 语句进行解析。在 Calcite中，默认采用 JavaCC 来生成词法分析器和语法分析器，并且 `JavaCC` 的依赖已经被封装到 `calcite-core`模块中。  

下面为了验证这部分，可以写一个 Calcite 使用 JavaCC 解析器的 Demo：    
* 添加 `calcite-core`依赖 
```xml
<dependency>
    <groupId>org.apache.calcite</groupId>
    <artifactId>calcite-core</artifactId>
    <version>1.35.0</version>
</dependency>
```

* 
```java
package com.javamain.calcite.playground;

import org.apache.calcite.config.Lex;
import org.apache.calcite.sql.SqlNode;
import org.apache.calcite.sql.parser.SqlParseException;
import org.apache.calcite.sql.parser.SqlParser;

public class UseJavaCCParserSQL {
    public static void main(String[] args) throws SqlParseException {
        System.out.println("\r\n... mysqlParser ...");
        mysqlParser();

        System.out.println("\r\n... ksqlParser ...");
        ksqlParser();

    }

    private static void mysqlParser() throws SqlParseException {
        // SQL语句
        String sql = "select * from t_user where id = 1 limit 10";
        // 解析配置SqlParse
        SqlParser.Config mysqlConfig = SqlParser.config().withLex(Lex.MYSQL);

        // 创建解析器
        SqlParser parser = SqlParser.create(sql, mysqlConfig);

        // 解析SQL语句
        SqlNode sqlNode = parser.parseQuery();
        System.out.println(sqlNode.toString());
    }

    private static void ksqlParser() throws SqlParseException {
        // SQL语句
        String sql = "select * from yzhoutpjson01 where `partition` in (0) limit 10";
        // 解析配置SqlParse
        SqlParser.Config mysqlConfig = SqlParser.config().withLex(Lex.JAVA);

        // 创建解析器
        SqlParser parser = SqlParser.create(sql, mysqlConfig);

        // 解析SQL语句
        SqlNode sqlNode = parser.parseQuery();
        System.out.println(sqlNode.toString());
    }
}
```

Output log:     
```bash
... mysqlParser ...
SELECT *
FROM `t_user`
WHERE `id` = 1
FETCH NEXT 10 ROWS ONLY

... ksqlParser ...
SELECT *
FROM `yzhoutpjson01`
WHERE `partition` IN (0)
FETCH NEXT 10 ROWS ONLY
```

Calcite 将 SQL 解析成几个重要部分，`SEELCT`、`FROM`、`WHERE`、`FETCH`。 针对 FETCH 是对 limit 的转换。   

注意，此时 Demo 是使用 toString() 打印出来，而实际使用中，我们是希望通过 Calcite 将 SQL 解析成一个抽象语法树`AST`。  

有了上面的理解，我们再回到`KSqlParser#parserTopic()`方法中来，它调用的是`sqlParser.parseStmt()`, 而不是 Demo 中的`parser.parseQuery()`，**我想这部分调用过 JAVA JDBC API 的同学并不陌生**，parserQuery()是专门用于解析 SQL查询语句（即 SEELCT 语句），而 parseStmt() 是用于解析一个 SQL 语句的通用方法，不光支持 SELECT，还可以 INSERT、UPDATE 等操作。    

在`KSqlParser#parserTopic()`的父方法栈`KafkaServiceImpl#segments()`中，已经对 SQL 语句做了 SELECT 判断，代码如下:            
```java    
省略部分代码...
if (!sql.startsWith("select") && !sql.startsWith("SELECT")) {
    kafkaSql.setStatus(false);
    return kafkaSql;
}
省略部分代码...
```

我们再来看`KSqlParser#parserTopic()`方法的返回值`TopicPartitionSchema tps`,它在`KafkaServiceImpl#segments()`方法中，将解析SQL 后得到的值赋值给`KafkaSqlInfo kafkaSql`, 代码如下：     
```java
TopicPartitionSchema tps = KSqlParser.parserTopic(sql);
if (tps != null && !"".equals(tps.getTopic())) {
    kafkaSql.setStatus(true);
    kafkaSql.setTableName(tps.getTopic());
    kafkaSql.setSeeds(getBrokers(clusterAlias));
    kafkaSql.setPartition(tps.getPartitions());
    kafkaSql.setLimit(tps.getLimit());
}
```
* tps.getTopic()    
* tps.getPartitions()  
* tps.getLimit()   

知道了 SQL 解析需要得到的值，那么在继续看`KSqlParser#parserTopic()`方法是如何解析抽象语法树 AST。        
![efak04](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak04.png)         

**org.smartloli.kafka.eagle.common.constant.KSqlParser#parseNode()**      
```java
private static void parseNode(SqlNode sqlNode, TopicPartitionSchema tps) {
    SqlKind sqlKind = sqlNode.getKind();
    switch (sqlKind) {
        case SELECT:
            String topic = "";
            SqlNode sqlFrom = ((SqlSelect) sqlNode).getFrom();
            SqlNode sqlWhere = ((SqlSelect) sqlNode).getWhere();
            if (sqlFrom.getKind() == SqlKind.IDENTIFIER) {
                topic = sqlFrom.toString();
            } else {
                SqlBasicCall sqlBasicCall = (SqlBasicCall) sqlFrom;
                if (sqlBasicCall.getKind() == SqlKind.AS && sqlBasicCall.operandCount() > 0) {
                    topic = sqlBasicCall.operand(0).toString();
                }
            }
            if (sqlWhere.getKind() == SqlKind.IN) {// one and
                SqlBasicCall sqlBasicCall = (SqlBasicCall) sqlWhere;
                if (sqlBasicCall.operandCount() > 1) {
                    String[] partitions = sqlBasicCall.operand(1).toString().split(",");
                    tps.getTopicSchema().put(topic, StrUtils.stringsConvertIntegers(partitions));
                    tps.setTopic(topic);
                    tps.setPartitions(StrUtils.stringsConvertIntegers(partitions));
                }
            } else if (sqlWhere.getKind() == SqlKind.AND) {// two and
                SqlBasicCall sqlBasicCall = (SqlBasicCall) sqlWhere;
                if (sqlBasicCall.operandCount() > 0) {
                    SqlNode sqlNodeChild = sqlBasicCall.operand(0);
                    if (sqlNodeChild.getKind() == SqlKind.IN) {
                        SqlBasicCall sqlBasicCallChild = (SqlBasicCall) sqlNodeChild;
                        if (sqlBasicCallChild.operandCount() > 1) {
                            String[] partitions = sqlBasicCallChild.operand(1).toString().split(",");
                            tps.getTopicSchema().put(topic, StrUtils.stringsConvertIntegers(partitions));
                            tps.setTopic(topic);
                            tps.setPartitions(StrUtils.stringsConvertIntegers(partitions));
                        }
                    } else if (sqlNodeChild.getKind() == SqlKind.AND) {
                        SqlNode sqlBasicCallChild = ((SqlBasicCall) sqlNodeChild).operand(0);
                        if (sqlBasicCallChild.getKind() == SqlKind.IN) {
                            SqlBasicCall sqlBasicCallGrandson = (SqlBasicCall) sqlBasicCallChild;
                            if (sqlBasicCallGrandson.operandCount() > 1) {
                                String[] partitions = sqlBasicCallGrandson.operand(1).toString().split(",");
                                tps.getTopicSchema().put(topic, StrUtils.stringsConvertIntegers(partitions));
                                tps.setTopic(topic);
                                tps.setPartitions(StrUtils.stringsConvertIntegers(partitions));
                            }
                        }
                    }
                }
            }
            break;
        case JOIN:
            SqlNode leftNode = ((SqlJoin) sqlNode).getLeft();
            SqlNode rightNode = ((SqlJoin) sqlNode).getRight();
            if (leftNode.getKind() == SqlKind.IDENTIFIER) {
                // tps.add(leftNode.toString());
            } else {
                parseNode(leftNode, tps);
            }
            if (rightNode.getKind() == SqlKind.IDENTIFIER) {
                // tps.add(rightNode.toString());
            } else {
                parseNode(rightNode, tps);
            }
            break;
        case UNION:
            SqlNode unionLeft = ((SqlBasicCall) sqlNode).operand(0);
            SqlNode unionRight = ((SqlBasicCall) sqlNode).operand(1);
            if (unionLeft.getKind() == SqlKind.IDENTIFIER) {
                // tps.add(unionLeft.toString());
            } else {
                parseNode(unionLeft, tps);
            }
            if (unionRight.getKind() == SqlKind.IDENTIFIER) {
                // tps.add(unionRight.toString());
            } else {
                parseNode(unionRight, tps);
            }
            break;
        case ORDER_BY:
            SqlOrderBy sqlOrderBy = (SqlOrderBy) sqlNode;
            if (!StrUtils.isNull(sqlOrderBy.fetch.toString())) {
                long limit = 0L;
                try {
                    limit = Long.parseLong(sqlOrderBy.fetch.toString());
                } catch (Exception e) {
                    LoggerUtils.print(KSqlParser.class).error("Parser limit string to long has error, msg is ", e);
                }
                tps.setLimit(limit);
            }
            parseNode(sqlOrderBy.query, tps);
        default:
            break;

    }
}
```      

>如何解析？    
`org.smartloli.kafka.eagle.common.constant.KSqlParser#parseNode()`方法并没有较复杂的处理逻辑，并且`case JOIN`、`case UNION`并不在 KSQL 的语法说明里，暂时不讨论这两个 case。下面给大家介绍`Calcite中的解析体系`。    

## 《Calcite 数据管理实战》 6.2 章节     
>关于这部分介绍，博主直接粘贴`Calcite 数据管理实战`的`Calcite中的解析体系`章节，这样也能避免因为我的失误，误导大家。        

### 6.2 Calcite中的解析体系  
6.1节主要讲了比较通用的语法解析原理，对于不同场景，语法解析过程会有不同的实现方式。对于数据管理系统，一般来说，这种语法解析主要针对的是将SQL语句解析成抽象语法树的过程，本节将会对Calcite中这个过程的实现方式进行介绍。   

#### 6.2.1 抽象语法树的概念
如前文所述，语法解析的最终结果是一棵抽象语法树，那么什么是抽象语法树呢？在计算机科学 中，抽象语法树是代码 结构的一种抽象表示。它以树状的形式表现出 语法结构，树上的每个节点都表示源码中的一种结构。
图6-2展示了抽象语法树的一个简单示例。我们如果给计算机输入的指令是“(1+2)*3”，那么经过语法解析以后就会生成抽象语法树，其中圆形节点表示叶子节点，一般是参数，方形节点表示非叶子节点，一般是操作符。当然，实际生成的抽象语法树要复杂得多，每个节点会存储许多必要的信息。抽象语法树将纯文本转换为一棵树，其中每个节点对应代码中的一种结构，例如上述的表达式转换为源码中的结构会变成图6-2（b）所示的形式。          
![efak05](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak05.png)    

同理，我们输入的一条SQL语句也会生成一棵抽象语法树，例如`select id from table where id > 1`。图6-3展示了该SQL语句生成的抽象语法树。      
![efak06](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak06.png)      

图6-3中，这棵树的每个节点仅仅是对语法的抽象，并未对应到相应的源码结构当中。因此为了能够匹配每个节点相应的源码结构，Calcite构建了它的SqlNode体系来完成这项任务。       

#### 6.2.2 SqlNode体系
SqlNode是负责封装语义信息的基础类，是Calcite中非常重要的概念，不只是解析阶段，也和后续的校验、优化息息相关，它是所有解析节点的父类。在Calcite中SqlNode的实现类有40多个，每个类都代表一个节点到源码结构的映射，其大致可以分为3类，即`SqlLiteral`、`SqlIdentifier`、`SqlCall`。图6-4展示了SqlNode及其子类体系。本小节将主要对SqlNode比较重要的几个实现类进行介绍。          

![efak07](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak07.png)   

##### 1．SqlLiteral
SqlLiteral类主要封装输入的常量，也被称作字面量。它和它的子类一般用来封装具体的变量值，同样我们也可以通过调用getValue方法返回我们所需要的值。为了实现其通用性，Calcite支持了很多数据类型，表6-1展示了当前版本SqlLiteral可以表示的常量类型        
 
![efak08](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak08.png)      

##### 2．SqlIdentifier
SqlIdentifier代表我们输入的标识符，例如在一条SQL语句中表的名称、字段名称，都可以封装成一个SqlIdentifier对象。      

##### 3．SqlCall  
图6-5展示了SqlCall及其子类的继承结构。每一个操作都可以对应一个SqlCall，如查询是SqlSelect，插入是SqlInsert。     
为了更加细粒度地介绍Calcite是如何使用SqlCall的子类来封装操作的，我们以负责查询的SqlSelect为例，介绍SqlCall内部具体是如何封装操作的。具体的实现方式如代码清单6-1所示       

![efak09](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak09.png)    

**代码清单6-1　SqlSelect中包含的属性以及常量**      
```java
/**
 * 
封装查询操作的SqlSelect节点 */
public class SqlSelect extends SqlCall {   
  public static final int FROM_OPERAND = 2;   
  public static final int WHERE_OPERAND = 3;   
  public static final int HAVING_OPERAND = 5;   
  SqlNodeList keywordList;   
  // 查询字段列表   
  @Nullable SqlNodeList
  selectList;   
  // 数据源信息   
  @Nullable SqlNode from;   
  // 过滤条件信息   
  @Nullable SqlNode where;   
  // 分组信息   
  @Nullable SqlNodeList groupBy;   
  @Nullable SqlNode having;
  SqlNodeList windowDecls;   
  @Nullable SqlNodeList orderBy;   
  @Nullable SqlNode offset;
  @Nullable SqlNode fetch;   
  @Nullable SqlNodeList hints;
``` 

通过观察SqlSelect的成员变量，可以很明显地发现在SqlSelect当中封装了数据源信息（FROM子句）、过滤条件信息（WHERE子句）、分组信息（GROUP BY子句）等查询信息。也就是说，当SQL语句是一条查询语句的时候，会生成一个SqlSelect节点，在这个节点下面封装了SQL语句当中每一个关键的参数。        

同理，在负责插入数据的SqlInsert中，不难发现该类封装了相应的信息。代码清单6-2展示了在SqlInsert中封装了目标表信息（targetTable）、源信息（source）、字段对应信息（columnList），基本上将插入数据时需要的信息都囊括了进来。          

**代码清单6-2　SqlInsert中包含的属性以及常量**    
```java
public class SqlInsert extends SqlCall {
    public static final SqlSpecialOperator OPERATOR =        
      new SqlSpecialOperator("INSERT",SqlKind.INSERT);    
    SqlNodeList keywords;    
    SqlNode targetTable;    
    SqlNode source;    
    @Nullable SqlNodeList columnList;
```
那么SqlNode中的各个类是如何工作的呢？我们举一个例子，如代码清单6-3所示，这是一条简单的SQL语句，其中包含字段的投影（id）、数据源的制定（t）、查询过滤条件（id>1）以及分组条件（id）。        

**代码清单6-3　SqlNode工作方式示例SQL语句**
```sql
select
    id
from t
where 
id > 1
```   

Calcite的SqlNode规范化，最终形成SqlNode树。图6-6展示了经过规约的SqlNode数据结构。       
![efak10](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak10.png)   

有了`Calcite的 SqlNode`体系的了解，再回看`KSqlParser#parseNode()`方法里面很多 class 就显得并不陌生，例如`SqlBasicCall`、`SqlSelect`、`SqlKind`。    

`KSqlParser#parseNode()`方法是学习解析 SqlNode的模板。         

## KafkaConsumerAdapter.executor() 查询数据       
![efak13](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak13.png)        

有了 `KafkaSqlInfo kafkaSql`对象，它包含 topic、partition、broker，这些信息已足够我们查询 Kafka 数据，`KafkaConsumerAdapter.executor()`方法其本身就是调用`kafka-client` API 构造 consumer，它可指定分区以及 offset进行读取消息, 将这部分数据存储到`List<JSONArray> dataSets`对象中。    

![efak14](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak14.png)  

注意：该部分如果不熟悉的话，可写一些查询 demo，例如 指定分区消费，指定 offset 消费，如何查询消息的最新位点等功能。(如果这块有什么问题，可留言告诉我)     

## KSqlUtils#query() 使用 SQL 检索数据      
消费 kafka Topic 的数据集存储在`List<JSONArray> dataSets`对象中，下面是`KSqlUtils#query()` 代码。               
**org.smartloli.kafka.eagle.core.sql.tool.KSqlUtils#query()**  
```java
/**
 * @param tabSchema : Table column,such as {"id":"integer","name":"varchar"}
 * @param tableName : Defining table names for query datasets, such as "user"
 * @param dataSets  : DataSets ,such as
 *                  [{"id":1,"name":"aaa"},{"id":2,"name":"bbb"},{}...]
 * @param sql       : such as "SELECT * FROM TBL"
 * @return String
 * @throws Exception : Throws an exception
 */
public static JSONObject query(JSONObject tabSchema, String tableName, List<JSONArray> dataSets, String sql) throws Exception {
    JSONObject queryResults = new JSONObject();
    String model = createTempJson();
    List<List<String>> list = new LinkedList<>();
    for (JSONArray dataSet : dataSets) {
        for (Object obj : dataSet) {
            JSONObject object = (JSONObject) obj;
            List<String> tmp = new LinkedList<>();
            for (String key : object.keySet()) {
                tmp.add(UnicodeUtils.encodeForUnicode(object.getString(key)));
            }
            list.add(tmp);
        }
    }
    JSqlMapData.loadSchema(tabSchema, tableName, list);

    Class.forName(JConstants.KAFKA_DRIVER);
    Properties info = new Properties();
    info.setProperty("lex", Lex.JAVA.toString());

    Connection connection = DriverManager.getConnection(Driver.CONNECT_STRING_PREFIX + "model=inline:" + model, info);
    Statement st = connection.createStatement();
    ResultSet result = st.executeQuery(UnicodeUtils.encodeForUnicode(sql));
    ResultSetMetaData rsmd = result.getMetaData();
    List<Map<String, Object>> ret = new ArrayList<>();
    while (result.next()) {
        Map<String, Object> map = new LinkedHashMap<>();
        for (int i = 1; i <= rsmd.getColumnCount(); i++) {
            map.put(rsmd.getColumnName(i), UnicodeUtils.decodeForUnicode(result.getString(rsmd.getColumnName(i))));
        }
        ret.add(map);
    }
    result.close();
    st.close();
    connection.close();
    queryResults.put("result", new Gson().toJson(ret));
    queryResults.put("size", ret.size());
    return queryResults;
}
```  

我们来看下`KSqlUtils#query()`方法的入参,如下图：   
![efak15](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak15.png)     

* JSONObject tabSchema:      
partition:integer   
offset:bigint  
msg:varchar  
timespan:varchar 
date:varchar    

* String tableName: 
topic 名称  

* List<JSONArray> dataSets:   
消费 kafka 的数据集   

* String sql:  
查询 SQL     

>我们刚了解了入参，为了避免下面内容对于概念不太了解的同学，建议可阅读`《Calcite 数据管理实战》 7.2 章节 元数据` 和 `adapter`(https://calcite.apache.org/docs/adapter.html)。我们需要提前了解`数据模型定义`、`自定义表元数据实现`, 不过很庆幸的是《Calcite 数据管理实战》都有说明。       

开始之前，必须知晓 KSQL 使用了 自定义表元数据实现 & 函数定义 两个特性。接下来，我们就开始深入了解。       

1.String model = createTempJson(); 定义数据模型，JSON 如下：    
```json
{
  "defaultSchema": "db",
  "schemas": [
    {
      "factory": "org.smartloli.kafka.eagle.core.sql.schema.JSqlSchemaFactory",
      "name": "db",
      "type": "custom"
    }
  ],
  "version": "1.30.0"
}
```   

>摘自《Calcite 数据管理实战》 7.2 章节 元数据
>#### 7.2.2 数据模型定义
>在Calcite中，定义数据模型默认采用配置文件的方式，我们只需要准备一个JSON或者YAML（Yet Another Markup Language，仍是一种标记语言）文件。由于JSON和YAML都可以完成参数配置的工作，因此它们之间可以互相转换。        
我们采用以下JSON文件，将其命名为“model.json”。其中定义了3个配置信息，即表示版本号的“version”、表示Schema默认名称的“defaultSchema”以及表示具体数据模式的“schemas”，如代码清单7-1所示。         
**代码清单7-1　数据模型用JSON文件方式定义的结构**    
```json   
{
   "version": "1.0", 
   "defaultSchema": "mongo", 
   "schemas": [...]
}
```
>当然，上述JSON文件也可以等价地替换成下面的YAML文件，如代码清单7-2所示。       
**代码清单7-2　数据模型用YAML文件方式定义的结构**
```yaml
version: 1.0
defaultSchema: mongo
schemas:
- [Schema...]
```
>我们可以看出，在这两个配置文件中，重点在Schema的定义上，在此之前先要了解Schema的分类。Calcite定义了3种类型，即MAP、JDBC、CUSTOM，虽然在实现上它们都有统一的接口，但具体的属性有些差别，可以参考Calcite源码org.apache.calcite.model包下的实现类。                 

下面对 KSQL 的自定义元数据实现进行介绍：      
![efak17](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak17.png)   

### JSqlSchemaFactory.java  
实现`SchemaFactory`接口，主要作用是基于`String model = createTempJson()`的JSON 传来的参数，构造 Schema 对象，也就是创建`JSqlSchema`实例。这里的处理与《Calcite 数据管理实战》中的`MysqlSchema` 不一样，它是需要连接 mysql查询后得到的结果，而 KSQL是经过`JSqlMapData.loadSchema(tabSchema, tableName, list);`处理后存储在`JSqlMapData#MAP`静态变量中。所以JSqlSchema构造方法传入 dbname 就知道 schema 信息。                  
```java
public class JSqlSchemaFactory implements SchemaFactory {

    @Override
    public Schema create(SchemaPlus parentSchema, String name, Map<String, Object> operand) {
       return new JSqlSchema(name);
    }
}
```    

### JSqlSchema.java    
`JSqlSchema`继承`AbstractSchema`，并重写了 AbstractSchema 中的一些方法。JSqlSchema 主要用于在 Apache Calcite 中定义一个结构化的 SQL schema，用于查询和操作自定义数据集。   

* JSqlSchema(String name)： 构造函数接受一个字符串 name（数据库名称）并将其存储在 dbName 字段中，用于表示当前 schema 所对应的数据库名称。     

* boolean isMutable()：该方法返回 true 或 false，表示 schema 是否是可变的。通过调用父类 AbstractSchema 的 isMutable 方法，这个方法可以决定 schema 是否可以在运行时被修改。        

* Expression getExpression()：该方法返回一个 Expression 对象，这个对象代表一个表达式。在 SQL 查询过程中，如果需要将一个 schema 转换为表达式（比如用于查询计划生成），这个方法会被调用。它使用父类的 getExpression 方法来获取表达式。    

* Multimap<String, Function> getFunctionMultimap()：返回 schema 中所有可用的函数，调用 ScalarFunctionImpl.createAll(JSONFunction.class) 创建所有的 JSONFunction 类中定义的标量函数。  

>注意：在 KSQL的官方文档提到支持 JSON,JSONArray 解析，https://docs.kafka-eagle.org/3.quickstart/7.ksql#id-3.7.2-parse-jsonobject-or-jsonarrays ，而 JSON `JSON(msg,'name')` ,JSONArray `JSONS(msg,'name')` 并不是 Calcite 内置的函数，可参考 https://calcite.apache.org/docs/reference.html        

* Map<String, Schema> getSubSchemaMap()：返回 schema 中的子 schema 映射。这个方法是为了支持嵌套的 schema，如果存在子 schema，则会返回它们的映射。     

* Map<String, Table> getTableMap()：返回 schema 中的所有表映射。        

```java
public class JSqlSchema extends AbstractSchema {

    private String dbName;

    public JSqlSchema(String name) {
        this.dbName = name;
    }

    @Override
    public boolean isMutable() {
        return super.isMutable();
    }

//  @Override
//  public boolean contentsHaveChangedSince(long lastCheck, long now) {
//     return super.contentsHaveChangedSince(lastCheck, now);
//  }

    @Override
    public Expression getExpression(SchemaPlus parentSchema, String name) {
        return super.getExpression(parentSchema, name);
    }

    @Override
    protected Multimap<String, Function> getFunctionMultimap() {
        ImmutableMultimap<String, ScalarFunction> funcs = ScalarFunctionImpl.createAll(JSONFunction.class);
        Multimap<String, Function> functions = HashMultimap.create();
        for (String key : funcs.keySet()) {
            for (ScalarFunction func : funcs.get(key)) {
                functions.put(key, func);
            }
        }
        return functions;
    }

    @Override
    protected Map<String, Schema> getSubSchemaMap() {
        return super.getSubSchemaMap();
    }

    @Override
    protected Map<String, Table> getTableMap() {
        Map<String, Table> tables = new HashMap<String, Table>();
        Database database = JSqlMapData.MAP.get(this.dbName);
        if (database == null) {
            return tables;
        }
        for (JSqlMapData.Table table : database.tables) {
            tables.put(table.tableName, new JSqlTable(table));
        }

        return tables;
    }


}
```

### JSONFunction.java 
```java
public class JSONFunction {

    /**
     * Parse a JSONObject.
     */
    public String JSON(String jsonObject, String key) {
        JSONObject object = com.alibaba.fastjson.JSON.parseObject(jsonObject);
        return object.getString(key);
    }

    /**
     * Parse a JSONArray.
     */
    public String JSONS(String jsonArray, String key) {
        JSONArray object = com.alibaba.fastjson.JSON.parseArray(jsonArray);
        JSONArray target = new JSONArray();
        for (Object tmp : object) {
            JSONObject result = (JSONObject) tmp;
            JSONObject value = new JSONObject();
            value.put(key, result.getString(key));
            target.add(value);
        }
        return target.toString();
    }

}
```     

### JSqlTable.java    
对表元数据，Calcite也提供了对应的实体——Table，不过这依然是一个接口，我们可以继承其子类——`AbstractTable`，只需要重写其getRowType方法，返回表的字段名和类型映射。 实现`ScannableTable`接口的`scan()`,通过实现 scan() 方法，你可以确保所有的 SQL 查询，无论底层数据源是什么，都能够按照相同的方式执行。       
```java
public class JSqlTable extends AbstractTable implements ScannableTable {
    private JSqlMapData.Table sourceTable;
    private RelDataType dataType;

    public JSqlTable(JSqlMapData.Table table) {
        this.sourceTable = table;
    }

    private static int[] identityList(int n) {
        int[] integers = new int[n];
        for (int i = 0; i < n; i++) {
            integers[i] = i;
        }
        return integers;
    }

    @Override
    public RelDataType getRowType(RelDataTypeFactory typeFactory) {
        if (this.dataType == null) {
            RelDataTypeFactory.Builder fieldInfo = typeFactory.builder();
            for (JSqlMapData.Column column : this.sourceTable.columns) {
                RelDataType sqlType = typeFactory.createJavaType(JSqlMapData.JAVATYPE_MAPPING.get(column.type));
                sqlType = SqlTypeUtil.addCharsetAndCollation(sqlType, typeFactory);
                fieldInfo.add(column.name, sqlType);
            }
            this.dataType = typeFactory.createStructType(fieldInfo.build().getFieldList());
        }
        return this.dataType;
    }

    @Override
    public Enumerable<Object[]> scan(DataContext dataContext) {
        final List<String> types = new ArrayList<String>(sourceTable.columns.size());
        for (JSqlMapData.Column column : sourceTable.columns) {
            types.add(column.type);
        }

        final int[] fields = identityList(types.size());
        return new AbstractEnumerable<Object[]>() {
            @Override
            public Enumerator<Object[]> enumerator() {
                return new JSqlEnumerator<Object[]>(fields, types, sourceTable.data);
            }
        };
    }

}
```

2.将`List<JSONArray> dataSets` 转成 `List<List<String>> list`对象。   
![efak16](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak16.png)        

3.loadSchema()方法 将 list 集合构建成一个符合 db 结构的数据集。   
```java
public static void loadSchema(JSONObject cols, String tableName, List<List<String>> datas) {
    Database db = new Database();
    Table table = new Table();
    table.tableName = tableName;
    for (String key : cols.keySet()) {
        Column _col = new Column();
        _col.name = key;
        _col.type = cols.getString(key);
        table.columns.add(_col);
    }
    table.data = datas;
    db.tables.add(table);
    MAP.put("db", db);
}

public static class Database {
    public List<Table> tables = new LinkedList<Table>();
}

public static class Table {
    public String tableName;
    public List<Column> columns = new LinkedList<Column>();
    public List<List<String>> data = new LinkedList<List<String>>();
}

public static class Column {
    public String name;
    public String type;
}
```

有了对`JSqlSchema#getTableMap()`的了解，loadSchema()方法将`List<List<String>> list`构建成所有表映射关系。        

4.构造查询过程  
```java
Class.forName(JConstants.KAFKA_DRIVER);
Properties info = new Properties();
info.setProperty("lex", Lex.JAVA.toString());

Connection connection = DriverManager.getConnection(Driver.CONNECT_STRING_PREFIX + "model=inline:" + model, info);
Statement st = connection.createStatement();
ResultSet result = st.executeQuery(UnicodeUtils.encodeForUnicode(sql));
```
 
要是不熟悉Calcite的同学，看过我之前 Blog`Calcite - Official - Background 扩展`,对上面 API 或多或少有些熟悉`HelloCalciteReadStaticData.java`  
```java
  Class.forName("org.apache.calcite.jdbc.Driver");
  Properties info = new Properties();
  info.setProperty("lex", "JAVA");
  Connection connection =
          DriverManager.getConnection("jdbc:calcite:", info);
  CalciteConnection calciteConnection =
          connection.unwrap(CalciteConnection.class);
  SchemaPlus rootSchema = calciteConnection.getRootSchema();
  Schema schema = new ReflectiveSchema(new HrSchema());
  rootSchema.add("hr", schema);
  Statement statement = calciteConnection.createStatement();
  ResultSet resultSet = statement.executeQuery(
          "select d.deptno, min(e.empid) as empid \n"
                  + "from hr.emps as e\n"
                  + "join hr.depts as d\n"
                  + "  on e.deptno = d.deptno\n"
                  + "group by d.deptno\n"
                  + "having count(*) > 1");
```

### 小结  
![efak19](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak19.png)    

KSQL 首先解析 SQL，获取 topic 相关信息，然后构造 Kafka Consumer 读取kafka 消息，将数据缓存起来，利用 Calcite 自定义表元数据，通过 Calcite SQL 查询数据出来。 在返回给页面。         

这里还有个需要注意的点：select * from topicName ... ,其中的 * 代表之前定义的`kafkaSql.getSchema()`集合，所在SQL 的WHERE 可以写 `partition`、Select 可以写`select JSON(msg,'name')`
```java
private KafkaSqlInfo segments(String clusterAlias, String sql) {
    KafkaSqlInfo kafkaSql = new KafkaSqlInfo();
    kafkaSql.setSql(sql);
    kafkaSql.getSchema().put("partition", "integer");
    kafkaSql.getSchema().put("offset", "bigint");
    kafkaSql.getSchema().put("msg", "varchar");
    kafkaSql.getSchema().put("timespan", "varchar");
    kafkaSql.getSchema().put("date", "varchar");
    
    省略部分代码...
}
```


## 总结   
下面展示了方法间整体调用关系，其重点是 SQL解析 以及自定义表元数据、函数，对数据集进行查询，从而达到数据检索的过程。不过每次查询也确实会消耗较多内存空间。         
![efak18](http://img.xinzhuxiansheng.com/blogimgs/calcite/efak18.png)      

refer   
1.https://docs.kafka-eagle.org/3.quickstart/7.ksql    
2.https://github.com/yahoo/CMAK           
3.《Calcite 数据管理实战》 6.2 章节 SqlNode 体系                   
4.《Calcite 数据管理实战》 7.2 章节 元数据      
5.https://calcite.apache.org/docs/adapter.html      

