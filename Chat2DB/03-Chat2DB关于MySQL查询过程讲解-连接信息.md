## Chat2DB关于MySQL查询过程讲解 - 连接信息

>Chat2DB version: 1.0.9 

### 引言    
在上一篇《Chat2DB关于MySQL查询过程讲解 - 连接测试》介绍了“测试连接”功能，本篇主要介绍“连接”功能的实现以及数据是如何被持久化。   

![addLink01](http://img.xinzhuxiansheng.com/blogimgs/chat2db/addLink01.png)   

### “连接”  
Chat2DB服务在配置MySQL`连接`后，该连接信息就好像被持久化了一样，因为服务在启动时，并未指定MySQL连接地址，反复启动都会发现之前配置的MySQL连接信息在页面都显示，那接下来我们就探索它是如何实现的？    

![addLink02](http://img.xinzhuxiansheng.com/blogimgs/chat2db/addLink02.png)          

`Rest API`      
```shell
http://127.0.0.1:10821/api/connection/datasource/create         

# POST 参数
{"ssh":{"use":"false","hostName":"","port":"","userName":"","localPort":"","password":""},"alias":"@localhost","host":"localhost","port":"3306","authentication":1,"user":"root","password":"12345678","database":"","url":"jdbc:mysql://localhost:3306/","jdbc":"8.0","extendInfo":[{"key":"zeroDateTimeBehavior","value":"convertToNull"}],"EnvType":"DAILY","type":"MYSQL"}
``` 

>汲取上一篇《Chat2DB关于MySQL查询过程讲解 - 连接测试》源码分析经验，Chat2DB中`***Converter`是做是实体类的转换，所以后面就不再过多介绍，除非有特殊用法。      

### DataSourceServiceImpl#create()  
```java
@Override
public DataResult<Long> create(DataSourceCreateParam param) {
    DataSourceDO dataSourceDO = dataSourceConverter.param2do(param);
    dataSourceDO.setGmtCreate(LocalDateTime.now());
    dataSourceDO.setGmtModified(LocalDateTime.now());
    dataSourceMapper.insert(dataSourceDO);
    return DataResult.of(dataSourceDO.getId());
}
``` 

这段代码，其实也没啥好介绍的，dataSourceMapper是Mybatis的Mapper映射，然后集成了Mybatis-plus对DB进行插入，Chat2DB服务配置的是H2数据库。我们看下`application-dev.yml`配置内容：   
```yml
spring:
  datasource:
    # 配置自带数据库的相对路径
    url: jdbc:h2:~/.chat2db/db/chat2db_dev;MODE=MYSQL
    driver-class-name: org.h2.Driver
  #  用于数据库表结构版本管理
  flyway:
    locations: classpath:db/migration/dev
  h2:
    console:
      enabled: true
      path: /h2
      settings:
        trace: true
        web-allow-others: true
# 端口号
server:
  port: 10821
```

基于以上内容，Chat2DB使用H2Database作为DB存储，利用Mybatis-plus做ORM操作，下面介绍下Chat2DB是如何集成H2Database?    

### 集成H2Database      
**在pom.xml添加h2依赖、flayway依赖**   
```xml
<!-- 启动数据库 -->
<dependency>
    <groupId>com.h2database</groupId>
    <artifactId>h2</artifactId>
</dependency>

``` 

**spring boot集成h2**    
```yml
spring:
  datasource:
    # 配置自带数据库的相对路径
    url: jdbc:h2:~/.chat2db/db/chat2db_dev;MODE=MYSQL
    driver-class-name: org.h2.Driver
  h2:
    console:
      enabled: true
      path: /h2
      settings:
        trace: true
        web-allow-others: true
```
`配置讲解`  
* spring.datasource.url：数据库的路径，而`MODE=MYSQL`参数是告诉H2Database模拟MySQL的行为，这里面包含以下：  
1.支持MySQL特有的SQL语言    
2.MySQL的数据类型映射到H2的相应数据类型 
3.识别MySQL的系统函数   
所以这样服务的SQL查询也同样能兼容MySQL数据库。  
* spring.datasource.url.driver-class-name: 连接驱动名称   
* h2.console.enabled: 这个属性用来启动 H2 控制台。如果设置为 true，则在应用启动时，H2 控制台也会一起启动    
* h2.console.path: 这个属性用来设置 H2控制台的访问路径。在上面的配置中，H2 控制台的访问路径被设置为 "/h2"，所以你可以通过访问 `http://localhost:10821/h2` 来打开 H2 控制台（Chat2DB服务端口号为 10821） 
* h2.console.settings.trace: 这个属性用来控制 H2 控制台是否打印跟踪信息。如果设置为 true，则 H2 控制台会打印详细的 SQL 执行信息，这对于调试是非常有用的。  
* h2.console.settings.web-allow-others: 如果此属性被设置为 true，那么不在服务器主机上的客户端也可以访问 H2 控制台。如果你的服务器不在本地运行，或者你需要从其他机器上访问 H2 控制台，那么你就需要开启这个设置。不过请注意，如果你没有使用额外的安全措施（比如 VPN 或防火墙）来保护你的 H2 控制台，那么这样做可能会带来安全风险。    

通过了解H2Database配置可知，H2Database提供了web console页面功能，能够查看和操作DB, 访问`http://localhost:10821/h2`即可。        
![addLink03](http://img.xinzhuxiansheng.com/blogimgs/chat2db/addLink03.png) 

>注意，Chat2DB在配置H2参数时，并没有配置username,password, 所以在页面连接时，**username，password为空**，`JDBC URL`配置成`jdbc:h2:~/.chat2db/db/chat2db_dev;MODE=MYSQL`即可。   

![addLink04](http://img.xinzhuxiansheng.com/blogimgs/chat2db/addLink04.png)      

#### 集成flayway 
我们先看下启动日志，在日志中能看到`Found resource: db`,`Validating V1_0_2__修改`等字样，这其实就是flayway的作用
```
2023-06-07 16:57:29.124  INFO 21952 --- [           main] c.a.d.s.s.config.oauth.SaLogForSlf4j     : 会话组件 StpLogic(type=login) 重置成功: class cn.dev33.satoken.jwt.StpLogicJwtForStateless
2023-06-07 16:57:29.130  INFO 21952 --- [           main] c.a.d.s.s.config.oauth.SaLogForSlf4j     : 全局组件 SaJsonTemplate 载入成功: cn.dev33.satoken.spring.json.SaJsonTemplateForJackson
2023-06-07 16:57:29.139  INFO 21952 --- [           main] c.a.d.s.s.config.oauth.SaLogForSlf4j     : 全局组件 SaTokenContext 载入成功: cn.dev33.satoken.spring.SaTokenContextForSpring
2023-06-07 16:57:29.415  INFO 21952 --- [           main] o.f.c.internal.license.VersionPrinter    : Flyway Community Edition 9.8.1 by Redgate
2023-06-07 16:57:29.416  INFO 21952 --- [           main] o.f.c.internal.license.VersionPrinter    : See what's new here: https://flywaydb.org/documentation/learnmore/releaseNotes#9.8.1
2023-06-07 16:57:29.416  INFO 21952 --- [           main] o.f.c.internal.license.VersionPrinter    : 
2023-06-07 16:57:29.419 DEBUG 21952 --- [           main] o.f.core.internal.util.FeatureDetector   : AWS SDK available: false
2023-06-07 16:57:29.420 DEBUG 21952 --- [           main] o.f.core.internal.util.FeatureDetector   : Google Cloud Storage available: false
2023-06-07 16:57:29.421 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Scanning for classpath resources at 'classpath:db/callback' ...
2023-06-07 16:57:29.421 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Determining location urls for classpath:db/callback using ClassLoader jdk.internal.loader.ClassLoaders$AppClassLoader@63947c6b ...
2023-06-07 16:57:29.422 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Unable to resolve location classpath:db/callback.
2023-06-07 16:57:29.422 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Scanning for classpath resources at 'classpath:db/migration/dev' ...
2023-06-07 16:57:29.422 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Determining location urls for classpath:db/migration/dev using ClassLoader jdk.internal.loader.ClassLoaders$AppClassLoader@63947c6b ...
2023-06-07 16:57:29.423 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Scanning URL: file:/E:/Code/Java/Chat2DB/ali-dbhub-server/ali-dbhub-server-start/target/classes/db/migration/dev
2023-06-07 16:57:29.424 DEBUG 21952 --- [           main] o.f.core.internal.util.FeatureDetector   : JBoss VFS v2 available: false
2023-06-07 16:57:29.426 DEBUG 21952 --- [           main] i.s.c.FileSystemClassPathLocationScanner : Scanning starting at classpath root in filesystem: E:\Code\Java\Chat2DB\ali-dbhub-server\ali-dbhub-server-start\target\classes\
2023-06-07 16:57:29.426 DEBUG 21952 --- [           main] i.s.c.FileSystemClassPathLocationScanner : Scanning for resources in path: E:\Code\Java\Chat2DB\ali-dbhub-server\ali-dbhub-server-start\target\classes\db\migration\dev (db/migration/dev)
2023-06-07 16:57:29.428 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Found resource: db/migration/dev/V1_0_0_1__初始化信息-新增用户.sql
2023-06-07 16:57:29.428 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Found resource: db/migration/dev/V1_0_0__初始化信息.sql
2023-06-07 16:57:29.428 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Found resource: db/migration/dev/V1_0_1__配置信息.sql
2023-06-07 16:57:29.429 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Found resource: db/migration/dev/V1_0_2__修改Console.sql
2023-06-07 16:57:29.429 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Found resource: db/migration/dev/V1_0_3_0__增加SSH.sql
2023-06-07 16:57:29.429 DEBUG 21952 --- [           main] o.f.c.i.s.classpath.ClassPathScanner     : Scanning for classes at classpath:db/migration/dev
2023-06-07 16:57:29.433 DEBUG 21952 --- [           main] o.f.c.i.resource.ResourceNameValidator   : Validating V1_0_1__配置信息.sql
2023-06-07 16:57:29.434 DEBUG 21952 --- [           main] o.f.c.i.resource.ResourceNameValidator   : Validating V1_0_0__初始化信息.sql
2023-06-07 16:57:29.435 DEBUG 21952 --- [           main] o.f.c.i.resource.ResourceNameValidator   : Validating V1_0_2__修改Console.sql
2023-06-07 16:57:29.435 DEBUG 21952 --- [           main] o.f.c.i.resource.ResourceNameValidator   : Validating V1_0_3_0__增加SSH.sql
2023-06-07 16:57:29.435 DEBUG 21952 --- [           main] o.f.c.i.resource.ResourceNameValidator   : Validating V1_0_0_1__初始化信息-新增用户.sql
2023-06-07 16:57:29.445  INFO 21952 --- [           main] o.f.c.i.database.base.BaseDatabaseType   : Database: jdbc:h2:~/.chat2db/db/chat2db_dev (H2 2.1)
2023-06-07 16:57:29.446 DEBUG 21952 --- [           main] o.f.c.i.database.base.BaseDatabaseType   : Driver  : H2 JDBC Driver 2.1.214 (2022-06-13)
2023-06-07 16:57:29.491 DEBUG 21952 --- [           main] org.flywaydb.core.FlywayExecutor         : DDL Transactions Supported: false
2023-06-07 16:57:29.492 DEBUG 21952 --- [           main] o.f.c.i.s.SchemaHistoryFactory           : Schemas: 
2023-06-07 16:57:29.493 DEBUG 21952 --- [           main] o.f.c.i.s.SchemaHistoryFactory           : Default schema: null
2023-06-07 16:57:29.500 DEBUG 21952 --- [           main] o.f.c.i.c.SqlScriptCallbackFactory       : Scanning for SQL callbacks ...
2023-06-07 16:57:29.510 DEBUG 21952 --- [           main] o.f.core.internal.command.DbValidate     : Validating migrations ...
2023-06-07 16:57:29.532 DEBUG 21952 --- [           main] o.f.core.internal.scanner.Scanner        : Filtering out resource: db/migration/dev/V1_0_1__配置信息.sql (filename: V1_0_1__配置信息.sql)
2023-06-07 16:57:29.532 DEBUG 21952 --- [           main] o.f.core.internal.scanner.Scanner        : Filtering out resource: db/migration/dev/V1_0_0__初始化信息.sql (filename: V1_0_0__初始化信息.sql)
2023-06-07 16:57:29.532 DEBUG 21952 --- [           main] o.f.core.internal.scanner.Scanner        : Filtering out resource: db/migration/dev/V1_0_2__修改Console.sql (filename: V1_0_2__修改Console.sql)
2023-06-07 16:57:29.532 DEBUG 21952 --- [           main] o.f.core.internal.scanner.Scanner        : Filtering out resource: db/migration/dev/V1_0_3_0__增加SSH.sql (filename: V1_0_3_0__增加SSH.sql)
2023-06-07 16:57:29.532 DEBUG 21952 --- [           main] o.f.core.internal.scanner.Scanner        : Filtering out resource: db/migration/dev/V1_0_0_1__初始化信息-新增用户.sql (filename: V1_0_0_1__初始化信息-新增用户.sql)
2023-06-07 16:57:29.549  INFO 21952 --- [           main] o.f.core.internal.command.DbValidate     : Successfully validated 5 migrations (execution time 00:00.036s)
2023-06-07 16:57:29.551 DEBUG 21952 --- [           main] o.f.core.internal.command.DbSchemas      : Skipping creation of existing schema: "PUBLIC"
2023-06-07 16:57:29.558  INFO 21952 --- [           main] o.f.core.internal.command.DbMigrate      : Current version of schema "PUBLIC": 1.0.3.0
2023-06-07 16:57:29.559  INFO 21952 --- [           main] o.f.core.internal.command.DbMigrate      : Schema "PUBLIC" is up to date. No migration necessary.
2023-06-07 16:57:29.561 DEBUG 21952 --- [           main] org.flywaydb.core.FlywayExecutor         : Memory usage: 152 of 292M
```

**添加pom依赖以及配置参数**
```xml
<!-- 数据库版本管理 -->
<dependency>
    <groupId>org.flywaydb</groupId>
    <artifactId>flyway-core</artifactId>
</dependency>
<dependency>
    <groupId>org.flywaydb</groupId>
    <artifactId>flyway-mysql</artifactId>
</dependency>
```

```yml
spring:
  #  用于数据库表结构版本管理
  flyway:
    locations: classpath:db/migration/dev
```
* spring.datasource.flyway.locations:    
>Flyway是一个用于数据库迁移（Database Migration）的开源工具，它通过跟踪你的数据库的变更历史来帮助你管理数据库的版本。你可以使用SQL脚本或者Java代码来描述数据库的变更，Flyway会自动将这些变更应用到数据库中，而且还能确保这些变更的顺序正确。    
下面是关于`flyway-core`和`flyway-mysql`的简单解释： 
1.`flyway-core`：这是Flyway的核心库，包含了所有基础的功能，包括迁移脚本的解析、迁移历史的管理、迁移操作的执行等等。无论你使用哪种数据库，都需要引入这个库。    
2.`flyway-mysql`：这个库是用来支持MySQL数据库的。  
我们可以看到flyway.locations配置的目录下是数据库的DDL SQL语句。


**在指定目录中创建SQL脚本** 
创建数据库迁移脚本：在你配置的数据库迁移脚本的路径下，创建数据库迁移脚本。Flyway支持SQL脚本和 Java-based migrations。SQL 脚本的命名应遵循 V<VERSION>__<NAME>.sql 的格式，例如 V1__create_table.sql。 

![addLink05](http://img.xinzhuxiansheng.com/blogimgs/chat2db/addLink05.png)      

### 总结    
Chat2DB服务默认使用H2Database作为存储，并且在服务启动时，通过flyway来初始化DB表结构以及做到自动变更DDL。    




