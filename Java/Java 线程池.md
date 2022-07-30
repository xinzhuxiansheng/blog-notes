
Book: 《Java并发编程实战》
Book Code: https://jcip.net/



### shutdownNow的局限性

当通过shutdownNow来强行关闭ExecutorService时，它会尝试取消正在执行的任务，并返回所有已提交但尚未未开始的任务，从而将这些任务写入日志或者保存起来以便之后进行处理。    

然而，我们无法通过常规方法来找出哪些任务已经开始但尚未结束。这意味着我们无法在关闭过程中知道正在执行的任务的状态，除非任务本身会执行某种检查。要知道哪些任务没有完成，你不仅需要知道哪些任务还没有开始，而且还需要知道当Executor关闭时哪些任务正在执行。    





### github








参考
https://www.cnblogs.com/xiaxj/p/14275671.html