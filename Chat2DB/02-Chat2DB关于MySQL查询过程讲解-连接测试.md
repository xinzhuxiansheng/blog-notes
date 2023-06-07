## Chat2DB关于MySQL查询过程讲解 - 连接测试 

>Chat2DB version: 1.0.9

### 引言  
以下摘自Chat2DB官网的一段介绍 

>Chat2DB 是一款有开源免费的多数据库客户端工具，支持windows、mac本地安装，也支持服务器端部署，web网页访问。和传统的数据库客户端软件Navicat、DBeaver 相比Chat2DB集成了AIGC的能力，能够将自然语言转换为SQL，也可以将SQL转换为自然语言，可以给出研发人员SQL的优化建议，极大的提升人员的效率，是AI时代数据库研发人员的利器，未来即使不懂SQL的运营业务也可以使用快速查询业务数据、生成报表能力。

本篇主要以集成MySQL查询为例，是如何实现Web SQL查询的。  

### 添加MySQL  

**入口**  
![querySQL01](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL01.png)  


`配置数据源`  

![querySQL02](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL02.png)    

`连接数据源`表单支持`JDBC 5.0/8.0`两个版本，我们可以配置MySQL连接地址，通过`连接测试`测试配置信息是否正确。 
**那是如何来区分 JDBC 5.0/8.0两个不同的版本的呢？以及MySQL连接信息又是如何持久化的呢**  

`Rest API`    
```shell
http://127.0.0.1:10821/api/connection/datasource/pre_connect

# POST 参数
{"ssh":{"use":"false","hostName":"","port":"","userName":"","localPort":"","password":""},"alias":"@localhost","host":"localhost","port":"3306","authentication":1,"user":"root","password":"12345678","database":"","url":"jdbc:mysql://localhost:3306/","jdbc":"8.0","extendInfo":[{"key":"zeroDateTimeBehavior","value":"convertToNull"}],"EnvType":"DAILY","type":"MYSQL"}
```

`DataSourceController#preConnect()`方法处理连接测试逻辑，在preConnect()方法中的`dataSourceWebConverter`,而它本身是一个抽象类,它使用了`@Mapper(componentModel = "spring")`，它是MapStruct库中的一个注解，是一个用于在Java Bean类型之间自动生成映射代码的库。它能够生成可在运行时实例化和使用的映射器对象，减少了手动编写字段到字段复制代码的需要。     

>注意默认是同名字段 

**DataSourceController#preConnect()**   
```java
  @RequestMapping("/datasource/pre_connect")
  public ActionResult preConnect(@RequestBody DataSourceTestRequest request) {
      DataSourcePreConnectParam param = dataSourceWebConverter.testRequest2param(request);
      return dataSourceService.preConnect(param);
  }
```   
我们可以通过IDE的`Diagrams`插件可以对比下两个实体类字段名称是否一样。 其实这里处理方式也特别像 hutool的`BeanUtil#copyProperties()`方法，进行字段的赋值，避免了手动去set较多字段。 
![querySQL03](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL03.png)   

接下来我们看preConnect()方法。    
```java
@Override
public ActionResult preConnect(DataSourcePreConnectParam param)  {
    DataSourceTestParam testParam
        = dataSourceConverter.param2param(param);
    DataSourceConnect dataSourceConnect = JdbcUtils.testConnect(testParam.getUrl(), testParam.getHost(),
        testParam.getPort(),
        testParam.getUsername(), testParam.getPassword(), DbTypeEnum.getByName(testParam.getDbType()),
        param.getJdbc(), param.getSsh(), KeyValue.toMap(param.getExtendInfo()));
    if (BooleanUtils.isNotTrue(dataSourceConnect.getSuccess())) {
        return ActionResult.fail(dataSourceConnect.getMessage(), dataSourceConnect.getDescription());
    }
    return ActionResult.isSuccess();
}
``` 
在对`testParam`做类型转换时，这里处理与上面`dataSourceWebConverter.testRequest2param`处理有稍许不同，`dataSourceConverter#param2param()`方法使用字段映射规则，示例如下：  
```java
@Mappings({
    @Mapping(source = "type", target = "dbType"),
    @Mapping(source = "user", target = "username")
})
public abstract DataSourceTestParam param2param(
    DataSourcePreConnectParam dataSourcePreConnectParam);
```
这意味着，在两个实体类字段不一样时，可通过定义字段的引射规则匹配并赋值。  

现在我们回到`JdbcUtils#testConnect()`,它的形参与平时我们访问MySQL没啥区别，但在JDBC连接MySQL（5.0/8.0）不同版本时，驱动名称是会有不同，那Chat2DB又是如何处理根据`param.getJdbc()`使用不同`mysql-connector`的呢？ 我们先看下其核心`Connection`对象的构建过程： 
```java
// 创建连接
connection = IDriverManager.getConnection(url, userName, password,  
DriverTypeEnum.getDriver(dbType, jdbc), properties);  
``` 
`DriverTypeEnum.getDriver(dbType, jdbc)` 根据db类型以及jdbc版本返回了一个`DriverTypeEnum`枚举，内容如下： 
![querySQL04](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL04.png)    

* **DBTypeEnum**: 用于标识DB类型  
* **driverClass**：DB连接驱动名称 
* **jar**: 驱动jar  
* **jdbc**: DB版本  

此时到`IDriverManager#getConnection()`方法  
```java
public static Connection getConnection(String url, Properties info, DriverTypeEnum driverTypeEnum)
    throws SQLException {
    if (url == null) {
        throw new SQLException("The url cannot be null", "08001");
    }
    DriverManager.println("DriverManager.getConnection(\"" + url + "\")");
    SQLException reason = null;
    DriverEntry driverEntry = DRIVER_ENTRY_MAP.get(driverTypeEnum);
    if (driverEntry == null) {
        driverEntry = getJDBCDriver(driverTypeEnum);
    }
    try {
        Connection con = driverEntry.getDriver().connect(url, info);
        if (con != null) {
            DriverManager.println("getConnection returning " + driverEntry.getDriver().getClass().getName());
            return con;
        }
    } catch (SQLException var7) {
        if (reason == null) {
            reason = var7;
        }
    }

    if (reason != null) {
        DriverManager.println("getConnection failed: " + reason);
        throw reason;
    } else {
        DriverManager.println("getConnection: no suitable driver found for " + url);
        throw new SQLException("No suitable driver found for " + url, "08001");
    }
}
```

通过driverTypeEnum枚举作为key，从`DRIVER_ENTRY_MAP`获取`DriverEntry`类型的对象，如果获取不到则生成一个并放入`DRIVER_ENTRY_MAP`中，接下来我们看`DriverEntry`对象是如何创建的？ 

```java
private static DriverEntry getJDBCDriver(DriverTypeEnum driverTypeEnum)
    throws SQLException {
    synchronized (driverTypeEnum) {
        try {
            if (DRIVER_ENTRY_MAP.containsKey(driverTypeEnum)) {
                return DRIVER_ENTRY_MAP.get(driverTypeEnum);
            }
            ClassLoader cl = getClassLoader(driverTypeEnum);
            Driver driver = (Driver)cl.loadClass(driverTypeEnum.getDriverClass()).newInstance();
            DriverEntry driverEntry = DriverEntry.builder().driverTypeEnum(driverTypeEnum).driver(driver).build();
            DRIVER_ENTRY_MAP.put(driverTypeEnum, driverEntry);
            return driverEntry;
        } catch (Exception e) {
            log.error("getJDBCDriver error", e);
            throw new SQLException("getJDBCDriver error", "08001");
        }
    }

}
```

### IDriverManager是如何加载MySQL不同JDBC   
上面我们已经分析到`IDriverManager#getJDBCDriver()`方法负责加载JDBC驱动类。 如果要动态加载class，需要做以下几点：    
* 下载对应版本的MySQL的mysql-connector jar  
* 扫描jar文件   
* 创建类加载器        
* 加载指定类名并创建实例对象        

**IDriverManager#getClassLoader()** 
```
public static ClassLoader getClassLoader(DriverTypeEnum driverTypeEnum) throws MalformedURLException {
    String jarPath = driverTypeEnum.getJar();
    if (CLASS_LOADER_MAP.containsKey(jarPath)) {
        return CLASS_LOADER_MAP.get(jarPath);
    } else {
        synchronized (jarPath) {
            if (CLASS_LOADER_MAP.containsKey(jarPath)) {
                return CLASS_LOADER_MAP.get(jarPath);
            }
            String[] jarPaths = jarPath.split(",");
            URL[] urls = new URL[jarPaths.length];
            for (int i = 0; i < jarPaths.length; i++) {
                File driverFile = new File(getFullPath(jarPaths[i]));
                urls[i] = driverFile.toURI().toURL();
            }
            //urls[jarPaths.length] = new File(JdbcJarUtils.getFullPath("HikariCP-4.0.3.jar")).toURI().toURL();

            URLClassLoader cl = new URLClassLoader(urls, ClassLoader.getSystemClassLoader());
            log.info("ClassLoader class:{}", cl.hashCode());
            log.info("ClassLoader URLs:{}", JSON.toJSONString(cl.getURLs()));

            try {
                cl.loadClass(driverTypeEnum.getDriverClass());
            } catch (Exception e) {
                //如果报错删除目录重试一次
                for (int i = 0; i < jarPaths.length; i++) {
                    File driverFile = new File(JdbcJarUtils.getNewFullPath(jarPaths[i]));
                    urls[i] = driverFile.toURI().toURL();
                }
                //urls[jarPaths.length] = new File(JdbcJarUtils.getFullPath("HikariCP-4.0.3.jar")).toURI().toURL();
                cl = new URLClassLoader(urls, ClassLoader.getSystemClassLoader());

            }
            CLASS_LOADER_MAP.put(jarPath, cl);
            return cl;
        }
    }
}
```
获取jar路径，扫描jar文件，创建url类加载器并指定父加载器，再通过loadClass()加载到JVM中，最后再放入`CLASS_LOADER_MAP.put(jarPath, cl);` 缓存中。  

>这段 Java 代码创建了一个新的 `URLClassLoader` 实例。`URLClassLoader` 是 Java 类加载器（ClassLoader）的一个具体实现，它按照特定的 URL（统一资源定位符）列表来加载类和资源。 
以下是代码的具体分析：  
- `urls`：这是一个 `URL[]` 数组，表示类加载器从中加载类和资源的 URL 列表。URL 可以引用本地文件系统上的文件，也可以引用网络上的资源。    
- `ClassLoader.getSystemClassLoader()`：这是父类加载器（parent class loader）。每个类加载器（除了引导类加载器）在其创建时都有一个相关的父类加载器。当请求类加载器加载类或资源时，它首先将请求委托给其父类加载器，这是双亲委派模型的核心。   
通过创建新的 `URLClassLoader` 实例，你可以动态地将额外的路径（在这种情况下是 URL）添加到应用程序的类路径中，从而可以从那些路径加载类和资源。这在需要动态加载类，如插件系统中，非常有用。        


**这里就丢了一个疑惑，mysql-connector连接器的jar是什么下载的?** 

![querySQL05](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL05.png)   

在`Chat2DB`项目启动日志中看到上面图片中的log，我想这里也帮我们解惑了，当服务启动后首先会判断某个目录下DB连接驱动jar是否存在，不存在则下载，那接着探究它是如何实现的？   
```
@Component
@Slf4j
public class JarDownloadTask implements CommandLineRunner {

    @Resource
    private AliDbhubProperties aliDbhubProperties;

    @Override
    public void run(String... args) throws Exception {
        DbhubContext.JDBC_JAR_DOWNLOAD_URL_LIST = aliDbhubProperties.getJdbcJarDownLoadUrls();
        JdbcJarUtils.asyncDownload(aliDbhubProperties.getJdbcJarDownLoadUrls());
    }
}
```
`JarDownloadTask`实现了`CommandLineRunner接口`， `CommandLineRunner` 是一个Spring Boot特性，它允许你在应用程序启动后运行一些特定的代码。你可以实现它来运行任何你需要在Spring Boot应用程序启动后执行的代码。 

从JarDownloadTask代码可知，服务启动后读取`application.yaml`配置，拉取jar并下载到用户目录下  
```java
// 指定目录
private static final String PATH = System.getProperty("user.home") + File.separator + ".chat2db" + File.separator
    + "jdbc-lib" + File.separator;
``` 

**配置信息**    
![querySQL06](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL06.png)   

**总体流程**    
![querySQL07](http://img.xinzhuxiansheng.com/blogimgs/chat2db/querySQL07.png)  


### 总结     
本篇blog的技术实现难点在于`类的动态加载`，这里特别提醒，必须要将已经加载的class放入缓存中，避免JVM频繁加载可能导致内存溢出问题。    



>在Java中，频繁的动态加载类可能导致Metaspace或者PermGen Space（取决于你使用的是Java 8+还是更早的版本）的内存溢出。  

>在Java 8之前，类元数据被存储在被称为PermGen Space的堆内存的一个特殊区域中。如果这个区域被填满，你将会得到一个java.lang.OutOfMemoryError: PermGen Space错误。    

>从Java 8开始，PermGen Space被移除并被Metaspace所取代，类的元数据现在被存储在Metaspace中。Metaspace位于本地内存（不是在Java虚拟机堆内存中），因此其最大值受限于你的系统内存。但是，仍然存在溢出的可能性。当Metaspace填满时，你会得到一个java.lang.OutOfMemoryError: Metaspace错误。  

>为什么动态加载类会导致这个问题？这是因为每当一个类被加载到Java虚拟机中时，关于这个类的元数据都会被存储在Metaspace或PermGen Space中。如果你在程序运行过程中动态加载大量的类，并且这些类在之后没有被卸载，那么这些类的元数据会持续占用内存，最终可能导致内存溢出。        

>解决这个问题的一个办法是增大Metaspace或PermGen Space的大小。你可以通过Java虚拟机的启动参数来实现，例如，-XX:MaxMetaspaceSize=512m或者-XX:MaxPermSize=512m。另外一个办法是确保你的应用程序在加载类之后，当类不再需要时可以正确地卸载它们。 
