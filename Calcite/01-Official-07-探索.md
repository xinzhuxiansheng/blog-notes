

TraitSet 是 Apache Calcite 优化器中非常核心的概念，它描述了一个关系表达式（RelNode）不仅仅“是什么”（逻辑内容），而且“长什么样”（物理属性）。

可以把 RelNode 想象成一份快递：

RelNode 的逻辑内容：包裹里装的是什么（数据本身，比如 Emp 表所有 deptno=10 的行）。
TraitSet（特征集）：包裹上的标签（怎么运输、怎么排序、怎么分布）。
哪怕包裹里的东西一模一样（数据相同），如果标签不同，对于接收方（后续的操作算子）来说，处理方式也完全不同。

1. TraitSet 包含什么？
TraitSet 本质上是一组 RelTrait（关系特征）的集合。Calcite 默认定义了三种最常用的特征：

Convention（约定/执行规范）：最关键的特征。
含义：数据在哪里、以什么格式存在？
例子：
Convention.NONE：逻辑节点（只是个概念，没法运行）。
EnumerableConvention：Java 内存中的迭代器（Iterator）。
JdbcConvention：数据库里的表。
SparkConvention：Spark RDD。
影响：你不能把一个 JDBC 的表直接喂给一个 Spark 的算子，必须先做“转换”（Converter）。
Collation（排序/整理）：
含义：数据目前是按照什么顺序排列的？
例子：[deptno, empno] 表示先按部门号排，再按员工号排。
影响：如果你想做 MergeJoin（归并连接），输入数据必须是排好序的。如果当前的 TraitSet 显示数据是乱序的，优化器就会自动插入一个 Sort 算子来满足要求。
Distribution（分布）：
含义：数据在分布式系统中是怎么存放的？
例子：
SINGLETON：全在一台机器上。
HASH_DISTRIBUTED(id)：按 ID 哈希分散在多台机器上。
BROADCAST_DISTRIBUTED：广播到所有机器上。
影响：如果你要做 Join，通常要求两个表的数据在同一台机器上。如果分布特征不满足，优化器会插入 Exchange 算子（比如 Shuffle 或 Broadcast）来重分布数据。
2. TraitSet 在优化中的作用：红娘与媒婆
优化器（VolcanoPlanner）的工作原理很大程度上依赖于 TraitSet。它的核心逻辑是**“特征匹配”与“特质转换”**。

场景举例：

假设你有一个查询：SELECT * FROM A JOIN B ON A.id = B.id。

需求方（Join 算子）提出要求：
为了做高效的 MergeJoin，我要求输入 A 和 B 必须满足 TraitSet：{Convention: Enumerable, Collation: [id]}（即：要是 Java 内存对象，且按 id 排好序）。
供给方（Scan A 算子）的现状：
Scan A 目前的 TraitSet 是：{Convention: JDBC, Collation: []}（即：在数据库里，乱序）。
冲突与解决：
优化器发现 TraitSet 不匹配（TraitMismatch）。
它会寻找 Converter（转换器） 规则。
第一步：把 JDBC 转成 Enumerable -> 插入 JdbcToEnumerableConverter。
第二步：把 乱序 转成 Sort[id] -> 插入 Sort 算子。
最终，优化器通过不断比对 TraitSet，自动补全了转换路径。

3. 如何理解 plus 和 replace
在写 Calcite 规则（Rule）时，你经常会看到这两种操作：

traitSet.plus(trait)：
给特征集添加一个新的特征（如果同类型的特征已存在，则覆盖）。
注意：TraitSet 是不可变的（Immutable），所以这个方法会返回一个新的 TraitSet 对象。
常用误区：Calcite 默认配置下，一个 TraitSet 中每种类型的特征（TraitDef）只能有一个值。比如你不能既是 Convention.JDBC 又是 Convention.SPARK。
traitSet.replace(trait)：
和 plus 基本一样，也是替换某种类型的特征。
总结
这句话可以概括 TraitSet 的精髓：

RelNode 决定了逻辑上的“对错”，TraitSet 决定了物理上的“好坏”与“可行性”。

没有正确的 Convention，程序跑不起来（类型不匹配）。
没有正确的 Collation / Distribution，查询会非常慢（甚至因为没有利用排序而报错）。
优化器的本质工作，就是寻找一条路径，以最小的代价（Cost），把数据的 TraitSet 转换成用户查询所需的 TraitSet。
