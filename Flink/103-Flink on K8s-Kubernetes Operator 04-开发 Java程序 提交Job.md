# Flink on Kubernetes - Kubernetes Operator - 开发 Java程序提交 Job

>Operator version: 1.8，Flink version: 1.17   

## 添加依赖 
1.添加 flink kubernetes operator Jar        
```xml
<!-- https://mvnrepository.com/artifact/org.apache.flink/flink-kubernetes-operator -->
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-kubernetes-operator</artifactId>
    <version>1.8.0</version>
</dependency>
```

2.添加 io.fabric8.kubernetes-client Jar     
```xml
<!-- https://mvnrepository.com/artifact/io.fabric8/kubernetes-client -->
<dependency>
    <groupId>io.fabric8</groupId>
    <artifactId>kubernetes-client</artifactId>
    <version>6.4.0</version>
</dependency>   
```

