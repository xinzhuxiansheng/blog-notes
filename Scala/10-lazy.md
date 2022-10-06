
## lazy

我们可以用预初始化字段来精确模拟类构造方法人参的初始化行为。不过有时候我们可能更希望系统自己能搞定应有的初始化顺序。可以通过将val定义成惰性的来实现。如果我们在val定义之前加上lazy修饰符，那么右侧的初始化表达式就只会在val第一次 被使用时求值。    
例如可以像下面这样用val定义-一个Demo对象:   
```scala
scala> object Demo {
    val x = { println("initializing x"); "done" }
}
defined object Demo
```
现在，先引用一下Demo，再引用Demo.x :    
```scala
scala> Demo
initializing x
res3: Demo. type = Demo$@2129a843
scala> Demo.x
res4: String = done
``` 

正如你看到的，一旦我们使用Demo,其x字段就被初始化了。对x的初始化是Demo初始化的一部分。不过,如果我们将x定义为lazy,情况就不同了:   
```scala
scala> object Demo {
lazy val x = { println("initializing x"); "done" }
defined object Demo
scala> Demo
res5: Demo. type = Demo$@5b1769c
scala> Demo .X
initializing X 
res6: String = done
```

现在，Demo的初始化并不涉及对x的初始化。对x的初始化被延迟到第一次访问x的hi后。这跟x用def定义成无参方法的情况类似。不过，不同于def,惰性的val永远不会被求值多次。事实上，在对惰性的val首次求值之后,其结果会被保存起来，在后续的使用当中，都会复用这个相同的val。   

从这个例子看，像Demo这样的对象本身好像行为也跟惰性的val类似，它们也是在第一次被使用时按需初始化的。这是对的。事实上,对象定义可以被看作是惰性的val的一一种简写，即用匿名类来描述对象内容。   

通过使用惰性的val，可以重新编写RationalTrait,如示例20.8。在这个新的特质定义中，所有具体字段都被定义为lazy。跟前一个示例20.4的RationalTrait定义相比还有一个不同，那就是require子句从特质的定义体中移到了私有字段g的初始化代码中，这个字段计算的是numerArg和denomArg的最大公约数。有了这些改动，在LazyRat ionalTrait被初始化的时候，已经没有什么需要做的了，所有的初始化代码现在都已经是惰性的val的右侧的一-部分。因此，在类定义之后初始化LazyRationalTrait的抽象字段是安全的。   
```scala
trait LazyRationalTrait {
    val numerArg: Int
    val denomArg: Int
    lazy val numer = numerArg / g
    lazy val denom = denomArg / g
    override def toString = numer + "/" + denom
    private lazy val g= {
        require(denomArg != 0)
        gcd(numerArg，denomArg)
    }
    private def gcd(a: Int, b: Int): Int =
        if(b==0)aelsegcd(b，a%b)
}
```

参考下面这个例子:
```scala
scala> val x= 2
x:Int=2
scala> new LazyRationalTrait {
        val numerArg = 1 * X
        val denomArg = 2 * X
        }
res7: LazyRationalTrait = 1/2
``` 

并不需要预初始化任何内容，有必要跟踪- -下上述代码最终输出1/2这个字符串的初始化过程: 
1.LazyRationalTrait的一个全新示例被创建，LazyRationalTrait 的初始化代码被执行。这段初始化代码是空的,这时LazyRationalTrait还没有任何字段被初始化。  
2.由new表达式定义的匿名子类的主构造方法被执行。这包括用2初始化numerArg,'以及用4初始化denomArg。 
3.解释器调用了被构造对象的toString方法，以便打印出结果值。  
4.在LazyRationalTrait特质的toString方法中，numer 字段被首次访问，因此，其初始化代码被执行。 
5.numer的初始化代码访问了私有字段g，因而g随之被求值。求值过程访问到numerArg和denomArg，这两个变量已经在第2步被定义。    
6.toString方法访问denom的值，这将引发denom的求值。对denom的求值会访问denomArg和g的值。g字段的初始化代码并不会被重新求值，因为它已经在第5步完成了求值。  
7.最后，结果字符串"1/2"被构造并打印出来。   

注意g的定义在LazyRationalTrait类中出现在numer和denom的定义之后。尽管如此，由于所有三个值都是惰性的，g将在numer和denom的初始化完成之前被初始化。    

这显示出惰性的val的一个重要性质:它们的定义在代码中的文本顺序并不重要，因为它们的值会按需初始化。因而，惰性的val可以让程序员从如何组织val定义来确保所有内容都在需要时被定义的思考中解放出来。    

不过，这个优势仅在惰性的val的初始化既不产生副作用也不依赖副作用的时候有效。在有副作用参与时，初始化顺序就开始变得重要了。这种情况下要跟踪初始化代码运行的顺序就可能变得非常困难,就像前一例所展示的那样。因此，惰性的val是对函数式对象的完美补充，对函数式对象而言初始化顺序并不重要，只要最终所有内容都被正常初始化即可。对于那些以指令式风格为主的代码而言，惰性的val就没那么适用了。  

>惰性函数式编程语言
Scala并不是首个利用到惰性定义和函数式代码的完美结合的编程语言。事实上，有这样一整个类目的“惰性函数式编程语言”，其中所有的值和参数都是被惰性初始化的。这一类编程语言中最有名的是Haskell [SPJ02]。        