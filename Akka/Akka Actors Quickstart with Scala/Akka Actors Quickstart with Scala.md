

## Akka Actors Quickstart with Scala

Akka是一个工具包和运行时，用于在JVM上构建高度并发、分布式和容错的事件驱动应用程序。Akka可以与Java和Scala一起使用。本指南通过描述Hello World示例的Scala版本来介绍Akka Actor。如果你更喜欢Akka与Java一起使用，请切换到“Akka Quickstart with Java guide”。

Actor是Akka中的执行单元。Actor模型是一种抽象，可以更轻松地编写正确的并发、并行和分布式系统。Hello World示例说明了Akka的基础知识。在30分钟内，你应该能够下载并运行示例及使用本指南了解示例的构建方式。   

在尝试这个例子之后，全面的入门指南是继续学习更多关于Akka的一个很好的下一步。    
Akka平台指南讨论了更多Akka概念和功能，并概述了Akka作为工具包。  


### 下载example
1.下载案例 “zip file”(https://example.lightbend.com/v1/download/akka-quickstart-scala?name=akka-quickstart-scala&_ga=2.250121094.1465766975.1660790490-1530800744.1660790489) 

```shell
chmod u+x ./sbt
chmod u+x ./sbt-dist/bin/sbt
```

### 运行example
1. 在控制台，进入“akka-quickstart-scala”目录
```shell
cd akka-quickstart-scala
```

2. 通过sbt下载项目依赖项
```shell
# 下载项目需要的依赖，这一步会花费一些时间
./sbt

# 在sbt命令符中，输入 reStart
sbt:akka-quickstart-scala> reStart
```
会输出以下内容: 
```shell
akka-quickstart-scala [2022-08-18 23:53:55,306] [INFO] [com.example.Greeter$] [AkkaQuickStart-akka.actor.default-dispatcher-5] [akka://AkkaQuickStart/user/greeter] - Hello Charles!
akka-quickstart-scala [2022-08-18 23:53:55,308] [INFO] [com.example.GreeterBot$] [AkkaQuickStart-akka.actor.default-dispatcher-5] [akka://AkkaQuickStart/user/Charles] - Greeting 1 for Charles
akka-quickstart-scala [2022-08-18 23:53:55,309] [INFO] [com.example.Greeter$] [AkkaQuickStart-akka.actor.default-dispatcher-5] [akka://AkkaQuickStart/user/greeter] - Hello Charles!
akka-quickstart-scala [2022-08-18 23:53:55,309] [INFO] [com.example.GreeterBot$] [AkkaQuickStart-akka.actor.default-dispatcher-5] [akka://AkkaQuickStart/user/Charles] - Greeting 2 for Charles
akka-quickstart-scala [2022-08-18 23:53:55,309] [INFO] [com.example.Greeter$] [AkkaQuickStart-akka.actor.default-dispatcher-5] [akka://AkkaQuickStart/user/greeter] - Hello Charles!
akka-quickstart-scala [2022-08-18 23:53:55,309] [INFO] [com.example.GreeterBot$] [AkkaQuickStart-akka.actor.default-dispatcher-3] [akka://AkkaQuickStart/user/Charles] - Greeting 3 for Charles
sbt:akka-quickstart-scala>
``` 

恭喜你刚刚运行了你第一个Akka应用程序。现在来看看它做了哪些事。

### example做了哪些事？
example由3个actors组成：
* Greeter: 接受向某人打招呼的命令，并以Greeted回复作为回应，以确认打招呼已经发出。
* GreeterBot: 接受来自Greeter的回复并发送一些额外的问候消息并收集回复，直到达到给定的的最大消息数。
* GreeterMain: 引导一切的守护者演员


### 使用Actor模型的好处
Akka的以下特性使你能够以直观的方式解决并发性问题和扩展性挑战:
* 事件驱动模型 - 参与者执行工作以响应消息。Actor之间的通信是异步的，允许Actor发送消息并继续自己的工作，而不会阻塞等待回复。
* 强隔离原则 - 与Scala中的常规对象不同，Actor在你可以调用的方法方面没有公共API。相反，它的公共API是通过参与者处理的消息来定义的。这可以防止Actor之间的任务状态共享。观察另一个参与者状态的唯一的方法是向它发送一条请求它的消息。
* 位置透明性 - 系统从工厂构造Actor并返回对实例的引用。因为位置无关紧要，Actor实例可以启动、停止、移动和重新启动已扩大和缩小规模以及从意外故障中恢复。
* 轻量级 - 每个实例仅消耗几百个字节，这实例上允许数百万个并发Actor存在于单个应用程序中。

让我们看看在Example示例的上下文中使用Actor和消息的一些最佳实践。

