# Java - 给 Spring Boot 服务配置通用脚本     

## 背景   
在学习和工作中接触了一些开源的 Java 项目，它们绝大多数在部署服务的根目录下存在 `bin` 子目录用来存放一些`sh`脚本对服务进行`启停操作`，存在 `conf` 子目录用来存放一些配置文件对服务进行配置参数管理, 例如 kafka,flink,rocketmq,streampark 等项目， 用起来确实很方便，下面列举一些示例：             

下面是 `RocketMQ` 的部署目录截图:         
![createsh01](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh01.jpg)      

下面是 `Kafka` 的部署目录截图    
![createsh02](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh02.jpg)      

该篇 Blog 主要介绍的内容是基于 `Spring Boot` 框架开发的服务如何也像上面优秀的开源项目一样，有 `bin` 目录提供脚本进行服务启停，有 `conf` 目录提供配置参数管理。   

## 有 与 没有 之间   
从 `https://start.spring.io/` 网站构建 Spring Boot `jar` 项目时，并不存在 `bin`,`conf`目录，下面是初始化项目的目录结构：      
![createsh03](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh03.jpg)  

这部分与 `Playframework for Scala` 框架非常不同，博主的 Blog (http://xinzhuxiansheng.com/) 是使用 Play for Scala 开发的，下面是它的目录结构：           

![createsh04](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh04.jpg)   

这部分 `Play for Scala` 做的比 Spring Boot 好太多，`好的习惯是成功的一部分`。   

在 Spring Boot `jar` 项目中，若不做特殊处理，配置文件会在打包项目时集成到 `jar` 文件内部， 这不利于我们去手动调整配置参数，可能我们会重新打包服务再部署，或者你将服务的配置参数集成到`配置中心`这样的服务中，或者集成到 MySQL，这些也能简化我们调整参数的成本，但对于一些业务量，暂不需要双活等情况下，这会增加一些开发成本。但我们还是可以通过一些其他手段来解决它的，下面会介绍。     

>当你通过某些手段提高一些事情的效率时，你可以比别人有更多的时间做其他事。       

## Spring Boot 集成 bin, conf     
我们先看下完整示例模板的结构，(这个项目模板是我写微信小程序的后台服务。)            

![createsh05](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh05.jpg)  

### 使用 maven-assembly-plugin 插件进行打包   
在项目根目录下创建名为 `distribution` 子模块， 在 pom.xml 添加服务入口 jar 依赖 和 `maven-assembly-plugin` 插件。 示例内容如下：                       

**pom.xml**    
```xml
<?xml version="1.0" encoding="UTF-8"?>
<project xmlns="http://maven.apache.org/POM/4.0.0"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
    <parent>
        <groupId>com.yzhou.wx</groupId>
        <artifactId>turntable-springboot</artifactId>
        <version>1.0-SNAPSHOT</version>
    </parent>
    <modelVersion>4.0.0</modelVersion>

    <artifactId>turntable-distribution</artifactId>

    <properties>
        <maven.compiler.source>8</maven.compiler.source>
        <maven.compiler.target>8</maven.compiler.target>
        <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding>
    </properties>

    <dependencies>
        <dependency>
            <groupId>com.yzhou.wx</groupId>
            <artifactId>turntable-server</artifactId>
            <version>1.0-SNAPSHOT</version>
        </dependency>
    </dependencies>

    <build>
        <finalName>turntable-server</finalName>
        <plugins>
            <plugin>
                <groupId>org.apache.maven.plugins</groupId>
                <artifactId>maven-assembly-plugin</artifactId>
                <configuration>
                    <descriptors>
                        <descriptor>assembly.xml</descriptor>
                    </descriptors>
                    <tarLongFileMode>posix</tarLongFileMode>
                </configuration>
                <executions>
                    <execution>
                        <id>make-assembly</id>
                        <phase>install</phase>
                        <goals>
                            <goal>single</goal>
                        </goals>
                    </execution>
                </executions>
            </plugin>
        </plugins>
    </build>

</project>
```

在 pom.xml 同级目录下 创建 `assembly.xml`，定义打包内容，示例内容如下：     

**assembly.xml**
```xml
<assembly>
    <id>${project.version}</id>
    <includeBaseDirectory>true</includeBaseDirectory>
    <formats>
        <format>dir</format>
        <format>tar.gz</format>
        <format>zip</format>
    </formats>
    <fileSets>
        <fileSet>
            <includes>
                <include>conf/**</include>
            </includes>
        </fileSet>
        <fileSet>
            <includes>
                <include>bin/*</include>
            </includes>
            <fileMode>0755</fileMode>
        </fileSet>
    </fileSets>
    <files>
        <file>
            <!--打好的jar包名称和放置目录-->
            <source>../server/target/turntable-server.jar</source>
            <outputDirectory>target/</outputDirectory>
        </file>
    </files>
    <moduleSets>
        <moduleSet>
            <useAllReactorProjects>true</useAllReactorProjects>
            <includes>
                <include>com.yzhou.wx:turntable-server</include>
            </includes>
        </moduleSet>
    </moduleSets>
</assembly>
```

### 创建 bin 目录 & 添加脚本   
在 `distribution` 目录下创建 `bin` 目录，分别创建以下 4个 file   
```bash
shutdown.cmd  
shutdown.sh   
startup.cmd  
startup.sh 
```

>脚本中的一些文件名，变量名定义需要根据不同的服务名称做一些调整。这点非常重要 !!!

**shutdown.cmd**    
```bash
@echo off

if not exist "%JAVA_HOME%\bin\jps.exe" echo Please set the JAVA_HOME variable in your environment, We need java(x64)! jdk8 or later is better! & EXIT /B 1

setlocal

set "PATH=%JAVA_HOME%\bin;%PATH%"

echo killing turntable server

for /f "tokens=1" %%i in ('jps -m ^| find "turntable.server"') do ( taskkill /F /PID %%i )

echo Done!
```

**shutdown.sh**
shutdown 根据 `进程描述中的特殊符号`来进行进程的筛选，再停止作业。我们必须保证它的`唯一性`。     

![createsh07](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh07.jpg)    

```bash
#!/bin/bash

cd `dirname $0`/../target
target_dir=`pwd`

pid=`ps ax | grep -i 'turntable.server' | grep ${target_dir} | grep java | grep -v grep | awk '{print $1}'`
if [ -z "$pid" ] ; then
        echo "no turntable server running."
        exit -1;
fi

echo "the turntable server(${pid}) is running..."

kill ${pid}

echo "send shutdown request to turntable server(${pid}) OK"
```

**startup.cmd**  
```bash
@echo off

if not exist "%JAVA_HOME%\bin\java.exe" echo Please set the JAVA_HOME variable in your environment, We need java(x64)! jdk8 or later is better! & EXIT /B 1
set "JAVA=%JAVA_HOME%\bin\java.exe"

setlocal enabledelayedexpansion

set BASE_DIR=%~dp0
rem added double quotation marks to avoid the issue caused by the folder names containing spaces.
rem removed the last 5 chars(which means \bin\) to get the base DIR.
set BASE_DIR="%BASE_DIR:~0,-5%"

set CUSTOM_SEARCH_LOCATIONS=file:%BASE_DIR%/conf/

set SERVER=turntable-server

set "APP_JVM_OPTS=-Xms512m -Xmx512m -Xmn256m"

rem set APP server options
set "APP_OPTS=%APP_OPTS% -jar %BASE_DIR%\target\%SERVER%.jar"

rem set APP server spring config location
set "APP_CONFIG_OPTS=--spring.config.additional-location=%CUSTOM_SEARCH_LOCATIONS%"

rem set APP server log4j file location
set "APP_LOG4J_OPTS=--logging.config=%BASE_DIR%/conf/turntable-server-logback.xml"


set COMMAND="%JAVA%" %APP_JVM_OPTS% %APP_OPTS% %APP_CONFIG_OPTS% %APP_LOG4J_OPTS% turntable.server %*

echo "turntable server is starting..."
rem start turntable server command
%COMMAND%
echo "turntable server is started!"
```

**startup.sh**      
其脚本核心是通过 `--spring.config.additional-location`,`--logging.config` 设置启动参数。     
![createsh06](http://img.xinzhuxiansheng.com/blogimgs/springboot/createsh06.jpg)   

```bash
#!/bin/bash

cygwin=false
darwin=false
os400=false
case "`uname`" in
CYGWIN*) cygwin=true;;
Darwin*) darwin=true;;
OS400*) os400=true;;
esac
error_exit ()
{
    echo "ERROR: $1 !!"
    exit 1
}
[ ! -e "$JAVA_HOME/bin/java" ] && JAVA_HOME=$HOME/jdk/java
[ ! -e "$JAVA_HOME/bin/java" ] && JAVA_HOME=/usr/java
[ ! -e "$JAVA_HOME/bin/java" ] && unset JAVA_HOME

if [ -z "$JAVA_HOME" ]; then
  if $darwin; then

    if [ -x '/usr/libexec/java_home' ] ; then
      export JAVA_HOME=`/usr/libexec/java_home`

    elif [ -d "/System/Library/Frameworks/JavaVM.framework/Versions/CurrentJDK/Home" ]; then
      export JAVA_HOME="/System/Library/Frameworks/JavaVM.framework/Versions/CurrentJDK/Home"
    fi
  else
    JAVA_PATH=`dirname $(readlink -f $(which javac))`
    if [ "x$JAVA_PATH" != "x" ]; then
      export JAVA_HOME=`dirname $JAVA_PATH 2>/dev/null`
    fi
  fi
  if [ -z "$JAVA_HOME" ]; then
        error_exit "Please set the JAVA_HOME variable in your environment, We need java(x64)! jdk8 or later is better!"
  fi
fi

export SERVER="turntable-server"

export JAVA_HOME
export JAVA="$JAVA_HOME/bin/java"
export BASE_DIR=`cd $(dirname $0)/..; pwd`
export CUSTOM_SEARCH_LOCATIONS=file:${BASE_DIR}/conf/

#===========================================================================================
# JVM Configuration
#===========================================================================================
JAVA_OPT="${JAVA_OPT} -Xms512m -Xmx512m -Xmn256m"

JAVA_MAJOR_VERSION=$($JAVA -version 2>&1 | sed -E -n 's/.* version "([0-9]*).*$/\1/p')
if [[ "$JAVA_MAJOR_VERSION" -ge "9" ]] ; then
  JAVA_OPT="${JAVA_OPT} -Xlog:gc*:file=${BASE_DIR}/logs/turntable_gc.log:time,tags:filecount=10,filesize=102400"
else
  JAVA_OPT_EXT_FIX="-Djava.ext.dirs=${JAVA_HOME}/jre/lib/ext:${JAVA_HOME}/lib/ext"
  JAVA_OPT="${JAVA_OPT} -Xloggc:${BASE_DIR}/logs/turntable_gc.log -verbose:gc -XX:+PrintGCDetails -XX:+PrintGCDateStamps -XX:+PrintGCTimeStamps -XX:+UseGCLogFileRotation -XX:NumberOfGCLogFiles=10 -XX:GCLogFileSize=100M"
fi

JAVA_OPT="${JAVA_OPT} -jar ${BASE_DIR}/target/${SERVER}.jar"
JAVA_OPT="${JAVA_OPT} ${JAVA_OPT_EXT}"
JAVA_OPT="${JAVA_OPT} --spring.config.additional-location=${CUSTOM_SEARCH_LOCATIONS}"
JAVA_OPT="${JAVA_OPT} --logging.config=${BASE_DIR}/conf/turntable-server-logback.xml"
JAVA_OPT="${JAVA_OPT} --server.max-http-header-size=524288"

if [ ! -d "${BASE_DIR}/logs" ]; then
  mkdir ${BASE_DIR}/logs
fi

echo "$JAVA $JAVA_OPT_EXT_FIX ${JAVA_OPT}"

echo "turntable server is starting..."

# start
echo "$JAVA $JAVA_OPT_EXT_FIX ${JAVA_OPT}" > ${BASE_DIR}/logs/start.out 2>&1 &
nohup "$JAVA" "$JAVA_OPT_EXT_FIX" ${JAVA_OPT} turntable.server >> /dev/null 2>&1 &
echo "turntable server is started!"
```

### 创建 conf 目录 & 添加配置文件    
在 `distribution` 目录下创建 `conf` 目录，分别创建以下 2个 file  
```bash
application.yaml 
turntable-server-logback.xml   
```  

**turntable-server-logback.xml**        
```bash
<configuration>

    <!-- 日志文件路径 -->
    <springProperty scope="context" name="logFile" source="logging.file"/>
    <property name="LOG_FILE" value="${logFile}"></property>

    <!-- 日志打印级别 -->
    <springProperty scope="context" name="logLevel" source="logging.level.root" defaultValue="info"/>
    <property name="LOG_LEVEL" value="${logLevel}"></property>

    <!-- 打印日志格式 -->
    <property name="FILE_LOG_PATTERN"
              value="${FILE_LOG_PATTERN:-%d{yyyy-MM-dd HH:mm:ss.SSS} ${LOG_LEVEL_PATTERN:-%5p} ${PID:- } --- [%t] %-40.40logger{39} : %m%n}"/>

    <!-- %m输出的信息,%p日志级别,%t线程名,%d日期,%c类的全名,%i索引【从数字0开始递增】,,, -->
    <!-- appender是configuration的子节点，是负责写日志的组件。 -->
    <!-- ConsoleAppender：把日志输出到控制台 -->
    <appender name="STDOUT" class="ch.qos.logback.core.ConsoleAppender">
        <encoder>
            <pattern>${FILE_LOG_PATTERN}</pattern>
            <!-- 控制台也要使用UTF-8，不要使用GBK，否则会中文乱码 -->
            <charset>UTF-8</charset>
        </encoder>
        <!-- 彩色日志 -->
        <layout class="ch.qos.logback.classic.PatternLayout">
            <pattern>
                %d{yyyy-MM-dd HH:mm:ss} [%thread] %magenta(%-5level) %green([%-50.50class]) : %cyan(%msg) %n
            </pattern>
        </layout>
    </appender>
    <!-- RollingFileAppender：滚动记录文件，先将日志记录到指定文件，当符合某个条件时，将日志记录到其他文件 -->
    <!-- 以下的大概意思是：1.先按日期存日志，日期变了，将前一天的日志文件名重命名为XXX%日期%索引，新的日志仍然是sys.log -->
    <!-- 2.如果日期没有发生变化，但是当前日志的文件大小超过1KB时，对当前日志进行分割 重命名-->
    <appender name="SYSLOG" class="ch.qos.logback.core.rolling.RollingFileAppender">
        <File>${LOG_FILE}</File>
        <!-- rollingPolicy:当发生滚动时，决定 RollingFileAppender 的行为，涉及文件移动和重命名。 -->
        <!-- TimeBasedRollingPolicy： 最常用的滚动策略，它根据时间来制定滚动策略，既负责滚动也负责触发滚动 -->
        <rollingPolicy class="ch.qos.logback.core.rolling.TimeBasedRollingPolicy">
            <!-- 活动文件的名字会根据fileNamePattern的值，每隔一段时间改变一次 -->
            <!-- 文件名：log/sys.2018-09-13.0.log -->
            <fileNamePattern>${LOG_FILE}.%d.%i.log</fileNamePattern>
            <!-- 每产生一个日志文件，该日志文件的保存期限为30天 -->
            <maxHistory>14</maxHistory>
            <timeBasedFileNamingAndTriggeringPolicy class="ch.qos.logback.core.rolling.SizeAndTimeBasedFNATP">
                <!-- maxFileSize:这是活动文件的大小，默认值是10MB -->
                <maxFileSize>64MB</maxFileSize>
            </timeBasedFileNamingAndTriggeringPolicy>
        </rollingPolicy>
        <encoder>
            <!-- pattern节点，用来设置日志的输入格式 -->
            <pattern>${FILE_LOG_PATTERN}</pattern> <!-- 记录日志的编码 -->
            <charset>UTF-8</charset> <!-- 此处设置字符集 -->
        </encoder>
    </appender>
    <!-- 只用保存输出error级别的日志 -->
    <appender name="ERRLOG" class="ch.qos.logback.core.rolling.RollingFileAppender">
        <File>${LOG_FILE}_err.log</File>
        <rollingPolicy class="ch.qos.logback.core.rolling.TimeBasedRollingPolicy">
            <FileNamePattern>${LOG_FILE}_err.%d.%i.log</FileNamePattern>
            <maxHistory>14</maxHistory>
            <timeBasedFileNamingAndTriggeringPolicy class="ch.qos.logback.core.rolling.SizeAndTimeBasedFNATP">
                <!-- maxFileSize:这是活动文件的大小，默认值是10MB -->
                <maxFileSize>64MB</maxFileSize>
            </timeBasedFileNamingAndTriggeringPolicy>
        </rollingPolicy>
        <encoder>
            <!-- pattern节点，用来设置日志的输入格式 -->
            <pattern>${FILE_LOG_PATTERN}</pattern> <!-- 记录日志的编码 -->
            <charset>UTF-8</charset> <!-- 此处设置字符集 -->
        </encoder>
        <!-- 下面为配置只输出error级别的日志 -->
        <filter class="ch.qos.logback.classic.filter.LevelFilter">
            <level>ERROR</level>
            <onMatch>ACCEPT</onMatch>
            <onMismatch>DENY</onMismatch>
        </filter>
    </appender>

    <!-- 控制台输出日志级别 -->
    <root level="${LOG_LEVEL}">
        <appender-ref ref="STDOUT"/>
        <appender-ref ref="SYSLOG"/>
        <appender-ref ref="ERRLOG"/>
    </root>

</configuration>
```  

**application.yaml**  
`application.yaml` 是 spring boot 的配置文件，这里直接从 resources 目录下拷贝到 `conf` 目录即可。   

这里需要注意， 在 turntable-server-logback.xml 中定义了一些变量，例如 `logging.file`,`logging.level.root` 参数；   

```xml
    <springProperty scope="context" name="logFile" source="logging.file"/>
    <property name="LOG_FILE" value="${logFile}"></property>

    <!-- 日志打印级别 -->
    <springProperty scope="context" name="logLevel" source="logging.level.root" defaultValue="info"/>
    <property name="LOG_LEVEL" value="${logLevel}"></property>
```

那么在 `application.yaml` 中添加以下配置：      
```bash
logging:
    file: ${user.home}/logs/${spring.application.name}/${spring.application.name}.log
    level:
        root: INFO
```  

到这里，我们需要做的配置。  

## 总结    
执行 `mvn clean install` 可对项目进行打包，在后面的服务启停，参数修改 效率都大大的提升。   