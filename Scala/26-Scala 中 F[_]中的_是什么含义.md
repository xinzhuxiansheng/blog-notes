## Scala 中 F[_]中的 '_' 是什么含义 
在 Scala 中，F[_]表示一个高阶类型（Higher-Kinded Type）或类型构造器（Type Constructor）。F是一个接受一个类型参数的抽象类型，而[_]则表示这个类型参数可以被任意类型替代。换句话说，F[_]表示一个通用的类型构造器，可以生成具体类型的容器或容器类。

在函数式编程中，使用高阶类型和类型构造器是为了支持泛型编程和抽象。通过使用F[_]，我们可以编写代码，它适用于一类通用的容器或计算上下文，而不仅仅是某个特定类型。

以下是一些常见的高阶类型和类型构造器的示例：

Option[_]：Option是一个类型构造器，Option[A]表示一个可能包含值（类型为A）或不包含值的容器。Option[_]表示可以替代A的任意类型。

List[_]：List是另一个类型构造器，List[A]表示一个包含类型为A的元素的列表。List[_]同样表示可以替代A的任意类型。

Future[_]：Future是一个类型构造器，用于表示异步计算的结果。Future[A]表示在未来某个时间点可能会获得一个类型为A的值的异步计算。Future[_]表示可以替代A的任意类型。

IO[_]：在 Cats Effect 等库中，IO表示一种描述延迟计算的数据类型。IO[A]表示一个延迟计算，可能在需要时执行，并且会产生类型为A的值。IO[_]表示可以替代A的任意类型。

总之，F[_]中的_表示一个类型参数的占位符，它可以用任何类型来替代，从而生成具体的类型。这种抽象概念在函数式编程中用于支持泛型和抽象化。


refer   
1.https://www.jianshu.com/p/18158357d360    