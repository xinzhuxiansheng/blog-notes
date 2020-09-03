场景：

现在很多公司，都有 maven 的私服 ，在maven项目中，基本上有两个仓库 ，一个是maven的公共仓库，一个是私服仓库；
有的时候，我们download 别人的代码的时候，pom文件中报错，往往是jar 找不到。你要是知道别人的私服地址，那直接在setting配置文件设置即可 ，要是不知道，怎么办？
那就只能 先 下载好jar 然后 通过 maven install 安装到本地仓库（操作前，请配置好maven 的环境变量）
解决：

加载jar 包到本地仓库（以加载jave-1.0.2.jar 为例）
首先在pom.xml文件中确保已经添加了 jave-1.0.2.jar jar的dependency
```xml
<dependency>
    <groupId>com.geshizhuanma</groupId>
    <artifactId>jave</artifactId>
    <version>1.0.2</version>
</dependency>

```
配置好，再执行以下命令：

```shell
mvn install:install-file -DgroupId=com.geshizhuanma -DartifactId=jave -Dversion=1.0.2 -Dpackaging=jar -Dfile=H:\jave-1.0.2.jar
```

说明：-Dfile指第三方jar的路径，其它的注意要确保maven命令中groupId、artifactId、version与pom.xml中的配置相同，-Dpackaging表示加载的文件类型