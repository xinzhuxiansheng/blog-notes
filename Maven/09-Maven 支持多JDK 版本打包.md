# Maven - Maven 支持多 JDK 版本打包程序    

## 背景          
在学习和工作中开发的 Java 项目，它们集成了一些框架和类库相关使用。在使用的过程，我除了会去看它们的官方文档、google的一些资料，也会在自己学习的项目中来验证框架或者类库的API判断是否可以满足开发需求,我给每个框架或者类库都创建一个 `module`。这样可以让测试代码清晰、隔离 maven dependencies & JDK版本，例如我下面的两个测试项目。        
* `javamain-services`    
![usetoolchains01](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains01.jpg)     

>项目中 Module 个数比图片中更多。     

* `flink-tutorial`    
![usetoolchains02](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains02.jpg)  

在上述的情况下，可能会遇到 Maven 聚合工程依赖多个 JDK 版本，例如 Module A 依赖 JDK1.8，Module B 依赖 JDK11, `javamain-services`,`flink-tutorial`项目亦是如此。项目中的 JDK 配置如下图所示：             
![usetoolchains03](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains03.jpg)       

基于上面的介绍，在 Idea 使用 Maven 打包项目时，会遇到 `invalid target release: 11` 异常，下面是 Idea Console 打印的异常信息。        

```bash
[ERROR] Failed to execute goal org.apache.maven.plugins:maven-compiler-plugin:3.8.0:compile (default-compile) on project flink-cdc: Fatal error compiling: invalid target release: 11 -> [Help 1]
[ERROR] 
[ERROR] To see the full stack trace of the errors, re-run Maven with the -e switch.
[ERROR] Re-run Maven using the -X switch to enable full debug logging.
``` 

## 分析 invalid target release: 11 异常信息       
在`背景`章节中贴出了 Maven 聚合工程依赖多个 JDK 版本示意图，该项目中打包时会遇到 `invalid target release: 11`异常。这是因为 `Module B` 使用的 `maven-compiler-plugin`插件设置打包的JDK 环境是 11。     
```xml
<plugin>
    <artifactId>maven-compiler-plugin</artifactId>
    <version>3.1</version>
    <configuration>
        <source>${maven.compiler.source}</source>
        <target>${maven.compiler.source}</target>
    </configuration>
</plugin>
```

>注意点： 如果在 Idea 中使用 Maven 打包，那么 JDK 环境依赖的是 `maven runner`的 JDK 环境，如果是在开发笔记本的 Maven 命令打包，则依赖的是笔记本配置的JDK 环境。   

![usetoolchains04](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains04.jpg)    

解决 `invalid target release: 11`异常，可以通过以下方式解决。       
1. 升级 Idea Maven Runner JDK Version，从 1.8 JDK 配置为 11 JDK。再重新使用 Idea Maven 打包。        
![usetoolchains05](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains05.jpg)       

达到的效果如下图所示：   
![usetoolchains06](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains06.jpg)        

2. 使用 `maven-compiler-plugin` maven 插件时，明确配置 `javac` 路径，如下所示：     
```bash
<plugin>
    <artifactId>maven-compiler-plugin</artifactId>
    <version>3.1</version>
    <configuration>
        <source>${maven.compiler.source}</source>
        <target>${maven.compiler.source}</target>
        <fork>true</fork> <!-- 必须启用fork -->
        <executable>D:\Software\Dev\JDK\jdk-11.0.23\bin\javac</executable>   
    </configuration>
</plugin>
```

3. 使用 `maven-toolchains-plugin` maven 插件，明确配置 `ModuleB` 模块打包时使用的 JDK 路径。  

>接下来，介绍 `maven-toolchains-plugin` maven 插件的使用。   

## 使用 `maven-toolchains-plugin` maven 插件     
配置后的整体结构如下图所示：    
![usetoolchains07](http://img.xinzhuxiansheng.com/blogimgs/maven/usetoolchains07.jpg)  

### 配置 Maven conf/toolchains.xml   
打开本地 Maven 安装目录下的 `conf/toolchains.xml`文件，添加 JDK 1.8，JDK 11 的安装目录，添加内容如下所示：         
```bash
<toolchain>
    <type>jdk</type>
    <provides>
        <version>1.8</version>
        <vendor>oracle</vendor>
    </provides>
    <configuration>
        <jdkHome>D:\Software\Dev\JDK\jdk-1.8</jdkHome>
    </configuration>
</toolchain>
<toolchain>
    <type>jdk</type>
    <provides>
        <version>11</version>
        <vendor>oracle</vendor>
    </provides>
    <configuration>
        <jdkHome>D:\Software\Dev\JDK\jdk-11.0.23</jdkHome>
    </configuration>
</toolchain>
```

>注意，`conf/toolchains.xml` 文件也复制一份到 `~/.m2/`目录下。   

### 配置 maven-toolchains-plugin    
在 `Module B`模块的 `pom.xml` 文件中添加 `maven-toolchains-plugin` 插件, 此时只需明确定义 toolschains 使用的是 JDK 11 即可。       
```bash
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-toolchains-plugin</artifactId>
    <version>3.2.0</version>
    <executions>
        <execution>
            <goals>
                <goal>toolchain</goal>
            </goals>
        </execution>
    </executions>
    <configuration>
        <toolchains>
            <jdk>
                <version>11</version> # 明确指定 JDK 版本
            </jdk>
        </toolchains>
    </configuration>
</plugin>
```  

到这里，`maven-toolchains-plugin` 插件使用已经介绍完成了。    

我再也不用担心不同 JDK 版本带来的打包烦恼了，我的测试代码放在同一个 Idea Project中的不同 Module，并且放在 github 便于跟踪 history， `彻底告别脏乱差`。      
