## SQLLine 

### 介绍 
SQLLine是一个基于Java开发的连接关系数据库和执行SQL语句的控制台工具(shell) 。   

### SQLLine 源码编译    
目前 SQLLine没有提供编译好的Jar，所以需自行编译打包，命令如下： 
```shell
git clone git://github.com/julianhyde/sqlline.git
cd sqlline
./mvnw package # 也可替换成本地环境Maven  mvn package
``` 

### SQLLine的基本操作   

启动 SQLLine      
```shell
# 1.进入编译后 target 文件假    
java -jar sqlline-1.13.0-SNAPSHOT-jar-with-dependencies.jar 
```

`后续操作，是启动SQLLine 后，再执行`    

SQLLine是专门用来进行数据库的连接的，因此它支持用户通过连接数据库的命令，以JDBC的数据库连接协议对数据库进行连接。   
SQLLine定义数据库连接的命令模板： 
```
!Connect jdbc:<jdbc_url> <user_name> <password>[driver_name]
```
其中，`jdbc_url`是适配用户使用的数据库的JDBC协议地址，是一个必填项，但是在这之前需要加上“jdbc:”，而且一般情况下，数据库的名称会成为这个URL中的一部分。后面紧跟着的`user_name`和`password`是用户名和密码，这两个配置同样是必填项。有时为了防止用户名和密码内部存在特殊字符可以使用引号将其包裹，当然此处既可以使用单引号，也可以使用双引号。最后的`driver_name`是使用的驱动名称，是一个可选项。           
利用SQLLine来连接数据库的命令： 
```
!Connect jdbc:calcite:model=src/test/resources/model.json admin admin   
```     
接着就可以开始查看元数据信息。值得注意的是，在SQLLine中，相关的命令与MySQL中的不同，如果需要查看所有表的元数据信息，则需要使用“!tables”命令来查看；如果需要查看某张表里面所有的字段元数据信息，则需要使用“!columns”命令或者“!describe”命令来查看。  


refer   
1.《Calcite数据管理实战》      
