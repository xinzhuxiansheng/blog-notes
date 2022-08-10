
Book: 《Java并发编程实战》
Book Code: https://jcip.net/


### Executor框架
任务是一组逻辑工作单元，而线程则是使任务异步执行的机制。    

`Executor接口`  
```java
package java.util.concurrent;
//.....

public interface Executor {

    /**
     * Executes the given command at some time in the future.  The command
     * may execute in a new thread, in a pooled thread, or in the calling
     * thread, at the discretion of the {@code Executor} implementation.
     *
     * @param command the runnable task
     * @throws RejectedExecutionException if this task cannot be
     * accepted for execution
     * @throws NullPointerException if command is null
     */
    void execute(Runnable command);
}
```
虽然Executor是个简单的接口，但它却为灵活且强大的异步任务执行框架提供了基础，该框架能支持多种不同类型的任务执行策略。它提供了一种标准的方法将任务的提交过程与执行过程解耦开来，并用Runnable来表示任务。Executor的实现还提供了对生命周期的支持，以及统计信息收集、应用程序管理机制和性能监视等机制。






### shutdownNow的局限性

当通过shutdownNow来强行关闭ExecutorService时，它会尝试取消正在执行的任务，并返回所有已提交但尚未未开始的任务，从而将这些任务写入日志或者保存起来以便之后进行处理。    

然而，我们无法通过常规方法来找出哪些任务已经开始但尚未结束。这意味着我们无法在关闭过程中知道正在执行的任务的状态，除非任务本身会执行某种检查。要知道哪些任务没有完成，你不仅需要知道哪些任务还没有开始，而且还需要知道当Executor关闭时哪些任务正在执行。    





### github








参考
https://www.cnblogs.com/xiaxj/p/14275671.html