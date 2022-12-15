
## How to Make a ZIO Application

>refer: https://zio.dev/guides/tutorials/configurable-zio-application

### Introduction（介绍）
在应用程序开发中可配置参数是最常见的要求之一，在本文中会介绍如何使用ZIO-Config library进行参数配置。


### Prerequisites（准备工作）
将使用”ZIO Quickstart: Restful Web Service“(https://zio.dev/guides/quickstarts/restful-webservice/) 作为项目模板，你需要提前下载好它。

### Problem（问题）
当前的web service项目中不允许我们配置host和端口，`我们希望能够配置服务的主机和端口，以便在运行应用程序之前，我们指定服务的主机和端口`

```scala
git clone git@github.com:khajavi/zio-quickstart-restful-webservice.git
cd zio-quickstart-restful-webservice
sbt run
```

### Solution (解决方案)
在开发ZIO应用程序时，以下介绍在ZIO环境中访问上下文信息两种方式：    
* 访问服务：我们可以从环境中访问服务接口，它们应该被实现并提供给世界尽头的整个应用程序（服务模式）。    

* 访问配置：我们可以访问作为应用程序一部分的配置。  

在本文中，我们将重点介绍第二种情况，即配置服务的主机和端口。`让我们看看我们需要采取哪些步骤来实现这一目标。 `

#### Step1: 定义配置数据类型
通过定义`HttpServerConfig`样例类来构建host，port参数    
```scala
case class HttpServerConfig(host:String, port:Int)
```


#### Step2: 从环境中访问配置
已经定义了配置数据类型，并且在`ZIO.service[HttpServerConfig]`方法中使用配置数据。

```scala

val workflow: ZIO[HttpServerConfig, IOException, Unit] =
  ZIO.service[HttpServerConfig].flatMap { config =>
    Console.printLine(
      "Application started with following configuration:\n" +
        s"\thost: ${config.host}\n" +
        s"\tport: ${config.port}"
    )
  }

```

**以下完整示例**
```scala
import zio._
import java.io.IOException

object Configuration extends ZIOAppDefault {

  val workflow: ZIO[HttpServerConfig, IOException, Unit] =
    ZIO.service[HttpServerConfig].flatMap { config =>
      Console.printLine(
        "Application started with following configuration:\n" +
          s"\thost: ${config.host}\n" +
          s"\tport: ${config.port}"
      )
    }

  def run = workflow
}
```
此时编译报错，

上面的错误是因为我们试图从环境中访问 HttpServerConfig 配置，但是我们没有为它提供一个层。    

为了实现这一点，我们需要采取两个步骤。  
* 为 HttpServerConfig 配置数据类型定义一个层。  
* 为我们的 ZIO 工作流程提供层。     
为了提供配置层，我们需要定义一个HttpServerConfig类型的ZLayer并使用 ZIO#provide 方法。
```
──── ZIO APP ERROR ───────────────────────────────────────────────────  Your effect requires a service that is not in the environment. Please provide a layer for the following type: 
 
 1. dev.zio.quickstart.config.HttpServerConfig  
 
 Call your effect's provide method with the layers you need. You can read more about layers and providing services here:  https://zio.dev/reference/contextual/  ────────────────────────────────────────────────────────────────────── 
```