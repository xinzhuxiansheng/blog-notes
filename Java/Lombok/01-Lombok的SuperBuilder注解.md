## Lombok的SuperBuilder注解

### 引言    
`@Builder` 和 `@SuperBuilder` 注解，它们可以帮助生成构建器（builder）模式的代码。`@Builder` 注解可以用来生成 builder pattern 风格的代码，这种模式特别适合在创建具有许多构造器参数的对象时使用。然而，`@Builder` 在处理继承关系的类时表现不佳。  

为了处理这种情况，Lombok 提供了 `@SuperBuilder` 注解。`@SuperBuilder` 可以被用来在具有继承关系的类上生成正确的构建器模式代码。使用 `@SuperBuilder`，你可以在子类中添加字段，并在构建对象时设置这些字段的值，同时保留父类字段的值。      

`@SuperBuilder` 的用法类似于 `@Builder`，但是它生成的代码和 `@Builder` 生成的代码有一些区别。特别是，`@SuperBuilder` 需要在每个子类上都使用这个注解，以保证生成正确的构建器代码。   

这是一个 `@SuperBuilder` 的简单示例：   

```java
import lombok.experimental.SuperBuilder;

@SuperBuilder
public class ParentClass {
    private String parentField;
}

@SuperBuilder
public class ChildClass extends ParentClass {
    private String childField;
}
```

在这个示例中，你可以使用如下的方式来构建一个 `ChildClass` 对象：    

```java
ChildClass child = ChildClass.builder()
    .parentField("Parent")
    .childField("Child")
    .build();
```
在这个例子中，你可以看到 `ChildClass.builder()` 是能够设置 `parentField` 和 `childField` 的。这就是 `@SuperBuilder` 的作用。    