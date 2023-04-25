## javassist中文tutorial

### 1.Reading and writing bytecode（读写字节码）
Javassist是一个处理Java字节码的类库，Java字节码存储在一个称为类文件（.class）的二进制文件中。每个类文件包含一个Java类或接口。   
Javassist.CtClass类是类文件的抽象表示。CtClass（编译时类）对象是处理类文件的句柄。以下程序是一个非常简单的示例：
```java
ClassPool pool = ClassPool.getDefault();
CtClass cc = pool.get("test.Rectangle");
cc.setSuperclass(pool.get("test.Point"));
cc.writeFile();
```
此程序首先获取一个ClassPool对象，该对象控制Javassist的字节码修改,ClassPool对象是表示类文件的CtClass对象的容器。它根据需求读取类文件以构造CtClass对象，并记录已构造的对象以便以后访问。要修改类的定义，用户必须首先从ClassPool对象中获取表示该类的CtClass对象的引用。get()函数用于此目的。在上面显示的程序中，从 ClassPool对象中获取表示`test.Rectangle`类的CtClass对象，并将其分配给变量cc。getDefault()返回的ClassPool对象将搜索默认的系统搜索路径。




refer
1. Javassist http://www.javassist.org/tutorial/tutorial.html