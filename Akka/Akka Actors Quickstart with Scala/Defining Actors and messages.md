

## Definning Actors and messages

每个参与者为它可以接受的消息定义一个类型T。案例类和案例对象可以生成出色的消息，因为它们是不可变的并且支持模式匹配，我们将在Actor中对它接受到的消息进行匹配时利用这一点。

Hello World Actors用三种不同的消息：
* Greet：发送给Greeter Actor命令打招呼
* Greeted：来自Greeter演员的回复以确认问候已经发生
* SayHello： 向GreeterMain命令启动问候进程

