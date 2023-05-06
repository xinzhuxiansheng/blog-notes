


### Stream操作的三个步骤
* 创建Stream
* 中间操作
* 终止操作（终端操作）


### Stream创建注意事项
* Stream只能操作一次
* Stream方法返回的是新的流
* Stream不调用终结方法，中间的操作不会执行


### 筛选与切片
* filter
* limit
* skip(n)
* distinct


### 映射的概念和案例
映射 -> map、flatMap

### 排序的使用方法
sorted()、sorted(Compareator comp)

### 查找匹配的多种方法
anyMatch、allMatch、noneMatch、findFirst、findAny、count、max、min

### 规约的概念和案例
映射 -> reduce(T iden,BinaryOperator b) 、 reduce(BinaryOperator b)

### 内部迭代和外部迭代概念
内部迭代 -> 遍历将在集合内部进行，我们不会显示的去控制这个循环
外部迭代 -> 显示的进行迭代操作


### 流本源、流算路和并发流

并发流 -> 解决多线程环境下的原子性、竞争甚至锁问题
获取“并发流” -> .parallelStream() 、.stream().parallel()

流短路 -> 前面的操作会影响后面的操作