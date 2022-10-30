
## Scala中的apply方法

>refer: https://blog.csdn.net/hzp666/article/details/115898835

### 场景
在Scla中实例化一个类，可以不用new，其原因是调用`伴生类`的apply()。 以下列举java与scala的差异:   
```java

// java
String [] myStrArr = new String[]{"BigData","Scala","Flink"}

// scala
val myStrArr = Array("BigData","Scala","Flink")
``` 
在上述的Scala代码中会生成一个Array对象，会自动去调用Array伴生类的apply()。

### apply()介绍
Scala中apply()最大的特点是 当一个实例化的类对象后面跟小括号(),并在小括号中传递了参数，那么Scala会自动在该类中查找apply()执行， ps: 这个apply()的参数要与实例化对象后面小括号内的参数一致    
**示例**    
```scala
class ApplyTest {
  def apply(name: String): Unit = {
    println(s"apply method, name is $name")
  }

}

object ApplyTest {
  def main(args: Array[String]): Unit = {
    val test = new ApplyTest
    test("yzhou")
  }
}
```

**apply()调用约定**     
用括号传递给类实例或单例对象名一个或多个参数时，有以下调用约定：    
* Scala会在相应的类或对象中查找方法名为apply的方法  
* 且参数列表与传入的参数一致的方法  
* 并用传入的参数来调用该apply()方法


### apply()作用
它利用伴生对象来创建实例化对象，因为object单例静态对象不能实例化。 `伴生对象，其实是一个与实体类名称相同的object单例对象`   



### 伴生对象中使用apply()
先通过一个示例来说明apply()的调用流程   
```scala
class ApplyTest(name: String) {
  def speak(): Unit = {
    println(s"$name is speaking")
  }

}

object ApplyTest extends App {
  def apply(name: String): ApplyTest = {
    new ApplyTest(name)
  }

  val p1 = ApplyTest("yzhou") // 调用伴生类中的apply()
  p1.speak()
}
``` 
打印的结果是: yzhou is speaking

**梳理流程**    
* 一个类class
* 给类定义一个伴生对象object
* 类的构造方法以apply方法的形式写在伴生对象中创建
* 实例化该类，scala会查找它的伴生类的apply() 且会创建类对象


### 使用case 取代 伴生对象中的apply()
在该段落之前，都是通过显示申明apply()方式来实例化对象，那是否存在无需显示申明apply()来创建对象？ 答案是肯定存在的 。 case会自动生成apply

>通过 case关键字，下面通过一些示例来介绍    
```scala
case class Dog(val name:String){
  def bark():Unit = {
    println("dog is wangwang")
  }
}

object Dog{
  def main(args: Array[String]): Unit = {
    val daHuang = Dog("daHuang") // 将Dog 申明为样例类
    daHuang.bark()
  }
}
```

