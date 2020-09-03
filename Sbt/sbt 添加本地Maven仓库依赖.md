打开你的 .sbt 目录，如 C:\Users\menci\.sbt，

添加 repositories 文件，内容如下： 
```xml
[repositories]
    local
    maven-local: file:////C:/Users/menci/.m2/repository/
    isuwang-public: http://nexus.oa.isuwang.com/repository/maven-public/
    aliyun: http://maven.aliyun.com/nexus/content/groups/public
```

注意： 
- maven-local 为你的本地 maven 仓库 
- isuwang-public 为你的私服地址