## Spring Boot 采集服务监控 

Spring Boot 可以很方便地通过 Actuator 和 Micrometer 来采集 JVM（Java Virtual Machine）相关的 Metrics。Actuator 是 Spring Boot 提供的一种生产级的特性，用于监控和管理应用程序。Micrometer 是一个应用程序指标（Metrics）的库，提供了各种监控系统的实现，包括 Prometheus、InfluxDB、Graphite、New Relic 等。

以下是步骤：

1. 首先，在你的 `pom.xml` 文件中添加 Actuator 和 Micrometer 的依赖：

```xml
<dependencies>
    <!-- Spring Boot Actuator -->
    <dependency>
        <groupId>org.springframework.boot</groupId>
        <artifactId>spring-boot-starter-actuator</artifactId>
    </dependency>

    <!-- Micrometer Prometheus registry -->
    <dependency>
        <groupId>io.micrometer</groupId>
        <artifactId>micrometer-registry-prometheus</artifactId>
    </dependency>
</dependencies>
```

2. 在 `application.properties` 或 `application.yml` 中开启和配置 Actuator endpoints，使其暴露 JVM Metrics：

```properties
management.endpoints.web.exposure.include=health,info,prometheus
management.endpoint.health.show-details=always
management.endpoint.metrics.enabled=true
management.metrics.export.prometheus.enabled=true
management.endpoint.prometheus.enabled=true
```

3. 现在你的应用程序已经开始收集 JVM Metrics，并通过 Actuator 的 `/actuator/prometheus` endpoint 暴露出来。你可以通过访问 `http://localhost:8080/actuator/prometheus`（假设你的应用运行在本地的8080端口）来查看这些 Metrics。

这些 Metrics 包括 JVM 的内存使用情况、线程状态、垃圾回收等信息。具体的 Metrics 取决于 JVM、Micrometer、以及你选择的监控系统。

如果你想要进一步定制你的 Metrics，比如添加自定义的 Metrics，你可以在你的代码中使用 Micrometer 的 API。例如，你可以创建一个 `MeterRegistry` bean，并使用它来创建和更新你的 Metrics。

