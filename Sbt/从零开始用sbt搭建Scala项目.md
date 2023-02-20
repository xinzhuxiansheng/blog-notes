## 从零开始用sbt搭建Scala项目

### 安装sbt
refer https://www.scala-sbt.org/1.x/docs/Setup.html

### sbt简单案例
refer https://www.scala-sbt.org/1.x/docs/sbt-by-example.html

* 1.从最小化开始构建sbt项目
```shell
mkdir foo-build
cd foo-build
touch build.sbt
```

* 2.常见sbt shell命令
```shell
# 进入foo-build目录执行
# 启动sbt shell
>sbt

# 退出sbt shell
>exit

# 编译项目
>compile

# 自动编译修改后文件
>~compile

# 查看help信息
>help
```

* 3.编译scala code
在`foo-build` project项目下创建 src/main/scala/example 目录，在example目录下创建`Hello.scala`
```scala
package example

object Hello {
  def main(args: Array[String]): Unit = {
    println("Hello")
  }
}
```
使用`~compile`编译，最后以输入 ENter键退出

* 4.运行程序
```shell
# 进入sbt shell
>sbt

# 运行程序
>run
```

* 5.在sbt shell临时指定scala version
```shell
# 设置当前shell窗口 临时scala version
>set ThisBuild / scalaVersion := "2.13.8"

# 查看当前scala version
>scalaVersion
```

* 6.将sbt shell当前会话保存到项目`build.sbt`
```shell
# 在上一步 在shell设置了scalaVersion,可以将该配置永久写入build.sbt中
session save

# 此时查看 build.sbt会存在 ThisBuild / scalaVersion := "2.13.8"配置

```

* 7.设置项目名称
```shell
# 在build.sbt设置name

ThisBuild / scalaVersion := "2.13.6"
ThisBuild / organization := "com.example"

lazy val hello = (project in file("."))
  .settings(
    name := "Hello"
  )

```

* 8.重新加载配置文件构建
使用`reload`命令重新加载构建。该命令导致重新读取 build.sbt 文件，并应用其设置
```shell
>reload
```

* 9.添加ScalaTest类库
编辑 build.sbt,内容如下：
```shell
ThisBuild / scalaVersion := "2.13.6"
ThisBuild / organization := "com.example"

lazy val hello = (project in file("."))
  .settings(
    name := "Hello",
    libraryDependencies += "org.scalatest" %% "scalatest" % "3.2.7" % Test,
  )

# 若build.sbt改变后，记得执行 reload，进行重新加载并构建
reload
```



* 10.配置测试
```shell
# 执行全部test
>test

# 执行增量测试, 以便于提高测试效率
>~testQuick
```

* 11.编写测试用例
创建测试目录，`src/test/scala/HelloSpec.scala`
```scala
import org.scalatest.funsuite._

class HelloSpec extends AnyFunSuite {
  test("Hello should start with H") {
    assert("hello".startsWith("H"))
  }
}
```

>import导入scalatest包，所以在build.sbt需添加scalatest依赖

* 11.添加依赖
编辑 build.sbt
```shell
ThisBuild / scalaVersion := "2.13.6"
ThisBuild / organization := "com.example"

lazy val hello = (project in file("."))
  .settings(
    name := "Hello",
    libraryDependencies += "com.typesafe.play" %% "play-json" % "2.9.2",
    libraryDependencies += "com.eed3si9n" %% "gigahorse-okhttp" % "0.5.0",
    libraryDependencies += "org.scalatest" %% "scalatest" % "3.2.7" % Test,
  )

# 同样记得 reload重新加载 build.sbt再构建
```

* 12.使用Scala REPL
（待补充）

* 13.在sbt添加子项目
编辑 build.sbt
```shell
ThisBuild / scalaVersion := "2.13.6"
ThisBuild / organization := "com.example"

lazy val hello = (project in file("."))
  .settings(
    name := "Hello",
    libraryDependencies += "com.eed3si9n" %% "gigahorse-okhttp" % "0.5.0",
    libraryDependencies += "org.scalatest" %% "scalatest" % "3.2.7" % Test,
  )

lazy val helloCore = (project in file("core"))
  .settings(
    name := "Hello Core",
  )

# 同样记得 reload重新加载 build.sbt再构建
```

* 14.子项目相关
```shell
# 列出所有子项目
>projects

# 编译子项目
>helloCore/compile 
```

* 15.给子项目添加ScalaTest
编辑 build.sbt
```shell

ThisBuild / scalaVersion := "2.13.6"
ThisBuild / organization := "com.example"

val scalaTest = "org.scalatest" %% "scalatest" % "3.2.7"

lazy val hello = (project in file("."))
  .settings(
    name := "Hello",
    libraryDependencies += "com.eed3si9n" %% "gigahorse-okhttp" % "0.5.0",
    libraryDependencies += scalaTest % Test,
  )

lazy val helloCore = (project in file("core"))
  .settings(
    name := "Hello Core",
    libraryDependencies += scalaTest % Test,
  )

```

* 16.从子项目中提取公共依赖，减少重复定义
编辑 build.sbt, 定义 **scalaTest**变量，以导入变量方式给子项目添加类库
```shell
ThisBuild / scalaVersion := "2.13.6"
ThisBuild / organization := "com.example"

val scalaTest = "org.scalatest" %% "scalatest" % "3.2.7"

lazy val hello = (project in file("."))
  .settings(
    name := "Hello",
    libraryDependencies += "com.eed3si9n" %% "gigahorse-okhttp" % "0.5.0",
    libraryDependencies += scalaTest % Test,
  )

lazy val helloCore = (project in file("core"))
  .settings(
    name := "Hello Core",
    libraryDependencies += scalaTest % Test,
  )


# 先执行reload，再执行 ~testQuick
```

