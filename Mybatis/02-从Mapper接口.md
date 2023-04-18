## 从Mapper接口

### 引言
《01-Mybatis源码环境搭建》blog中，搭建了`javamain-mybatis`模块用来测试Mybatis-3源码，在`UserServlet`类doGet()方法中使用Mybatis查询DB中user表数据。那是如果做到的呢？    
**UserServlet**
```java
SqlSessionFactory sqlSessionFactory = MybatisUtils.sqlSessionFactory;
// 2. 从 SqlSessionFactory 中获取 SqlSession
try (SqlSession sqlSession = sqlSessionFactory.openSession()) {
    // 3. 获取 Mapper 接口的实例
    UserMapper userMapper = sqlSession.getMapper(UserMapper.class);

    // 4. 调用 Mapper 接口的方法执行 SQL 操作
    int userId = Integer.parseInt(req.getParameter("userId"));
    user = userMapper.getUserById(userId);
    

    ......部分片段
```

>在没有开始之前，先提一些待解释的问题：  
1. UserMapper是接口并没有定义实现类，而在调试过程中userMapper={$Proxy20@4223}，这是什么？
![userMapperProxy01](images/userMapperProxy01.png)

2. @Select 查询出来的数据 是如何映射到User对象的

3. #{} 与 ${} 生成sql的区别

4. #{} 是如何拼接参数，如何拼接多个参数

5. 查询数据的列名与User字段的映射规则是什么


### 有实现类的动态代理（$Proxy是由JDK动态代理生成）
如果**$Proxy**开头的类，你还暂时不清楚是什么？，建议你将下面的案例深刻理解一遍，说Mybatis动态之前，先来看一下JDK实现动态代理的案例：    
一般来说定义JDK动态代理分为三个步骤，如下所示：
1. 定义代理接口
2. 定义代理接口实现类
3. 定义动态代理调用处理器

那么，就按照以上步骤实现`JDK动态代理`

**定义代理接口**
Moveable接口定义一个move()方法，用来描述“移动”
```java
public interface Moveable {
    void move();
}
```

**定义代理接口实现类**
Car实现了Moveable，实现了小车是“如何移动”
```java
public class Car implements Moveable{
    @Override
    public void move() {
        //实现开车
        try {
            Thread.sleep(new Random().nextInt(1000));
            System.out.println("汽车行驶中....");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
```

**定义动态代理调用处理器**
TimeHandler实现InvocationHandler，它记录Car“移动了多长时间”，

>`InvocationHandler`是一个Java接口，它用于处理动态代理对象上的方法调用。简单来说，当您创建一个动态代理对象时，您需要提供一个实现了 InvocationHandler 接口的类。这个类中的invoke()方法会在代理对象上的任何方法被调用时执行。当您使用动态代理时，您无需显式地实现代理类。代理类是在运行时自动生成的，它将所有方法调用委托给InvocationHandler。这使得您可以在运行时动态地改变代理对象的行为，而无需更改或重新编译源代码。  

所以你会看到`TimeHandler`的invoke()，博主使用if判断当前调用的methodName是否是equals、hashCode 和 toString，这样在**调试过程中**避免打印重复log，`就像上面解释的那样调用被代理类的任何方法,都会执行invoke()`
```java
public class TimeHandler implements InvocationHandler {
    public TimeHandler(Object target) {
        super();
        this.target = target;
    }

    private Object target;

    /*
        InvocationHandler 的 invoke 方法在代理对象上的每个方法调用时都会被执行。通常，invoke 方法执行多次的原因是在使用代理对象时调用了多个方法。
        此外，一些方法，例如 equals、hashCode 和 toString，可能会在不经意间被调用。这可能是因为某些内部操作，例如在调试过程中 IDE 可能会调用 toString
        方法以显示对象的表示。这可能导致在调试过程中看到多次调用 invoke 方法。
        要解决这个问题，您可以在 InvocationHandler 的 invoke 方法中过滤掉不需要代理的方法
     */

    /*
     * 参数：
     * proxy  被代理对象
     * method  被代理对象的方法
     * args 方法的参数
     *
     * 返回值：
     * Object  方法的返回值
     * */
    @Override
    public Object invoke(Object proxy, Method method, Object[] args)
            throws Throwable {

        // 过滤掉不需要代理的方法，例如 equals、hashCode 和 toString
        if (method.getName().equals("equals") ||
                method.getName().equals("hashCode") ||
                method.getName().equals("toString")) {
            return method.invoke(target, args);
        }

        long starttime = System.currentTimeMillis();
        System.out.println("汽车开始行驶....");
        method.invoke(target);
        long endtime = System.currentTimeMillis();
        System.out.println("汽车结束行驶....  汽车行驶时间："
                + (endtime - starttime) + "毫秒！");
        return null;
    }
}
```

此时，编写main() 来测试以上代码
```java
public class TestMain {
    /**
     * JDK动态代理测试类
     */
    public static void main(String[] args) {
//        Car car = new Car();
//        InvocationHandler h = new TimeHandler(car);
//        Class<?> cls = car.getClass();
//        /**
//         * loader  类加载器
//         * interfaces  实现接口
//         * h InvocationHandler
//         */
//        Moveable m = (Moveable) Proxy.newProxyInstance(cls.getClassLoader(), cls.getInterfaces(), h);
//        m.move();

        Moveable car = new Car();
        Moveable proxy = (Moveable) Proxy.newProxyInstance(car.getClass().getClassLoader(), car.getClass().getInterfaces(),
                new TimeHandler(car));
        proxy.move();

        // 保存代理类的字节码到文件
        saveProxyClassToFile("$Proxy0.class");
    }

    /**
     * sun.misc.ProxyGenerator 类在 JDK 9 及更高版本中已被移除。在这些版本中，您需要使用第三方库，如 ASM 或 Javassist，生成动态代理类的字节码
     */
    private static void saveProxyClassToFile(String fileName) {
        byte[] classBytes = ProxyGenerator.generateProxyClass(
                "$Proxy0",
                new Class<?>[]{Moveable.class}
        );

        Path outputPath = Paths.get(fileName);
        try (FileOutputStream fos = new FileOutputStream(outputPath.toFile())) {
            fos.write(classBytes);
            System.out.println("Proxy class saved to: " + outputPath.toAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
```

从原理的角度上解析一下，main()方法是执行过程：  
第一步创建了Moveable的实现类    
第二步创建被代理对象的动态代理对象，这里有读者会有疑问，怎么证明这个是动态代理对象？ 如图所示   
![userMapperProxy02](images/userMapperProxy02.png)  
JDK动态代理对象名称是有规则的，凡是经过Proxy类生成的动态代理对象，前缀必然是`$Proxy`，后面的数字也是名称组成部分。  

如果读者想要一探究竟，查看`ProxyClassFactory`,
```java
private static final class ProxyClassFactory
    implements BiFunction<ClassLoader, Class<?>[], Class<?>>
{
    // prefix for all proxy class names
    private static final String proxyClassNamePrefix = "$Proxy";

    // next number to use for generation of unique proxy class names
    private static final AtomicLong nextUniqueNumber = new AtomicLong();
```

因为`$Proxy`类是由Java动态代理在运行时自动生成的类。这个类在内存中创建，并直接加载到 JVM 中，因此不会在本地文件中找到它，如果需进一步分析它，可以通过编程方式将生成的代理类的字节码（bytecode）保存到文件中，以便进行分析和调试。 `TestMain`的saveProxyClassToFile
()方法就是用来将proxy代理类打印到文件中，内容如下： 
```java
//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by FernFlower decompiler)
//

import com.javamain.proxy.Moveable;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.lang.reflect.UndeclaredThrowableException;

public final class $Proxy0 extends Proxy implements Moveable {
    private static Method m1;
    private static Method m3;
    private static Method m2;
    private static Method m0;

    public $Proxy0(InvocationHandler var1) throws  {
        super(var1);
    }

    public final boolean equals(Object var1) throws  {
        try {
            return (Boolean)super.h.invoke(this, m1, new Object[]{var1});
        } catch (RuntimeException | Error var3) {
            throw var3;
        } catch (Throwable var4) {
            throw new UndeclaredThrowableException(var4);
        }
    }

    public final void move() throws  {
        try {
            super.h.invoke(this, m3, (Object[])null);
        } catch (RuntimeException | Error var2) {
            throw var2;
        } catch (Throwable var3) {
            throw new UndeclaredThrowableException(var3);
        }
    }

    public final String toString() throws  {
        try {
            return (String)super.h.invoke(this, m2, (Object[])null);
        } catch (RuntimeException | Error var2) {
            throw var2;
        } catch (Throwable var3) {
            throw new UndeclaredThrowableException(var3);
        }
    }

    public final int hashCode() throws  {
        try {
            return (Integer)super.h.invoke(this, m0, (Object[])null);
        } catch (RuntimeException | Error var2) {
            throw var2;
        } catch (Throwable var3) {
            throw new UndeclaredThrowableException(var3);
        }
    }

    static {
        try {
            m1 = Class.forName("java.lang.Object").getMethod("equals", Class.forName("java.lang.Object"));
            m3 = Class.forName("com.javamain.proxy.Moveable").getMethod("move");
            m2 = Class.forName("java.lang.Object").getMethod("toString");
            m0 = Class.forName("java.lang.Object").getMethod("hashCode");
        } catch (NoSuchMethodException var2) {
            throw new NoSuchMethodError(var2.getMessage());
        } catch (ClassNotFoundException var3) {
            throw new NoClassDefFoundError(var3.getMessage());
        }
    }
}

```

可以看出生成的动态代理类，继承了Proxy类，然后对Moveable接口进行了实现，通过Idea或者class反序列工具jd-ui，查看move()方法调用的super.h正是Proxy.newProxyInstance()传入的InvocationHandler接口的实现类`TimeHandler`invoke()方法。

也就是说，在main()方法中调用proxy.move()时，方法调用链是这样的：
![invoker01](images/invoker01.png)  

以上介绍完JDK的动态代理的实现过程会发现main()中是有被代理接口的实现类Car，而**UserMapper是没有，那又是如何实现的呢？又有什么不同？**

不知道读者是否跟我一样有一种错觉，感觉@Select注解的SQL就像是接口方法的实现 :) ;

### 无实现类的动态代理
我们先来看下普通动态代理有没有可能不用实现类，仅靠接口实现呢？

**定义代理接口**
```java
public interface Moveable {
    void move();
}
```

**定义动态代理调用处理器**
应没有实现类，method.invoke()无形参可调用
```java
public class TimeHandler implements InvocationHandler {

    @Override
    public Object invoke(Object proxy, Method method, Object[] args)
            throws Throwable {

        // 过滤掉不需要代理的方法，例如 equals、hashCode 和 toString
        if (method.getName().equals("equals") ||
                method.getName().equals("hashCode") ||
                method.getName().equals("toString")) {
            return "";
        }

        long starttime = System.currentTimeMillis();
        System.out.println("汽车开始行驶....");
        // method.invoke();
        long endtime = System.currentTimeMillis();
        System.out.println("汽车结束行驶....  汽车行驶时间："
                + (endtime - starttime) + "毫秒！");
        return null;
    }
}
```

**TestMain**
```java
public class TestMain {
    public static void main(String[] args) {

        Moveable proxy = (Moveable) Proxy.newProxyInstance(Moveable.class.getClassLoader(),
                new Class[]{Moveable.class},
                new TimeHandler());
        proxy.move();
    }
}
```

可以看到，这里对`Proxy.newProxyInstance()`方法的参数作出了变化，之前是通过实现类





refer
1. https://www.51cto.com/article/640783.html
2. https://blog.mybatis.org/
3. https://mybatis.org/mybatis-3/zh/index.html