
# InnoDB是如何实现事务的

InnoDB通过Buffer Pool，LogBuffer，Redo Log，Undo Log来实现事务，以一个update语句为例：
1. InnoDB在收到一个update语句后，会先根据条件找到数据所在的页，并将该页缓存在Buffer Pool中
2. 执行update语句，修改Buffer Pool中的数据，也就是内存中的数据
3. 针对update语句生成一个Redo Log对象，并存入LogBuffer中
4. 针对update语句生成undo log日志，用于事务回滚
5. 如果事务提交，那么则把Redo Log对象进行持久化，后续还有其他机制将Buffer Pool中所修改的数据页持久化到磁盘中
6. 如果事务回滚，则利用undo log日志进行回滚

