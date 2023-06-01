## TreeSet使用  

### 介绍    
在Scala中有叫`TreeSet`数据结构，它是一种有序集合，内部使用二叉搜索树实现，保证了元素的有序性和不重复性。值得注意的是，由于`TreeSet`内部需要对元素进行比较以保持顺序，因此TreeSet中的元素需要是可比较的。 在Scala中`TreeSet`也同样分区**不可变**和**可变**。  

### 案例  
定义`Person`样例类，它包含`name`,`age`两个字段，需求是先按照age从大到小排序，再按照name从大到小排序。由于Scala的TreeSet提供了一个隐式Ordering，该Ordering定义了元素之间的排序规则，所以定义了`customOrdering`隐式参数，不过它默认是降序，要想创建一个**倒序**的Ordering，我们需要首先为类型Person创建一个正序的Ordering，然后对这个Ordering调用reverse方法。这是因为Scala需要先知道如何对Person进行正序排序，才能知道如何对它进行倒序排序。 于是就有了`reverseOrdering`的定义。   

>在Scala中，隐式值（implicit values）是一种特殊的值，它们可以自动传递给需要这些值的函数或方法，而不需要明确地指定。你可以将它们理解为在代码的背景中默默工作的值。 但在本案例中定义了2个Ordering隐式参数，所以在TreeSet定义的时候需显示指定具体的隐式参数。  

```java
case class Person(name: String, age: Int)

object TreeSetTest {
  def main(args: Array[String]): Unit = {
    implicit val customOrdering: Ordering[Person] = Ordering.by(person => (person.age, person.name))
    implicit val reverseOrdering: Ordering[Person] = customOrdering.reverse

    val people = mutable.TreeSet(
      Person("Alice", 30),
      Person("Alice", 25),
      Person("Bob", 25),
      Person("Alice", 35),
      Person("Bob", 30)
    )(reverseOrdering) // 指定隐士的Ordering参数
    people.foreach(println)
  }
}
``` 
