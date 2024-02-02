## Scala 中 Monad、Sync 和 Concurrent 

在函数式编程中，Monad、Sync 和 Concurrent 是三个常见的类型类，用于描述不同类型的副作用和并发性操作。它们之间有关系，但也有一些区别：

### Monad（单子）：  

Monad 是最基本的类型类之一，用于表示可以进行顺序计算的上下文（或容器）。    
Monad 提供了 flatMap（>>=）操作，用于将一个值从一个上下文中提取并传递给下一个计算，同时考虑了上下文中的副作用。   
例子：Option、Either、Future 都是 Monad 的实例。   

### Sync（同步）：   

Sync 是一个高级类型类，它扩展了 Monad 的概念，用于表示可以进行副作用和阻塞操作的上下文。    
Sync 提供了阻塞操作的能力，允许在纯函数式环境中执行阻塞的 I/O 操作。     
例子：IO 类型（在 Cats Effect 或 Monix 中）是 Sync 的实例。    

### Concurrent（并发）：

Concurrent 是更高级的类型类，它扩展了 Sync 的概念，用于表示可以进行并发操作的上下文。      
Concurrent 提供了执行并发计算的能力，允许以纯函数式的方式处理并发操作，例如并行计算和协作多线程操作。    
例子：IO 类型（在 Cats Effect 或 Monix 中）通常也是 Concurrent 的实例。    

联系和区别：

Sync 和 Concurrent 都扩展了 Monad，因此它们都支持 flatMap 操作。        
Sync 和 Concurrent 均用于处理副作用，但 Sync 更关注阻塞式的 I/O 操作，而 Concurrent 更关注并发性。     
Concurrent 具有在多线程环境中执行计算的能力，因此它通常还提供了诸如 parMapN、race、async 等操作，以支持并行计算和协作多线程操作。     
Concurrent 可以用于构建高性能、高并发的应用程序，而 Sync 用于执行有阻塞操作的应用程序。       
在某些函数式编程库中，IO 类型通常是 Sync 和 Concurrent 的实例，因此它可以处理副作用、阻塞操作和并发性。        

refer   
1.https://www.jianshu.com/p/7a3b7da4d687            

