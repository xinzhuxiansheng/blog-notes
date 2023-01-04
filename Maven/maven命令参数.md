**`正文`**
[TOC]

## 跳过 gpg
```shell
-Dgpg.skip
```

## 跳过单元测试
```shell
-Dmaven.test.skip=true
-DskipTests=true
```


## 将本地jar 安装到本地仓库
```shell
mvn install:install-file -DgroupId=com.xinzhuxiansheg -DartifactId=demo -Dversion=1.0.2 -Dpackaging=jar -Dfile=H:\demo-1.0.2.jar
```
说明: -Dfile值第三方jar的路径，其它的注意要确保maven命令中groupId，artifactId，version与pom.xml中的配置相同，-Dpackaging表示加载的文件类型



mvn clean  install package -Papache-release  -DskipTests  -Dgpg.skip  -Dmaven.javadoc.skip=true -Dcheckstyle.skip



## 利用maven执行 java程序
```shell
mvn exec:java -Dexec.mainClass="top.guoziyang.mydb.backend.Launcher" -Dexec.args="-create /tmp/mydb"
```

refer:
https://blog.csdn.net/sayyy/article/details/102938703
https://github.com/CN-GuoZiyang/MYDB

