## Idea Debug环境搭建.md

dev分支

### Run/Debug Configurations
创建 “Dlink” Spring Boot

```java
// VM options
-Dloader.path=/Users/yiche/Code/JAVA/yzhou/dlink/build/dlink-release-0.6.1/lib
-Ddruid.mysql.usePingMethod=false
```

### 开发者指南
参考wiki： http://www.dlink.top/docs/next/developer_guide/local_debug


>注意： 在dinky 0.6版本后，不需要额外启动前端，启动后端后便可访问 127.0.0.1:8888
