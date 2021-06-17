[TOC]


## 特质用作接口

`结论：`
在绝大多数情况下，特质可以像Java的接口那样使用。在特质中，只需要声明继承类应该实现的方法。
类遵循如下规则通过extends或者with关键字继承特质:
* 如果一个类继承一个特质，使用extends。
* 如果一个类继承多个特质，第一个特质使用extends，其余的特质使用with。
* 如果一个类继承了一个类（或者抽象类）和一个特质，继承类（或者抽象类）使用extends，继承特质使用with。
  
也可以在特质中使用`字段`。


## 使用特质中的抽象字段和实际字段

`问题：`在特质中声明抽象或实际字段，以便任何 **继承了此特质的类都可以使用这些字段**

>note
(抽象/实际)字段：声明一个字段并给其赋一个初始值，这个字段就是实际字段，否则，这个字段就是抽象字段

```scala
trait PizzaTrait {
    var numToppings: Int  // abstract
    var size = 14         // concerte
    val maxNumToppings = 10 // concerte
}

在继承这个特质的类中，需要定义一个这些抽象字段的值，否则，这个类必须是抽象类。
```
`结论: `
一个特质的字段可以声明为var或者val。在一个子类（或子特质）中覆写特质中的var字段时，不需要override关键字，但当覆写一个特质中的val字段时，需要使用override关键字:

```scala
trait PizzaTrait {
    val maxNumToppings: Int
}

class Pizza extends PizzaTrait {
    override val maxNumToppings = 10 // 'override' is required
}
```





