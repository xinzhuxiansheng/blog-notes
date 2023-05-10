## final关键字01

### 引言
在学习JVM中的类加载机制中，了解到`final`关键字修饰变量时，编译器就对其进行优化操作。我们先从一个案例描述：    

```java
// ConstClass
public class ConstClass {
    static {
        System.out.println("ConstClass init!");
    }
    // 无final关键字
    public static String HELLOWORLD = "hello world";
}

// Notnitialization
public class Notnitialization {
    public static void main(String[] args) {
        // 测试03
        System.out.println(ConstClass.HELLOWORLD);
    }
}
```
**输出结果**    
```shell
ConstClass init!
hello world
```

将ConstClass类中的HELLOWORLD字段添加`final`关键字   
```java
// ConstClass
public class ConstClass {
    static {
        System.out.println("ConstClass init!");
    }
    // 添加final关键字
    public static final String HELLOWORLD = "hello world";
}
```
**输入结果**    
```shell
# 注意：若在idea中无法复现,删除target，重新启动main即可
hello world
```

此时，ConstClass的static并没有打印，可以说明该类没有初始化

### 分析
这是因为虽然在Java源码中确实引用了ConstClass类的常量HELLOWORLD，但其实在编译阶段通过常量传播优化，已经将此常量的值“hello world”直接存储在NotInitialization类的常量池中，以后NotInitialization对常量ConstClass.HELLOWORLD的引用，实际都被转化为NotInitialization类对自身常量池的引用了。也就是说，实际上NotInitialization的Class文件之中并没有ConstClass类的符号引用入口，这两个类在编译成Class文件后就已不存在任何联系了。

由于final字段的值不会改变，因此编译器可能会在编译时期就对这些字段进行优化。 


<br/>
<br/>
<br/>
<br/>
<br/>

refer
1.《深入理解Java虚拟机》第三版




