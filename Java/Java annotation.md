**`正文`**

[TOC]

## 注解
注解为我们代码中添加信息提供了一种形式化的方法，使我们可以在稍后某个时刻非常方便地使用这些数据。
1.它可以提供用来完整地描述程序所需的信息，而这些信息是无法用Java来表达的。因此，注解使得我们能够以将由编译器来测试和验证的格式，存储有关程序的额外信息。
2.注解可以用来描述符文件，甚至或是新的类定义，并且有助于减轻编写"样板"代码的负担。通过使用注解，我们可以将这些元数据保存在Java源代码中，并利用annotation API为自己的注解构建处理工具，同时，注解的优点还包括：更加干净易读的代码

## 基本语法
注解的语法比较简单，除了@符号的使用之外，它基本与Java固有的语法一致。Java SE5内置了三种，定义在java.lang中的注解:
1.@Override,表示当前的方法定义将覆盖超类中的方法。如果你不小心拼写错误，或者方法签名对不上被覆盖的方法，编译器就会发出错误提示。
2.@Deprecated，如果程序员使用了注解为它的元素，那么编译器会发出警告信息。
3.@SuppressWarnings，关闭不当的编译器警告信息。在Java SE5之前的版本中，也可以使用该注解，不过会被忽略不起作用。

## 定义注解
1.@Target用来定义你的注解应该用于什么地方(例如是一个方法或者一个域)
2.@Rectetion用来定义该注解在哪一个级别可用，在源代码中(SOURCE),类文件中(CLASS)或者运行时(RUNTIME)
3.没有元素的注解称为标记注解


## 案例
下面是一个简单的注解，我们可以用它来跟踪一个项目中的用例。如果一个方法或一组方法实现了某个用例的足球，那么程序员可以为此方法加上该注解。于是，项目通过计算已经实现的用例，就可以很好地掌控项目的进展。而如果要更改或修改系统的业务逻辑，则维护该项目的开发人员也可以很容易德在代码中找到对应的用例。
```java
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface UseCase {
    public int id();
    public String description() default "no description";
}
```

注意, id和description类似方法定义






```java
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface Bean {
    String name() default "";
}
```
