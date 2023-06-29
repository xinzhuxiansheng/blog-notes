## Connector依赖包是如何通过install-plugin脚本下载的    

>version: 2.3.2 

### 引言    
SeaTunnel在部署过程中，需要通过脚本或者手动下载多个`Connector`, 其实本身没必要特别详细介绍，主要是想记录下来，不同项目又是如何管理依赖jar的。例如在`Chat2DB`项目中，通过在yaml配置文件中配置jar的下载地址， 而SeaTunnel是通过 Maven来下载。各自的实现不同。 

`Chat2DB`   

![chat2dbjar01](http://img.xinzhuxiansheng.com/blogimgs/seatunnel/chat2dbjar01.png)    

在SeaTunnel执行`install-plugin.sh`时，它的日志跟Maven下载依赖如出一辙，可环境中确实没有安装过maven，这又是怎么一回事呢？        
![jarmanager03](http://img.xinzhuxiansheng.com/blogimgs/seatunnel/jarmanager03.png)  

### 了解mvnw    
在SeaTunnel的安装目录下有`mvnw`文件，我想用过Spring Boot的对这个文件应该很熟悉，在通过`start.spring.io`网站构建Spring Boot项目时会自动生成。    

`安装目录`  
![jarmanager01](http://img.xinzhuxiansheng.com/blogimgs/seatunnel/jarmanager01.png)   


`start.spring.io`   
![jarmanager02](http://img.xinzhuxiansheng.com/blogimgs/seatunnel/jarmanager02.png)        


>`mvnw`是一个在Spring Boot项目中经常见到的可执行Unix shell脚本，与此同时你也可能会看到`mvnw.cmd`，这是其在Windows系统中的对应脚本。这两个文件是Maven Wrapper（mvn wrapper）的一部分。   
Maven Wrapper是一个为了提供一种简单且方便的方式来运行Maven构建的工具。通过在你的项目中使用Maven Wrapper，你可以让其他开发者或者构建服务器在无需预先安装Maven或特定版本的Maven的情况下，直接运行项目中的Maven构建。  
当你运行`mvnw`或者`mvnw.cmd`命令时，这个脚本会检查是否已经下载并缓存了所需版本的Maven，如果没有，它会自动下载；然后它会用这个Maven版本来运行构建。      
比如，你可以用`./mvnw clean install`命令来启动一个Maven构建，就像你直接运行`mvn clean install`一样。不过前者会自动处理Maven的版本和安装。   
使用Maven Wrapper的好处在于，你可以确保所有开发者和构建系统都使用同一个Maven版本，从而避免因版本差异导致的构建问题。同时，新的开发者可以在不必关心如何安装和配置Maven的情况下，直接投入到项目开发中。   


### install-plugin.sh   
在`install-plugin.sh`脚本中，最核心是：     
```
${SEATUNNEL_HOME}/mvnw dependency:get -DgroupId=org.apache.seatunnel -DartifactId=${line} -Dversion=${version} -Ddest=${SEATUNNEL_HOME}/connectors/seatunnel
``` 
`mvnw`会检查是否已经下载并缓存了所需版本的Maven，如果没有，它会自动下载；然后它会用这个Maven版本来运行构建。

当执行完脚本后，你可以在`~/.m2/`看到仓库地址以及maven安装包。   
![jarmanager04](http://img.xinzhuxiansheng.com/blogimgs/seatunnel/jarmanager04.png)        

>需注意，脚本中使用 -Ddest参数 用来指定jar的下载路径，若不指定，则默认在`/root/.m2/repository`目录下。  


**后续在自己的项目中动态加载jar时候，对于jar的管理，又多了一些实战方法。**  


