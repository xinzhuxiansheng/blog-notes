# Flink 源码 - 编译异常 Could not transfer artifact org.pentaho:pentaho-aggdesigner-algorithm:pom:5.1.5-jhyde from/to conjars (http://conjars.org/repo): conjars.org: Unknown host conjars.org 

>Flink version: 1.15.4  

## 问题描述  
```bash
[ERROR] Failed to execute goal on project flink-connector-hive_2.12: Could not resolve dependencies for project org.apache.flink:flink-connector-hive_2.12:jar:1.15.4: Failed to collect dependencies at org.apache.hive:hive-exec:jar:2.3.9 -> org.pentaho:pentaho-aggdesigner-algorithm:jar:5.1.5-jhyde: Failed to read artifact descriptor for org.pentaho:pentaho-aggdesigner-algorithm:jar:5.1.5-jhyde: The following artifacts could not be resolved: org.pentaho:pentaho-aggdesigner-algorithm:pom:5.1.5-jhyde (present, but unavailable): Could not transfer artifact org.pentaho:pentaho-aggdesigner-algorithm:pom:5.1.5-jhyde from/to conjars (http://conjars.org/repo): conjars.org: Unknown host conjars.org -> [Help 1]
[ERROR] 
[ERROR] To see the full stack trace of the errors, re-run Maven with the -e switch.
[ERROR] Re-run Maven using the -X switch to enable full debug logging.
[ERROR] 
[ERROR] For more information about the errors and possible solutions, please read the following articles:
[ERROR] [Help 1] http://cwiki.apache.org/confluence/display/MAVEN/DependencyResolutionException
[ERROR] 
[ERROR] After correcting the problems, you can resume the build with the command
[ERROR]   mvn <args> -rf :flink-connector-hive_2.12
```  

## 解决  
在 flink 项目根目录下的 `pom.xml` 添加以下内容  
```xml
<repositories>
    <repository>
        <id>repository.jboss.org</id>
        <url>http://repository.jboss.org/nexus/content/groups/public/</url>
        <snapshots>
            <enabled>false</enabled>
        </snapshots>
        <releases>
            <enabled>false</enabled>
        </releases>
    </repository>  

    <!-- 增加 以下配置 -->
    <repository>
        <id>spring</id>
        <url>https://maven.aliyun.com/repository/spring</url>
    </repository>
</repositories>
``` 
