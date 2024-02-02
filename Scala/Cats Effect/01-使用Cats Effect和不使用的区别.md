## 使用 Cats Effect 和不使用的区别  

### 使用Cats Effect的示例           
```scala
import cats.effect.{ExitCode, IO, IOApp}
import scala.concurrent.duration._

object CatsEffectExample extends IOApp {
  def run(args: List[String]): IO[ExitCode] = {
    val task1 = IO.sleep(2.seconds) >> IO(println("Task 1 complete"))
    val task2 = IO.sleep(1.second) >> IO(println("Task 2 complete"))
    val combined = for {
      fiber1 <- task1.start
      fiber2 <- task2.start
      _ <- IO(println("Both tasks started"))
      _ <- fiber1.join
      _ <- fiber2.join
    } yield ()
    combined.as(ExitCode.Success)
  }
}
```

### 不使用 Cats Effect的示例 
```scala
object NonCatsEffectExample {
  def main(args: Array[String]): Unit = {
    val task1 = new Thread {
      override def run(): Unit = {
        Thread.sleep(2000)
        println("Task 1 complete")
      }
    }
    val task2 = new Thread {
      override def run(): Unit = {
        Thread.sleep(1000)
        println("Task 2 complete")
      }
    }
    task1.start()
    task2.start()
    println("Both tasks started")
  }
}
```     

在这两个示例中，我们都模拟了两个并发任务的执行，一个延迟2秒，另一个延迟1秒。在使用Cats Effect的示例中，我们使用了IOApp和Cats Effect库提供的IO类型来管理并发操作，而不使用Cats Effect的示例中，我们`手动创建`线程来执行任务。        

需要注意的是，使用Cats Effect的示例中，我们利用了Cats Effect提供的start函数创建了Fiber来执行任务，并且可以通过>>组合操作符来顺序执行任务。而不使用Cats Effect的示例中，我们手动创建了线程，并且需要使用Thread.sleep来模拟延迟。     

这只是一个简单的示例，Cats Effect在处理更复杂的并发场景和错误处理时会提供更多的好处。实际应用中，使用Cats Effect可以提供更高效、安全和可维护的并发编程体验。    

>注意： 不使用 Cats Effect 程序的行为表现为在启动两个线程（task1 和 task2）之后打印 "Both tasks started"，然后等待这两个线程完成。这种行为的原因是 Java 程序（或 Scala 程序，因为 Scala 基于 JVM）在所有非守护线程执行完毕之前不会终止。    

在您的例子中，task1 和 task2 是由 new Thread 创建的普通线程，默认情况下，它们都是非守护线程。当这些线程被启动（task1.start() 和 task2.start()）时，它们会并行运行，而主线程（执行 main 方法的线程）继续执行并打印 "Both tasks started"。   

然而，尽管主线程可能在两个子线程完成之前就执行完毕了，JVM 不会立即退出，因为它还在等待所有非守护子线程完成。在您的例子中，JVM 会等待 task1 和 task2 这两个线程完成其 run 方法中的操作。只有当这两个线程都执行完毕，JVM 才会终止程序。      

这种行为与 cats-effect 中的行为有所不同，因为 cats-effect 提供了更多的控制和更丰富的异步操作处理方式。在 cats-effect 中，如果您启动了一个异步任务但没有显式地等待它（比如通过 join），主线程可能会在任务完成之前就结束，除非您提供了一种机制来确保等待这些任务。          


refer   
1.https://www.jianshu.com/p/082989e9cde6        
