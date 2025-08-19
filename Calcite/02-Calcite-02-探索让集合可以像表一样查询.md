# Calcite - 探索让集合可以像表一样查询         

## 引言        
想必你一定听过:"内存相对于磁盘IO，网络IO是廉价的"，再或者你可能也会有这种感受:"SQL 是比较简单的编程语言"。 在工作中，使用 java 集合作为数据缓存是较常见的一种做法，若要查询集合中数据，大多数情况指定某个 key 查询单一值 或者调用 lambda 和 function 变量组合查询，这种方式灵活性较高，代码简洁。      

其实我们也可以借助 Calcite 让集合像 Table 一样使用 SQL 查询 !  相较于 function 变量变更数据处理逻辑来说，SQL 更简单。         

接下来，我们通过一个案例来介绍 "让集合可以像表一样查询"。     
  
## Calcite 是 ？  
它? 你不知道？没事，你可以看我的公众号文章了解 Calcite：    
* `Calcite - Official - Background 扩展` https://mp.weixin.qq.com/s/diOGnl-Vx81__8DcCGfkrQ   
* `Calcite - Official - Tutorial 扩展` https://mp.weixin.qq.com/s/H7kbIAvKdIHgF5L_jJH4mg  
* `Calcite - JavaCC - 介绍及安装` https://mp.weixin.qq.com/s/-mZjfiPblYd6-waMR0IN7w    
* `Calcite - JavaCC - 官网 Example 入门实践` https://mp.weixin.qq.com/s/Cfi79VuJO33ti4fdY5-qrw   
* `Calcite - JavaCC - 扩展实践` https://mp.weixin.qq.com/s/jP4zTSisaHttMWUKnEDbYg   
* `Calcite - 探索 Kafka Eagle KSQL 数据查询实现` https://mp.weixin.qq.com/s/55QMA0EI-Lbr3PSj60mQtg   
* `Calcite - Official - Tutorial CSV 案例说明补充 (不包含执行规则)` https://mp.weixin.qq.com/s/YdKV4V4ToRy0h42brg4PSg   

## 让集合可以像表一样查询     

**结构图**
![querylistbycalcite03](http://img.xinzhuxiansheng.com/blogimgs/calcite/querylistbycalcite03.jpg)  

**项目示例** 
![querylistbycalcite04](http://img.xinzhuxiansheng.com/blogimgs/calcite/querylistbycalcite04.jpg)     

### 构造集合数据  
我使用 fabric8 k8s client 查询 Pod 列表信息，以此来构造一个集合数据，调用 `QueryDataMain#getPodInfos()` 方法可以获取到 Pod 相关字段信息 `namespace, podName, ready, status, restartCount, age, podIP, node`。  

查询结果示例：  
![querylistbycalcite01](http://img.xinzhuxiansheng.com/blogimgs/calcite/querylistbycalcite01.jpg)   

现在我们想通过 SQL 查询的方式对 `podInfos` 集合数据进行检索。 下面我们会逐步完善 `QueryDataMain#main()`。 

`QueryDataMain.java`
```java
import io.fabric8.kubernetes.api.model.ContainerStatus;
import io.fabric8.kubernetes.api.model.NamespaceList;
import io.fabric8.kubernetes.api.model.Pod;
import io.fabric8.kubernetes.api.model.PodList;
import io.fabric8.kubernetes.client.KubernetesClient;
import io.fabric8.kubernetes.client.KubernetesClientBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.sql.SQLException;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class QueryDataMain {
  private static final Logger logger = LoggerFactory.getLogger(QueryDataMain.class);

  public static void main(String[] args) throws SQLException {
    List<PodInfo> podInfos = getPodInfos();

    // 对 podInfos 集合进行检索 

  }

  private static List<PodInfo> getPodInfos() {
    List<PodInfo> podInfos = new ArrayList<>();
    try {
      // 创建 Kubernetes Client 实例
      KubernetesClient client = new KubernetesClientBuilder().build();

      // 获取命名空间列表
      NamespaceList namespaceList = client.namespaces().list();

      // 打印命名空间信息
      logger.info("Available namespaces:");
      namespaceList.getItems().forEach(namespace -> {
        logger.info("Namespace: {}", namespace.getMetadata().getName());
      });

      // 获取所有Pod信息
      logger.info("Pod Information:");
      PodList podList = client.pods().inAnyNamespace().list();

      for (Pod pod : podList.getItems()) {
        String namespace = pod.getMetadata().getNamespace();
        String podName = pod.getMetadata().getName();
        String podIP = pod.getStatus().getPodIP() != null ? pod.getStatus().getPodIP() : "";
        String node = pod.getSpec().getNodeName() != null ? pod.getSpec().getNodeName() : "";
        String status = pod.getStatus().getPhase();

        // 计算Pod的Age
        Date creationTimestamp = pod.getMetadata().getCreationTimestamp() != null ?
                Date.from(Instant.parse(pod.getMetadata().getCreationTimestamp())) : null;
        String age = calculateAge(creationTimestamp);

        // 计算Ready状态容器数
        int readyContainers = 0;
        int totalContainers = 0;
        int restartCount = 0;

        if (pod.getStatus().getContainerStatuses() != null) {
          totalContainers = pod.getStatus().getContainerStatuses().size();
          for (ContainerStatus containerStatus : pod.getStatus().getContainerStatuses()) {
            if (containerStatus.getReady()) {
              readyContainers++;
            }
            restartCount += containerStatus.getRestartCount();
          }
        }

        String ready = readyContainers + "/" + totalContainers;

        // 创建PodInfo对象
        PodInfo podInfo = new PodInfo(namespace, podName, ready, status, restartCount, age, podIP, node);
        podInfos.add(podInfo);
      }

      // 关闭客户端
      client.close();
    } catch (Exception e) {
      logger.error("Error while connecting to Kubernetes", e);
    }
    return podInfos;
  }

  private static String calculateAge(Date creationTimestamp) {
    if (creationTimestamp == null) {
      return "Unknown";
    }

    Duration duration = Duration.between(creationTimestamp.toInstant(), Instant.now());
    long days = duration.toDays();
    long hours = duration.minusDays(days).toHours();
    long minutes = duration.minusDays(days).minusHours(hours).toMinutes();

    if (days > 0) {
      return days + "d" + hours + "h";
    } else if (hours > 0) {
      return hours + "h" + minutes + "m";
    } else {
      return minutes + "m";
    }
  }
}
```

`PodInfo.java`
```java
@Data
public class PodInfo {
    private String namespace;
    private String podName;
    private String ready;
    private String status;
    private int restartCount;
    private String age;
    private String podIP;
    private String node;

    public PodInfo() {}

    public PodInfo(String namespace, String podName, String ready, String status, 
                   int restartCount, String age, String podIP, String node) {
        this.namespace = namespace;
        this.podName = podName;
        this.ready = ready;
        this.status = status;
        this.restartCount = restartCount;
        this.age = age;
        this.podIP = podIP;
        this.node = node;
    }
    
    @Override
    public String toString() {
        // 原始输出格式
        return String.format("%s\t\t%s\t\t%s\t\t%s\t\t%d\t\t%s\t\t%s\t\t%s",
                namespace, podName, ready, status, restartCount, age, podIP, node);
    }
}
```

接下来，我们开始使用 Calcite 来构造 SQL 查询逻辑。      

### 自定义表元数据（Schema、Table）  
我们需要继承 `AbstractSchema` 类，Schema 在 Calcite 中是由对应的实体类定义的，虽然是一个接口，但我们不用从零开始，Calcite 已经提供了相关的接口——AbstractSchema，我们只需要实现这个接口和其中的 getTableMap() 方法, key 为表名，value 为 Table实例。    

由于该案例中的 `schmea` 是一次性创建好的，所以暂时并不需要 `SchemaFactory`。  

`K8sSchema.java`   
```java
import org.apache.calcite.schema.Table;
import org.apache.calcite.schema.impl.AbstractSchema;

import java.util.HashMap;
import java.util.Map;

public class K8sSchema extends AbstractSchema {
  Map<String, Table> tableMap = new HashMap<>();

  public void addTable(String name, Table table) {
    tableMap.put(name, table);
  }

  public K8sSchema() {
  }

  @Override
  protected Map<String, Table> getTableMap() {
    return tableMap;
  }
}
```


K8sSchema 使用到了 Table，而 Calcite 也提供了对应的实体——Table，不过这依然是一个接口，`K8sSchema` 帮助我们定义 Schmea 层级，那下面的 `K8sTable` 帮助我们定义 Table 层级， 所以我们可以继承其子类——`AbstractTable`，我们只需要重写其 `getRowType()` 方法，定义 Table 的字段名和类型映射。     

K8sTable 也实现`ScannableTable`接口的`scan()`方法 ，`K8sTable` 下一个层级是 `数据`，我们需要在 scan() 方法实现迭代器 (Enumerator)，以此来实现数据的访问范式，由于我们得数据是一个`集合`， 所以又创建了 `PodInfoList` 包装其内部数据集。  

**定义迭代器 (Enumerator)**    
scan() 方法返回值需实现 `Enumerator` 接口，用于返回数据。实现类提供了4个主要的方法:close、moveNext、current、reset。   
* close：该方法用于关闭迭代器和相关资源。该方法属于幂等操作，不论调用几次结果都是相同的。    
* moveNext：该方法用于判断是否还有下一个元素，如果有则返回true，否则返回false。创建完迭代器之后或者调用完 reset 方法之后，指针指向的便是第一个元素之前的位置而不是第一个元素。因此执行的时候最先调用的便是moveNext方法，用于移动指针到第一个元素。当指针移动到末尾之后便会返回false。    
* current: moveNext方法会判断是否还有下一个元素，并移动指针。current() 方法便是返回当前指针指向的值。因此一般调用的顺序是在创建完迭代器之后使用 moveNext() 方法后移指针，之后使用 current方法返回当前指针指向的值。       
* reset：该方法用于重置迭代器的指针，将其指向第一个元素之前的位置。使用的时候需要考虑该迭代器是否支持重置，如果不支持则需要考虑抛出异常。     

`K8sTable.java`    
```java
import org.apache.calcite.DataContext;
import org.apache.calcite.linq4j.AbstractEnumerable;
import org.apache.calcite.linq4j.Enumerable;
import org.apache.calcite.linq4j.Enumerator;
import org.apache.calcite.rel.type.RelDataType;
import org.apache.calcite.rel.type.RelDataTypeFactory;
import org.apache.calcite.schema.ScannableTable;
import org.apache.calcite.schema.impl.AbstractTable;
import org.apache.calcite.sql.type.SqlTypeName;
import org.jetbrains.annotations.NotNull;

import java.util.Iterator;

public class K8sTable extends AbstractTable implements ScannableTable {

  private PodInfoList podInfoList;

  public K8sTable(PodInfoList podInfoList) {
    this.podInfoList = podInfoList;
  }

  @Override
  public Enumerable<Object[]> scan(DataContext root) {
    return new AbstractEnumerable<Object[]>() {

      @Override
      public Enumerator<Object[]> enumerator() {
        // 获取 PodInfo 迭代器
        Iterator<PodInfo> iterator = podInfoList.getIterator();

        return new Enumerator<Object[]>() {
          private Object[] current;

          @Override
          public Object[] current() {
            return current;
          }

          @Override
          public boolean moveNext() {
            // 检查是否还有下一个元素
            if (iterator.hasNext()) {
              PodInfo podInfo = iterator.next();

              // 处理 podInfo 为 null 的情况
              if (podInfo != null) {
                current = new Object[]{
                        podInfo.getNamespace() != null ? podInfo.getNamespace() : "",
                        podInfo.getPodName() != null ? podInfo.getPodName() : "",
                        podInfo.getReady() != null ? podInfo.getReady() : "",
                        podInfo.getStatus() != null ? podInfo.getStatus() : "",
                        podInfo.getRestartCount(), 
                        podInfo.getAge() != null ? podInfo.getAge() : "",
                        podInfo.getPodIP() != null ? podInfo.getPodIP() : "",
                        podInfo.getNode() != null ? podInfo.getNode() : ""
                };
              } else {
                // podInfo 为 null 时返回默认值
                current = new Object[]{"", "", "", "", 0, "", "", ""};
              }
              return true;
            }
            current = null;
            return false;
          }

          @Override
          public void reset() {

          }

          @Override
          public void close() {

          }
        };
      }

      @NotNull
      @Override
      public Iterator<Object[]> iterator() {
        Iterator<PodInfo> iterator = podInfoList.getIterator();
        return new Iterator<Object[]>() {
          @Override
          public boolean hasNext() {
            return iterator.hasNext();
          }

          // 迭代返回每行数据
          @Override
          public Object[] next() {
            PodInfo next = iterator.next();
            return new Object[]{
                    next.getNamespace(),
                    next.getPodName(),
                    next.getReady(),
                    next.getStatus(),
                    next.getRestartCount(),
                    next.getAge(),
                    next.getPodIP(),
                    next.getNode()};
          }
        };
      }
    }

            ;
  }

  @Override
  public RelDataType getRowType(RelDataTypeFactory typeFactory) {
    return typeFactory.builder()
            .add("namespace", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .add("podName", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .add("ready", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .add("status", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .add("restartCount", typeFactory.createSqlType(SqlTypeName.INTEGER))
            .add("age", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .add("podIP", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .add("node", typeFactory.createSqlType(SqlTypeName.VARCHAR))
            .build();
  }
}
```

`PodInfoList`类比较简单，我们将 `QueryDataMain#getPodInfos()`查询到的 Pod 信息集合放入到 PodInfoList 对象中，交于它统一管理，例如重新赋值 Pod信息，动态添加 Pod 信息，返回PodInofs 集合数据的迭代器。     
`PodInfoList`
```java
import com.google.common.collect.Lists;

import java.util.Iterator;
import java.util.List;

public class PodInfoList {
  private List<PodInfo> podInfos;

  public PodInfoList(List<PodInfo> podInfos) {
    this.podInfos = podInfos;
  }

  public void setPodInfoist(List<PodInfo> podInfos) {
    this.podInfos = podInfos;
  }

  public static PodInfoList create() {
    return new PodInfoList(Lists.newArrayList());
  }

  public void addItem(PodInfo item) {
    podInfos.add(item);
  }

  public Iterator<PodInfo> getIterator() {
    return podInfos.iterator();

  }
}
```   

下面就到了我们得重头戏了，上面 `K8sSchema` 对应的 Schema， `K8sTable` 对应的 Table， `PodInfoList` 对应的 List Data， 最后交给 `PodInfoCalciteManager` 使用 Calcite 来关联它们。    
* `PodInfoCalciteManager#PodInfoCalciteManager()` 方法调用 Calcite API 初始化 RootSchema，我们将 k8sSchema，k8sTable 放入 rootSchema 中。   
* `PodInfoCalciteManager#updatePodInfoList()` 方法更新集合数据，以此达到更新数据。    
* `executeQuery()` 调用 `jdbc:calcite:` 协议查询数据。    

>这里需要注意的是 Properties info 值，它会影响你传入 SQL 的写入方式，例如，caseSensitive 是否忽略大小写， unquotedCasing 为 UNCHANGED，Sql 中的 field 保留原有输入格式。   

`PodInfoCalciteManager.java`
```java
import org.apache.calcite.jdbc.CalciteConnection;
import org.apache.calcite.schema.SchemaPlus;
import org.apache.calcite.schema.Table;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.sql.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

public class PodInfoCalciteManager {
  private static final Logger logger = LoggerFactory.getLogger(PodInfoCalciteManager.class);

  private PodInfoList podInfoList;
  private K8sSchema k8sSchema;
  private CalciteConnection calciteConnection;
  private SchemaPlus rootSchema;

  public PodInfoCalciteManager() throws SQLException {

    this.podInfoList = PodInfoList.create();
    Table k8sTable = new K8sTable(this.podInfoList);
    this.k8sSchema = new K8sSchema();
    this.k8sSchema.addTable("podInfoTable", k8sTable);

    // Initialize Calcite connection
    Properties info = new Properties();
    info.setProperty("caseSensitive", "true");
    info.setProperty("unquotedCasing", "UNCHANGED");
//    info.setProperty("quoting", "BACK_TICK");
    Connection connection = DriverManager.getConnection("jdbc:calcite:", info);
    this.calciteConnection = connection.unwrap(CalciteConnection.class);
    this.rootSchema = calciteConnection.getRootSchema();
    this.rootSchema.add("podInfoSchema", k8sSchema);
  }

  public void updatePodInfoList(List<PodInfo> newPodInfoList) {
    this.podInfoList.setPodInfoist(newPodInfoList);
    System.out.println("PodInfoList has been updated.");
  }

  public List<PodInfo> executeQuery(String sql) {
    List<PodInfo> podInfoList = new ArrayList<>();
    try {
      ResultSet resultSet = executeQuerySql(sql);
      while (resultSet.next()) {
        PodInfo podInfo = new PodInfo();
        podInfo.setNamespace(resultSet.getString("namespace"));
        podInfo.setPodName(resultSet.getString("podName"));
        podInfo.setReady(resultSet.getString("ready"));
        podInfo.setStatus(resultSet.getString("status"));
        podInfo.setRestartCount(resultSet.getInt("restartCount"));
        podInfo.setAge(resultSet.getString("age"));
        podInfo.setPodIP(resultSet.getString("podIP"));
        podInfo.setNode(resultSet.getString("node"));
        podInfoList.add(podInfo);
      }
    } catch (SQLException e) {
      throw new RuntimeException(e);
    }
    return podInfoList;
  }

  private ResultSet executeQuerySql(String sql) throws SQLException {
    Statement statement = calciteConnection.createStatement();
    return statement.executeQuery(sql);
  }
}
```   

### main() 方法查询     
现在我们来完成 `QueryDataMain#main()` 查询部分。 首先将 podInfos 集合数据通过`引用传递`放入到 k8sTable 中， 最后调用 `podInfoCalciteManager.executeQuery()` 方法传入 SQL 执行查询逻辑，      

>SQL 编写格式与 `PodInfoCalciteManager#PodInfoCalciteManager()` 的 Properties info 配置 `息息相关`。  

```java
public class QueryDataMain {
  private static final Logger logger = LoggerFactory.getLogger(QueryDataMain.class);

  public static void main(String[] args) throws SQLException {
    List<PodInfo> podInfos = getPodInfos();
    PodInfoCalciteManager podInfoCalciteManager = new PodInfoCalciteManager();
    podInfoCalciteManager.updatePodInfoList(podInfos);
    List<PodInfo> result = podInfoCalciteManager.executeQuery("select * from podInfoSchema.podInfoTable where namespace like '%flink%'");
    if (result != null) {
      result.forEach(System.out::println);
    }
  }

// ...... 省略部分代码   
```

结果：  
![querylistbycalcite02](http://img.xinzhuxiansheng.com/blogimgs/calcite/querylistbycalcite02.jpg)  

到这里，我们得查询示例就开发完成。   