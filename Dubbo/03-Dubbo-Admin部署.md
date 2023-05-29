## Dubbo Admin 部署

### 部署流程    

**1.** 克隆 dubbo-admin项目源码,地址为：https://github.com/apache/dubbo-admin， 不过请注意，dubbo-admin仓库默认分支是 `refactor-with-go
`， 根据README.md介绍：  
```
此分支是 Dubbo Admin 正在基于 Go 语言重构的开发分支，目前仍在开发过程中。 如您正寻求将 Dubbo Admin 用作生产环境，想了解 Admin 的能力及安装方式，请参见 develop 分支 及内部相关使用说明。
``` 

所以checkout到`develop`分支，并且它是java编写   


**2.** 修改配置参数（改不改可自行决定）    
配置文件路径在：`dubbo-admin\dubbo-admin-server\src \main\resources lapplication.properties` ,修改以下参数：        

```shell
# 修改端口
server.port=38080

# 修改zookeeper地址
admin.registry.address=zookeeper://127.0.0.1:2181
admin.config-center=zookeeper://127.0.0.1:2181
admin.metadata-report.address=zookeeper://127.0.0.1:2181

# 修改账号密码
admin.root.user.name=root
admin.root.user.password=root
```

**3.** 编译打包 
```shell
mvn clean package
``` 
编译后的jar包在`dubbo-admin\dubbo-admin-distribution\target`目录下  

**4.** 运行
```shell
java -jar xxx.jar
```


### Q&A

**1.** No compiler is provided in this environment. Perhaps you are running on a JRE rather than a JDK?
A:
请确保本地环境是否配置JAVA_HOME的环境变量