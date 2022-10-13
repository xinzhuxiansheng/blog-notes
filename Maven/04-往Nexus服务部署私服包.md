**`正文`**
[TOC]

## 配置setting.xml
```xml
#配置nexus免密登录
<server>
    <id>releases</id>
    <username>xxx</username>
    <password>xxxxx</password>
</server>

<server>
    <id>snapshots</id>
    <username>xxx</username>
    <password>xxxxx</password>
</server>
```

## 配置pom.xml
```xml
<distributionManagement>
    <repository>
        <id>releases</id>
        <name>xxxx releases</name>
        <url>http://xxxxxx.com/nexus/content/repositories/releases</url>
    </repository>
    <snapshotRepository>
        <id>snapshots</id>
        <name>xxxx snapshots</name>
        <url>http://xxxxxx.com/nexus/content/repositories/snapshots</url>
    </snapshotRepository>
</distributionManagement>
```

## 配置 maven-source-plugin插件
```xml
<!--将源码也部署到私服-->
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-source-plugin</artifactId>
    <version>2.2.1</version>
    <configuration>
        <attach>true</attach>
    </configuration>
    <executions>
        <execution>
            <phase>compile</phase>
            <goals>
                <goal>jar</goal>
            </goals>
        </execution>
    </executions>
</plugin>
```

##执行 clean deploy命令
