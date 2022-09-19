
## 密封类(sealed)

每当我们编写一个模式匹配时，都需要确保完整地覆盖了所有可能的case。有时候可以通过在末尾添加一一个缺省case来做到，不过这仅限于有合理兜底的场合。如果没有这样的缺省行为，我们如何确信自己覆盖了所有的场景呢?   

我们可以寻求Scala编译器的帮助，帮我们检测出match表达式中缺失的模式组合。为了做到这一点，编译器需要分辨出可能的case有哪些。一般来说，在Scala中这是不可能的，因为新的样例类随时随地都能被定义出来。例如没有人会阻止你在现在的四个样例类所在的编译单元之外的另一个编译单元中给Expr的类继承关系添加第五个样例类。   

解决这个问题的手段是将这些样例类的超类标记为密封(sealed) 的。密封类除了在同一个文件中定义的子类之外，不能添加新的子类。这一点对于模式匹配而言十分有用，因为这样- - 来我们就只需要关心那些已知的样例类。不仅如此，我们还因此获得了更好的编译器支持。如果我们对继承自密封类的样例类做匹配，编译器会用警告消息标示出缺失的模式组合。   

如果你的类打算被用于模式匹配，那么你应该考虑将它们做成密封类。只需要在类继承关系的顶部那个类的类名前面加上sealed关键字。这样，使用你的这组类的程序员在模式匹配你的这些类时，就会信心十足。这也是为什么sealed关键字通常被看作模式匹配的执照的原因。示例15.16给出了Expr被转成密封类的例子。   

`样例类和模式匹配`
```scala
sealed abstract class Expr
case class Var(name: String) extends Expr
case class Number (num: Double) extends Expr
case class Un0p( operator: String, arg: Expr) extends Expr
case class Bin0p( operator: String,
    left: Expr, right: Expr) extends Expr
```

现在我们可以试着定义一个漏掉了某些可能case的模式匹配:
```scala
def describe(e: Expr): String = e match {
    case Number(_) => "a. number" 
    case Var(_) => "a variable"
}
```
我们将得到类似下面这样的编译器警告:
```
warning; match is not exhaustive !
missing combination Un0p
missing combination BinOp
```
这样的警告告诉我们这段代码存在产生MatchError异常的风险，因为某些可能出现的模式( Un0p、Bin0p)并没有被处理。这个警告指出了潜在的运行时错误源，因此这通常有助于我们编写正确的程序。    

不过，有时候你也会遇到编译器过于挑剔的情况。举例来说，你可能从上下文知道你永远只会将describe应用到Number或Var,因此你很清楚不会有MatchError发生。这时你可以对describe添加一个捕获所有的case,这样就不会有编译器告警了:    
```scala
def describe(e: Expr): String = e match {
    case Number(_ _) => "a number"
    case Var(_) => "a variable"
    case _ => throw new Runt imeException //不应该发生
}
```
这样可行，但并不理想。你可能并不会很乐意，因为你被迫添加了永远不会被执行的代码(也可能是你认为不会)，而所有这些只是为了让编译器闭嘴。    

一个更轻量的做法是给match表达式的选择器部分添加一个@unchecked注解。就像这样:    
```scala
def describe(e: Expr): String = (e: @unchecked) match {
    case Number(_) => "a number"
    case Var(_) => "a variable"
}
```
一般来说，可以像添加类型声明那样对表达式添加注解:在表达式后加一个冒号和注解的名称(以@打头)。例如，在本例中我们给变量e添加了@unchecked注解，即“e: @unchecked"。 @unchecked注解对模式匹配而言有特殊的含义。如果match表达式的选择器带上了这个注解，那么编译器对后续模式分支的覆盖完整性检查就会被压制。