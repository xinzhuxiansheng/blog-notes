# Flink 源码 - Standalone - 应用 JDBC Catalog                

>Flink version: 1.17.2       

## 背景  

## 配置 JDBC Catalog   
参考官网文档 https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/connectors/table/jdbc/#dependencies，在`lib/`目录下添加相关依赖。我配置的是 MySQL，我添加的依赖如下：   
```bash
flink-connector-jdbc-3.1.2-1.17.jar
mysql-connector-j-8.0.33.jar
```


https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/connectors/table/jdbc/#dependencies    



refer       
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/connectors/table/jdbc/#jdbc-catalog     
