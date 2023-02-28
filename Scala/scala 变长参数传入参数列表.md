
## ":_*"的作用
在scala开源项目中有大量的:_*，其作用是把Array、List转换为参数列表，作为变长参数传入参数列表。   

以下给出一段`StackOverflow`的FAQ refer: https://stackoverflow.com/questions/7938585/what-does-param-mean-in-scala   
``` 
a:A 为类型归属 ,: _* 是类型归属的一个特殊实例，它告诉编译器将序列类型的单个参数视为可变参数序列，即可变参数。       
``` 

**以下给出一些示例，更好理解** 
range01是Range类型，range01:_* 将它展开成一个列表

```scala
val range01 = 1 to 10
val t1 = Traversable(range01)
println(s"t1: $t1")
println(s"t1 size: ${t1.size}")
range01.foreach(r => print(s"$r, "))

println("")
println(" 1 to 10 use :_* ")

val t2 = Traversable(range01: _*)
println(s"t2: $t2")
println(s"t2 size: ${t2.size}")
```

Output: 
```
t1: List(Range 1 to 10)
t1 size: 1
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
 1 to 10 use :_* 
t2: List(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
t2 size: 10
```

