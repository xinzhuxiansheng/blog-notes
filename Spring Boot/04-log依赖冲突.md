
## log依赖冲突

>以下列举多种依赖冲突情况

### log4j-slf4j-impl cannot be present with log4j-to-slf4j

Spring boot服务无法启动，提示`log4j-slf4j-impl cannot be present with log4j-to-slf4j` 异常描述。 仅需要将Spring Boot自带的Log包排除掉即可。

博主习惯使用log4j2+slf4j作为 Spring Boot的日志框架,所以也同时排除的logback

```

<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter</artifactId>
    <exclusions>
        <exclusion>
            <artifactId>logback-classic</artifactId>
            <groupId>ch.qos.logback</groupId>
        </exclusion>
        <exclusion>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-logging</artifactId>
        </exclusion>
    </exclusions>
</dependency>

```

