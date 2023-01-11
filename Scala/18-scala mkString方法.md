
## Scala mkString方法

>当你想要将集合（不区分可变与不可变）的元素转换成字符串，转换过程可能会添加分隔符、前缀或者后缀。那用mkString()再合适不过了。

### 实战mkString
```scala
object MkStringTest extends App {

  val a = Array("I","love","scala")

  // 字符串拼接形式
  println(a.mkString)

  // 以逗号分隔, 类似于java的 String.join()
  println(a.mkString(","))

  // 添加前缀和后缀
  println(a.mkString("[",",","]"))
}
```

**扩展**
如果你恰好有一个列表的列表，你想转换成一个String，比如下面的数组数组，先把集合展平，然后调用mkString。
```scala
val b = Array(Array("a", "b"), Array("c", "d"))
println(b.flatten.mkString(","))
```

