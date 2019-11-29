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


mvn clean  install package -Papache-release  -DskipTests  -Dgpg.skip  -Dmaven.javadoc.skip=true -Dcheckstyle.skip