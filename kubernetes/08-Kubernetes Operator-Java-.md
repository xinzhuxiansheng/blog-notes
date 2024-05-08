# Kubernetes Operator - Java - 实战     

## 引言     
为了更好介绍 `Flink Kubernetes Operator`的相关实现细节，我们可以`开发一个0到1 的Java Operator`。                  


## 开发 

### 1.根据模板创建项目    
```shell
mvn io.javaoperatorsdk:bootstrapper:4.8.3:create -DprojectGroupId=com.xinzhuxiansheng.javaproject -DprojectArtifactId=javamain-k8sOperator  
```



<operator.sdk.version>4.8.3</operator.sdk.version>

<dependency>
    <groupId>io.javaoperatorsdk</groupId>
    <artifactId>operator-framework</artifactId>
    <version>${operator.sdk.version}</version>
</dependency>       


refer       
1.https://javaoperatorsdk.io/docs/getting-started               
2.https://github.com/eugenp/tutorials/tree/master/kubernetes-modules/k8s-operator                   
3.https://www.baeldung.com/java-kubernetes-operator-sdk             
