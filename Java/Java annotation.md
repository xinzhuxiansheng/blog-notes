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

## 元注解
| key      |    value |
| :-------- | :-------- |
| @Target  | 表示该注解可以用于什么地方。可能的ElementType参数包括：CONSTRUCTOR：构造器的生命，FIELD: 域声明(包括enum实例)，LOCAL_VAVIABLE：局部变量声明，METHOD:方法声明，PACKAGE：包声明，PARAMETER:参数声明，TYPE：类、接口（包括注解类型）或enum声明|
| @Retention  | 表示需要在什么级别保存该注解信息。可选的RetentionPolicy参数包括：
SOURCE:注解将被编译器丢弃，CLASS：注解在class文件中可用，但会被VM丢弃，RUNTIME:VM将在运行期也保留注解，因此可以通过反射机制读取注解的信息|
| @Documented  | 将此注解包含在Javadoc中|
| @Inherited  | 允许子类继承父类中的注解|

大多数时候，程序员主要是定义自己的注解，并编写自己的处理器来处理他们。

## 注解元素
注解元素可用的类型如下所示：
* 所有基本类型(int,float,boolean  ...)
* String
* Class
* enum
* Annotation
以上类型的数组


## 默认值限制
* 元素不能有不确定的值，也就是说，元素必须要么具有默认值，要么在使用注解时提供元素的值
* 对于非基本类型的元素，无论是在源代码声明时，或是在注解接口中定义默认值时，都不能以null作为其值。这个约束使得处理器很难表现一个元素的存在或缺失的状态，因此在每个注解的声明中，所有的元素都存在，并且都具有相应的值。为了绕开这个约束，我们只能自己定义一些特殊的值，例如`空字符串或负数`，以此表示某个元素不存在;


```java
//在class上定义
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.Type)
public @interface JsonSerializable{

}

//在field上定义
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface JsonElement{
    public String key() default "";
}

//在method上定义
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
public @interface JsonSerializable{

}

//通过特殊的默认值)(空字符串或负数) 来标记 该字段缺失的状态
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface SimulatingNull(){
    public int id() default -1;
    public String description() default "";
}

//

```


## 简单场景
下面是一个简单的注解，我们可以用它来跟踪一个项目中的用例。如果一个方法或一组方法实现了某个用例的足球，那么程序员可以为此方法加上该注解。于是，项目通过计算已经实现的用例，就可以很好地掌控项目的进展。而如果要更改或修改系统的业务逻辑，则维护该项目的开发人员也可以很容易德在代码中找到对应的用例。
```java
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface UseCase {
    public int id();
    public String description() default "no description";
}
```


注意, id和description类似方法定义。由于编译器会对id进行类型检查，因此将用例文档的追踪数据库与源代码相关联是可靠的。description元素有一个default值，如果在注解某个方法时没有给出description的值，则该注解的处理器就会使用此元素的默认值。
```java
public class PasswordUtils{

    @UseCase(id=47,description = "Passwords must contain at least one numeric")
    public boolean validatePassword(String password){
        return (password.matches("\\w*\\d\\w*"));
    }

    @UseCase(id=48)
    public String encryptPassword(String password){
        return new StringBuilder(password).reverse().toString();
    }

    @UseCase(id=49,description = "New passwords can't equal previously used ones")
    public boolean checkForNewPassword(List<String> prevPasswords,String password){
        return !prevPasswords.contains(password);
    }
}
```


## 编写注解处理器
下面是一个非常简单的注解处理器，我们将用它来读取PasswordUtils类，并使用发射机制查找@UseCase标记。我们为其提示了一组id值，然后它会列出PasswordUtils中找到的用例，以及缺失的用例。
```java
public class UseCaseTracker{
    public static void trackUseCases(List<Integer> useCases,CLass<?> cl){
        for(Method m:cl.getDeclaredMethods()){
            UseCase uc = m.getAnnotation(UseCase.class);
            if(uc != null){
                System.out.println("Found Use Case:" + uc.id()+" "+uc.description());
                useCases.remove(new Integer(uc.id()));
            }
        }
        for(int i:useCases){
            System.out.println("Warning:Missing use case-" + i);
        }
    }

    public static void main(String[] args){
        List<Integer> useCases = new ArrayList<Integer>();
        Collections.addAll(useCases,47,48,49,50);
        trackUseCases(useCases,PasswordUtils.class);
    }
}
```
这个程序用到两个反射的方法：`getDeclaredMethods()`和`getAnnotation()`，它们都属于AnnotatedElement接口(Class,Method与Field等类都实现了该接口)。getAnnoation()方法返回指定类型的注解对象，在这里就是UseCase。如果被注解的方法上没有该类型的注解，则返回null值。然后我们通过调用id()和description()方法从返回的UseCase对象中提取元素的值。其中，encriptPassword()方法在注解的时候没有指定description的值，因此处理器在处理它对应的注解时，通过description()方法取得的时默认值no description。



> 参考：《java编程思想》 注解
