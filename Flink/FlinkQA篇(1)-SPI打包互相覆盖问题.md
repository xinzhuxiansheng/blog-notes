

## Jar作业打包时，SPI互相覆盖问题

环境：Flink on Kubernetes Application Mode Jar作业，flink v: 1.14.4, flink cdc v:2.2.0
背景：使用DataSteam API开发Flink CDC Mysql to Mysql数据同步
问题：项目在Idea中运行没有问题，可打包提交到Flink后运行报错，JobManager Pod报错，查看日志提示未找到SQL对应connector的SPI定义。

### 错误日志
JobManager Pod日志，`kubectl -n xxx log ....`
```
Caused by: org.apache.flink.table.api.ValidationException: Could not find any factory for identifier 'mysql-cdc' that implements 'org.apache.flink.table.factories.DynamicTableFactory' in the classpath.
Available factory identifiers are:
blackhole
datagen
filesystem
print
	at org.apache.flink.table.factories.FactoryUtil.discoverFactory(FactoryUtil.java:399) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.flink.table.factories.FactoryUtil.enrichNoMatchingConnectorError(FactoryUtil.java:583) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.flink.table.factories.FactoryUtil.getDynamicTableFactory(FactoryUtil.java:561) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.flink.table.factories.FactoryUtil.createTableSource(FactoryUtil.java:146) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.flink.table.planner.plan.schema.CatalogSourceTable.createDynamicTableSource(CatalogSourceTable.java:116) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.flink.table.planner.plan.schema.CatalogSourceTable.toRel(CatalogSourceTable.java:82) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.calcite.sql2rel.SqlToRelConverter.toRel(SqlToRelConverter.java:3585) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.calcite.sql2rel.SqlToRelConverter.convertIdentifier(SqlToRelConverter.java:2507) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.calcite.sql2rel.SqlToRelConverter.convertFrom(SqlToRelConverter.java:2144) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.calcite.sql2rel.SqlToRelConverter.convertFrom(SqlToRelConverter.java:2093) ~[flink-table_2.11-1.14.4.jar:1.14.4]
	at org.apache.calcite.sql2rel.SqlToRelConverter.convertFrom(SqlToRelConverter.java:2050) ~[flink-table_2.11-1.14.4.jar:1.14.4]
```

### 定位过程
脑补下这块知识：每个connector的源码中在`resources`下的META-INFO.services目录定义SPI，所以这块通过`JD-GUI`确认项目打的fatjar是否对应的mysql-cdc SPI，剩下就是mysql-cdc的SPI叫啥，访问Flink CDC github查看。  下面给出截图说明：      

**flink-cdc-connectors**
![QA1flinkcdc源码01](http://img.xinzhuxiansheng.com/blogimgs/flink/QA1flinkcdc源码01.jpg)      

----    

**使用JD-GUI反编译工具**
![QA1反编译service01](http://img.xinzhuxiansheng.com/blogimgs/flink/QA1反编译service01.jpg)    

通过以上工具，分别将 `flink-table-planner`,`flink-connector-jdbc`的jar注释后打包发现，`META-INF.services.org.apache.flink.table.factories.Factory`的SPI内容一直再变化，后来发现它们与`flink-connector-mysql-cdc`的SPI名称冲突，导致在打包的时候，会相互覆盖内容。       

所以剩下的就是解决如何将同名的SPI内容合并？ 

### 问题解决
项目使用的打包插件是`maven-shade-plugin`，SPI的合并需要在shade插件配置transformers，而其中ServicesResourceTransformer是解决SPI文件的合并。      
```xml
<!--在transformers节点配置-->
<transformer implementation="org.apache.maven.plugins.shade.resource.ServicesResourceTransformer"/>
```

**完整配置如下：**
```xml
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-shade-plugin</artifactId>
    <version>3.0.0</version>
    <executions>
        <!-- Run shade goal on package phase -->
        <execution>
            <phase>package</phase>
            <goals>
                <goal>shade</goal>
            </goals>
            <configuration>
                <artifactSet>
                    <excludes>
                        <exclude>org.apache.flink:force-shading</exclude>
                        <exclude>com.google.code.findbugs:jsr305</exclude>
                        <exclude>org.slf4j:*</exclude>
                        <exclude>log4j:*</exclude>
                    </excludes>
                </artifactSet>
                <filters>
                    <filter>
                        <!-- Do not copy the signatures in the META-INF folder.
                        Otherwise, this might cause SecurityExceptions when using the JAR. -->
                        <artifact>*:*</artifact>
                        <excludes>
                            <exclude>META-INF/*.SF</exclude>
                            <exclude>META-INF/*.DSA</exclude>
                            <exclude>META-INF/*.RSA</exclude>
                        </excludes>
                    </filter>
                </filters>
                <transformers>
                    <transformer implementation="org.apache.maven.plugins.shade.resource.ServicesResourceTransformer"/>
                    <transformer
                            implementation="org.apache.maven.plugins.shade.resource.ManifestResourceTransformer">
                        <!--                                    <mainClass>com.yzhou.perf.KafkaProducerPerfTestJob</mainClass>-->
                    </transformer>
                </transformers>
            </configuration>
        </execution>
    </executions>
</plugin>
``` 

最后打包后的SPI合并内容如下：
![QA1反编译service02](http://img.xinzhuxiansheng.com/blogimgs/flink/QA1反编译service02.jpg)    