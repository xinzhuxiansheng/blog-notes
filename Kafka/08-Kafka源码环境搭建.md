--In Blog
--Tags: Kafka,Kafka 2.2.1

# Kafka 源码环境搭建

>下面说的Kafka,没有做特殊说明，那就是指`2.2.1`版本

## 源码下载
Github: https://github.com/apache/kafka  Tags: 2.2.1

## Debug Environment Require
>一定要仔细阅读 `README.md`(https://github.com/apache/kafka/blob/2.2.1/README.md)

* Kafka requires Gradle 5.0 or higher.(Gradle版本 >= 5.0)
* Java 8 should be used for building in order to support both Java 8 and Java 11 at runtime.(JDK8)
* Scala 2.12 is used by default, see below for how to use a different Scala version or all of the supported Scala versions(默认Scala version=2.12，或根据不同版本编译)

`Gradle安装及环境配置，Scala安装 请自行安装`

**Debug环境**
* Gradle 5.5.1
* Scala 2.12.8
* Java 1.8.0_271

## Idea 导入项目

**1. Gradle配置**

![Gradle配置](http://118.126.116.71/blogimgs/kafka/GradleSetting.png)

.......... 慢慢等Gradle 下载依赖包

**2. Zookeeper环境搭建**

zookeeper version: 3.4.13
单机启动即可

**3. clients模块`common.message`编译**

`clients`模块 ,resources资源文件夹 `common.message`，需要编译，产出物在src的generated目录下

本地是Windows10 OS,先进入Kafka目录，执行编译
```shell
gradle clean releaseTarGz -x test
```
![编译common_message包](http://118.126.116.71/blogimgs/kafka/编译common_message包.png)

**4. 配置Application main方法启动**

Idea配置`Run/Debug Configurations`，创建`Application`启动
* Main class: kafka.Kafka
* VM options: 
```shell
-Dcom.sun.management.jmxremote.port=9999
-Dcom.sun.management.jmxremote.ssl=false
-Dcom.sun.management.jmxremote.authenticate=false
-Dfile.encoding=UTF-8
```
* Program arguments: config/server.properties
* Use classpath of module: kafka.core.main

>请注意: 修改server.properties 配置项
* advertised.listeners
* log.dirs
* zookeeper.connect

**5. Project Structure 配置log4j相关jar依赖**

若不配置log4j，`提示SLF4J异常日志并且看不到启动日志`，所以对我们来说无法通过日志分析
异常提示：
```java
SLF4J: Failed to load class "org.slf4j.impl.StaticLoggerBinder".
SLF4J: Defaulting to no-operation (NOP) logger implementation
SLF4J: See http://www.slf4j.org/codes.html#StaticLoggerBinder for further details.
```

>Open Project Structure  --> core模块 main的 Dependencies
* 调整log4j:1.2.17 执行周期由 Scope: Provided -> Compile
* 添加Library slf4j-log4j12:1.7.25 Scope: Compile

![log4jdebug环境配置](http://118.126.116.71/blogimgs/kafka/log4jdebug环境配置.png)


**6. Core模块配置 log4j.properties**

若不配置log4j.properties，`提示log4j异常日志`
```java
log4j:WARN No appenders could be found for logger (kafka.utils.Log4jControllerRegistration$).
log4j:WARN Please initialize the log4j system properly.
log4j:WARN See http://logging.apache.org/log4j/1.2/faq.html#noconfig for more info.
```

>在 `core`模块main目录下创建`resources文件夹`,并且创建`log4j.properties`, 具体如何配置请自行配置
仅提供参考：
```shell
log4j.rootLogger=DEBUG, stdout

log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.stdout.layout=org.apache.log4j.PatternLayout
log4j.appender.stdout.layout.ConversionPattern=%-d{yyyy-MM-dd HH:mm:ss}  [ %C:%L ] - [ %p ]  %m%n
```



#### 开始Debug吧 !!!