
# 安装部署

>streamx-console 是一个综合实时数据平台，低代码 ( Low Code ) ,Flink Sql 平台，可以较好的管理 Flink 任务，集成了项目编译、发布、参数配置、启动、savepoint,火焰图 ( flame graph ) ,Flink SQL,监控等诸多功能于一体，大大简化了 Flink 任务的日常操作和维护，融合了诸多最佳实践。其最终目标是打造成一个实时数仓，流批一体的一站式大数据解决方案。


按照官网描述，步骤安装及配置`streamx-console`服务

## 源码安装
涉及到源码编译,有以下要求：
Maven 3.6+
npm 7.11.2 ( https://nodejs.org/en/ )
JDK 1.8+

**编译**
```shell
git clone https://github.com/streamxhub/streamx.git
cd Streamx
mvn clean install -DskipTests -Denv=prod
``` 
安装完成之后就看到最终的工程文件，位于 streamx/streamx-console/streamx-console-service/target/streamx-console-service-1.0.0-bin.tar.gz。

**配置mysql**
参考官网步骤

>请关注以下FAQ

## FAQ
1. MySQL表主键问题？
由于streamx使用了flyway来保持版本平稳升级，目的自动升级sql ddl，这样带来极大的方便, 但是：有时我们需要修改历史sql ddl文件，这会带来一些问题, 通常情况下ddl sql只有DBA才用权限去执行,普通用户没有执行ddl的权限，这会导致flyway自动升级失败,也是问题。    
可能存在问题是：
DBA所管理的MySQL要求表必须包含主键，而streamx-console涉及的表有 `t_role_menu` 没有主键，这反而造成 服务启动不了，报错， 我也向社区提了issue https://github.com/streamxhub/streamx/issues/482 。 

异常信息如下： 
```
2021-12-02 00:05:05 | ERROR | main | org.springframework.boot.SpringApplication ] Application run failed
org.springframework.beans.factory.UnsatisfiedDependencyException: Error creating bean with name 'flinkSqlController': Unsatisfied dependency expressed through field 'flinkSqlService'; nested exception is org.springframework.beans.factory.UnsatisfiedDependencyException: Error creating bean with name 'flinkSqlServiceImpl': Unsatisfied dependency expressed through field 'backUpService'; nested exception is org.springframework.beans.factory.UnsatisfiedDependencyException: Error creating bean with name 'applicationBackUpServiceImpl': Unsatisfied dependency expressed through field 'applicationService'; nested exception is org.springframework.beans.factory.BeanCreationException: Error creating bean with name 'flywayInitializer' defined in class path resource [org/springframework/boot/autoconfigure/flyway/FlywayAutoConfiguration$FlywayConfiguration.class]: Invocation of init method failed; nested exception is org.flywaydb.core.internal.command.DbMigrate$FlywayMigrateException: 
Migration V1_1__init_data.sql failed
------------------------------------
SQL State  : HY000
 

省略.........

------------------------------------
SQL State  : HY000
Error Code : 3098
Message    : The table does not comply with the requirements by an external plugin.
Location   : db/migration/V1_1__init_data.sql (/data/streamx-console-service-1.2.0/bin/file:/data/streamx-console-service-1.2.0/lib/streamx-console-service-1.2.0.jar!/db/migration/V1_1__init_data.sql)
Line       : 78
Statement  : -- ----------------------------
-- Records of t_role_menu
-- ----------------------------
INSERT INTO `t_role_menu` VALUES (1, 1)

        at org.flywaydb.core.internal.sqlscript.DefaultSqlScriptExecutor.handleException(DefaultSqlScriptExecutor.java:253)
        at org.flywaydb.core.internal.sqlscript.DefaultSqlScriptExecutor.executeStatement(DefaultSqlScriptExecutor.java:202)
        at org.flywaydb.core.internal.sqlscript.DefaultSqlScriptExecutor.execute(DefaultSqlScriptExecutor.java:125)
        at org.flywaydb.core.internal.resolver.sql.SqlMigrationExecutor.execute(SqlMigrationExecutor.java:77)
        at org.flywaydb.core.internal.command.DbMigrate.doMigrateGroup(DbMigrate.java:367)
        ... 76 common frames omitted
Caused by: java.sql.SQLException: The table does not comply with the requirements by an external plugin.
        at com.mysql.cj.jdbc.exceptions.SQLError.createSQLException(SQLError.java:129)
        at com.mysql.cj.jdbc.exceptions.SQLError.createSQLException(SQLError.java:97)
        at com.mysql.cj.jdbc.exceptions.SQLExceptionsMapping.translateException(SQLExceptionsMapping.java:122)
        at com.mysql.cj.jdbc.StatementImpl.executeInternal(StatementImpl.java:764)
        at com.mysql.cj.jdbc.StatementImpl.execute(StatementImpl.java:648)
        at com.zaxxer.hikari.pool.ProxyStatement.execute(ProxyStatement.java:95)
        at com.zaxxer.hikari.pool.HikariProxyStatement.execute(HikariProxyStatement.java)
        at org.flywaydb.core.internal.jdbc.JdbcTemplate.executeStatement(JdbcTemplate.java:235)
        at org.flywaydb.core.internal.sqlscript.StandardSqlStatement.execute(StandardSqlStatement.java:42)
        at org.flywaydb.core.internal.sqlscript.DefaultSqlScriptExecutor.executeStatement(DefaultSqlScriptExecutor.java:189)
        ... 79 common frames omitted
2021-12-02 00:05:05 | INFO  | Thread-0 | com.streamxhub.streamx.console.StreamXConsole ] [StreamX] application shutdown now, pid: 7953
```

>离线安装MySQL，请参考：https://github.com/xinzhuxiansheng/blog-notes/blob/master/MySQL/Mysql%208%E7%A6%BB%E7%BA%BF%E5%AE%89%E8%A3%85.md

