## SeaTunnel本地调试环境搭建        

>version: 2.3.2 

### 引言    
在本篇介绍如何为 SeaTunnel 设置开发环境，然后在 JetBrains IntelliJ IDEA 中运行一个简单的示例。您可以在任何您喜欢的开发环境中开发或测试 SeaTunnel 代码，但这里我们以 JetBrains IDEA 为例，一步步教您环境。   

### 环境准备    
在开始讲如何搭建环境之前，我们需要做一些准备工作。确保您已经安装了以下软件：    
* 已安装 Git。
* 安装Java（现已支持JDK8/JDK11）并设置JAVA_HOME。
* 已安装 Scala（目前仅支持 scala 2.11.12）。
* 已安装 JetBrains IDEA。   

### 导入项目    

**1.克隆源码**      
```shell
## git clone
git clone git@github.com:apache/seatunnel.git   

## 请确保是dev分支 
git checkout -b xxx-dev origin/dev
```

>**这里留个TODO** ，在SeaTunnel源码搭建过程中发现，非`dev`分支下，总是出乎所料的有两个version，这让我导入项目过程中算是瞎扯腾了好一会。 或者，你不妨关注该issue （https://github.com/apache/seatunnel/issues/4896），内容也同样没有该如何操作，因为，用了2.3.2版本，肯定希望关注的是 tag 2.3.2的源码，不明白为什么，等深入研究后，再做分析。     
     

**2.在Idea中安装Scala插件**     
安装Scala插件（https://plugins.jetbrains.com/plugin/1347-scala）    

**3.在Idea中安装Lombok插件**        
安装Lombok插件（https://plugins.jetbrains.com/plugin/6317-lombok）  

**导入Code Style**  
Apache SeaTunnel 使用 Spotless 进行代码样式和格式检查。您可以运行以下命令，Spotless 会自动为您修复代码样式和格式错误：  
```shell
./mvnw spotless:apply   
```

您可以将预提交挂钩文件 `/tools/spotless_check/pre-commit.sh` 复制到您的 `.git/hooks/` 目录中，以便每次您使用 git commit 提交代码时，Spotless 都会自动为您修复问题。     


**4.Maven相关命令**         
```shell
## 先执行mvn install 编译并安装 jar到本地仓库   
mvn clean install -DskipTests   
```

>注意 Idea导入项目后，首先一定要`mvn install` 否则代码无法正常运行，若代码出现部分类找不到情况，请一定，一定，一定，记得重新刷新下Maven项目     
![debug01](http://img.xinzhuxiansheng.com/blogimgs/seatunnel/debug01.png)  

**需特别注意！！！**        

### 跑通示例代码    
完成上述所有操作后，您只需完成环境设置即可运行我们为您提供的开箱即用的示例。所有示例都在模块 Seatunnel-examples 中，您可以选择您感兴趣的一个，根据需要在 IDEA 中运行或调试它。
这里我们以 `Seatunnel-examples/seatunnel-flink-connector-v2-example/src/main/java/org/apache/seatunnel/example/flink/v2/SeaTunnelApiExample.java` 为例，运行成功后可以看到输出如下：    
```
2023-06-30 23:02:07,326 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=1  rowIndex=4:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : jpEgH, 2036822176
2023-06-30 23:02:07,326 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=2  rowIndex=4:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : KNaVA, 1295072156
...
...
2023-06-30 23:02:07,327 INFO  org.apache.seatunnel.connectors.seatunnel.console.sink.ConsoleSinkWriter - subtaskIndex=1  rowIndex=11:  SeaTunnelRow#tableId= SeaTunnelRow#kind=INSERT : lxclk, 1523020468
```

refer   
1.https://seatunnel.apache.org/docs/2.3.2/contribution/setup