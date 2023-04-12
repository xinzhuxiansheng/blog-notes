
# ACID靠什么保证的？

* A 原子性由undo log日志保证，它记录了需要回滚的日志信息，事务回滚时撤销已经执行成功的sql
* C 一致性由其他特性保证、程序代码要保证业务的一致性
* I 隔离性由MVCC来保证
* D 持久性由内存+redo log来保证，mysql修改数据同时在内存和redo log记录这次操作，宕机的时候可以从redo log回复

## redo log如何与bin log保持一致：
InnoDB redo log写盘，InnoDB 事务进入prepare状态，如果前面 prepare成功，binlog写盘，再继续将事务日志持久化到binlog，如果持久化成功，那么InnoDB事务则进入commit状态（在redo log里面写一个commit记录） 

redo log的刷盘会在系统空闲时进行