## 匿名类与Lambda的区别 

### 匿名类
在学习了`Lambda`和`Functional Interface`之后，你可能会注意到它们与匿名内部类有些相似性：类型的联合声明和实例化。一个接口或者一个扩展类可以“即时”被实现，而无需一个单独的Java类，那么如果他们都需要实现一个具体的接口，**Lambda表达式和匿名类之间有什么不同呢？**    

从表面上看，由匿名类实现的函数接口与其lambda表达式表示形式非常相似，除了额外的样板代码，示例如下：  

`匿名类与lambda表达式比较`  
```java
// 函数接口（隐式）
public interface HelloWorld {
    String sayHello(String name);
}

// 作为匿名类
public static void main(String... args) {

    var helloWorld = new HelloWorld() {

        @Override
        public String sayHello(String name) {
            return "hello, " + name + "!";
        }
    };
}

// 作为lambda
public static void main(String... args) {

    HelloWorld helloWorld = name -> "hello, " + name + "!";
}
```

>这是否意味着Lambda表达式只是实现函数接口作为匿名类的语法糖？


>语法糖
语法糖描述了添加到语言中的特性，让你作为开发者的生活更“甜蜜”，所以某些结构可以更简洁或清晰地表达，或以另一种方式表达。      

>例如Java的import语句允许你在没有其完全限定名的情况下使用类型。另一个例子是使用var进行引用的类型推断，或者使用<>（钻石）操作符进行泛型类型推断。这两个特性都简化了你的代码，使之更适合“人类消费”。编译器将“去糖化”代码，并直接处理其“苦味”。        


Lambda表达式可能看起来像语法糖，但实际上它们的内涵要丰富得多。真正的差异除了冗长,在于生成的字节码，如示例所示，以及运行时如何处理它。   

`匿名类和Lambda之间的字节码差异`
```
// 匿名类
0: new #7 // class HelloWorldAnonymous$1  (一个新的匿名内部类HelloWorldAnonymous$1的对象在其周围的类HelloWorldAnonymous中创建)
3: dup
4: invokespecial #9 // Method HelloWorldAnonymous$1."<init>":()V (匿名类的构造函数被调用。在JVM中，对象的创建是2步进行的)
7: astore_1
8: return
// LAMBDA
0: invokedynamic #7, 0 // InvokeDynamic #0:sayHello:()LHelloWorld; (invokedynamic操作码隐藏了创建lambda背后的全部逻辑)
5: astore_1
6: return
```

这两种变体都有共同的`astore_1`调用，该调用将一个引用存储到局部变量中以及返回调用，所以这两种调用都不会成为字节码分析的一部分。  

匿名类示例创建一个新的匿名类型HelloWorldAnonymous$1的对象，产生三个操作码： 
* **new**: 创建一个新的未初始化的类型实例。
* **dup**: 通过复制将值放到栈顶。   
* **invokespecial**: 调用新创建的对象的构造方法来完成其初始化。     

另一方面，lambda示例不需要创建一个需要放到栈上的实例。相反，它使用单个操作码invokedynamic将创建lambda的整个任务委托给JVM。  

**INVOKEDYNAMIC指令**   
Java 7引入了新的JVM操作码`invokedynamic`，能支持像Groovy或JRuby这样的动态语言更灵活的方法调用。这个操作码是一个更多样化的调用变体，因为它的实际目标，比如方法调用或lambda body，在类加载时是未知的。JVM不是在编译时链接这样的目标，而是将动态调用站点与实际的目标方法链接起来。 

运行时在首次`invokedynamic调用`时使用一个“引导方法”来确定应该调用哪个方法。你可以将其视为利用JVM直接反射创建lambda的。这样，JVM可以通过使用不同的策略来优化创建任务，比如动态代理、匿名内部类，或java.lang.invoke.MethodHandle。    

`lambda和匿名内部类之间的另一个重大区别是它们各自的作用域。内部类创建其自己的作用域，隐藏其本地变量，这就是为什么this关键字引用的是内部类本身的实例，而不是周围的作用域。另一方面，lambda完全存在于它们周围的作用域。变量不能使用相同的名字重声明，而不是静态的情况下，this引用的是创建lambda的实例。`             

>正如你所看到的，lambda表达式并不是语法糖。     

refer   
1.https://github.com/benweidig/a-functional-approach-to-java    
2.《A Functional Approach to Java》    