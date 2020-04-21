**正文**




## Java8中的常用函数式接口
![Java8中的常用函数式接口01](images/Java8中的常用函数式接口01.png)
![Java8中的常用函数式接口02](images/Java8中的常用函数式接口02.png)




## Predicate<T>
| 函数式接口      |    函数描述符 | 原始类型特化  |
| :-------- | --------:| :--: |
| Predicate<T>  | T->boolean |  InePredicate,LongPredicate,DoublePredicate   |

### 介绍谓词组合
谓词接口包括三个方法：negate，and和or，让你可以重用已有的Predicate来创建更复杂的谓词。比如，你可以使用negate方法来返回一个Predicate的非，比如苹果不是红的：
```java
//产生现有Prdicate对象redApple的非
Predicate<Apple> notRedApple = redApple.negate();
```
你可能想要把两个Lambda用and方法组合起来，比如一个苹果即是红色又比较重：
```java
//链接两个谓词来生成另一个Predicate对象
Predicate<Apple> redAndHeavyApple = redApple.and(a->a.getWeight()>150);
```
你可以进一步组合谓词，表达要么是重(150克以上)的红苹果，要么是绿苹果：
```java
//链接Predicate的方法来构造更复杂Predicate对象
Predicate<Apple> redAndHeavyAppleOrGreen = redApple.and(a->a.getWeight()>150).or(a->"green".equals(a.getColor()));
```
这一点为什么很好呢？从简单Lambda表达式触发，你可以构建更复杂的表达式，但读起来仍然和问题的陈述差不多！请注意，and和or方法是按照在表达式链中的位置，从左向右确定优先级的。因此，a.or(b).and(c)可以看作(a||b)&&c。


### Predicate<T>实战

1. test()
作用：
  * 评估参数里面的表达式
  * 它的返回值是一个boolean

局限性案例
```java
public static void main(String[] args) {
    Predicate<String> predicate = new Predicate<String>() {
        @Override
        public boolean test(String s) {

            return s.equals("zhangsan");
        }
    };

    System.out.println(predicate.test("lisi"));
    System.out.println("--- --- --- --- --- ---");
    System.out.println(predicate.test("zhangsan"));
}
```
代码中使用了匿名函数实例化了Predicate接口，在test方法中实现字符串的equals(), 当我们系统或者某个单一方法职责处理逻辑，这样写没啥问题，可有几个场景需要处理:
  * 接口中判断方法较多，比如 判断字符串长度是否大于5，字符串是否包含敏感字符
  * 类型改变，比如 支持数字判断是否大于5，传入的数字是否是奇数

局限性案例：
```java
//CheckValue
public interface CheckValue {
    public boolean stringEquals(String p);
    public boolean stringLength(String p);
    public boolean checkSensitiveStr(String p);

    public boolean intEquals(int p);
    public boolean checkOdd(int p);
}

//main
public static void main(String[] args) {
    CheckValue checkValue = new CheckValue() {
        @Override
        public boolean stringEquals(String p) {
            return false;
        }

        @Override
        public boolean stringLength(String p) {
            return false;
        }

        @Override
        public boolean checkSensitiveStr(String p) {
            return false;
        }

        @Override
        public boolean intEquals(int p) {
            return false;
        }

        @Override
        public boolean checkOdd(int p) {
            return false;
        }
    };
}
```

java8案例：
```java
public static void main(String[] args) {

    PredicateTest03 predicate = new PredicateTest03();

    System.out.println(predicate.stringConditionByFunction("lisi",value->value.equals("lisi")));
    System.out.println(predicate.stringConditionByFunction("shangsan",value->value.length()>5));
    
    System.out.println(predicate.intConditionByFunction(12345,value -> String.valueOf(value).length() > 5));
    System.out.println(predicate.intConditionByFunction(4,value -> value % 2 == 0));
}

//方法只需要定义2个
public boolean stringConditionByFunction(String value, Predicate<String> predicate) {
    return predicate.test(value);
}
public boolean intConditionByFunction(int value, Predicate<Integer> predicate) {
    return predicate.test(value);
}
```
  * 利用函数传参，无须在接口定义太多方法，实现代码精简
  * Predicate的test() 利用函数参数，就可以在test执行"是"

`and(),negate(),or(),isEqual() 返回的都是 Predicate<T> , test()返回的是boolean 所以在Predicate传参时候 一定要执行test(value)`

1. and()
等同于 逻辑与 && 
```java
public static void main(String[] args) {

    PredicateAND predicateAND = new PredicateAND();

    System.out.println(predicateAND.testAndMethod("zhangsan",
        stringOne -> stringOne.equals("zhangsan"),stringTwo -> stringTwo.length() > 5));
}

/**
    *
    * @param stringOne         待判断的字符串
    * @param predicateOne      断定表达式1
    * @param predicateTwo      断定表达式2
    * @return                    是否满足两个条件
    */
public boolean testAndMethod(String stringOne, Predicate<String> predicateOne,Predicate<String> predicateTwo) {

    return predicateOne.and(predicateTwo).test(stringOne);
}
```

3. negate()
等同于 逻辑非
```java
public static void main(String[] args) {

    PredicateNegate predicateNegate = new PredicateNegate();

    System.out.println(predicateNegate.testNageteMethod("zhangsan",stringOne -> stringOne.equals("zhangsan")));


}

public boolean testNageteMethod(String stringValue, Predicate<String> predicate) {
    //!(test(strinValue))
    return predicate.negate().test(stringValue);
}

//结果: false
```

4. or()
等同于 逻辑或
```java
public static void main(String[] args) {

    PredicateOr predicateOr = new PredicateOr();

    System.out.println(predicateOr.testOrMethod("zhangsan"
            , stringOne -> stringOne.equals("zhangsan111")
            ,stringTwo -> stringTwo.length() > 50
            ,stringThree -> stringThree.length() % 2 == 0));
}

public boolean testOrMethod(String stringOne, Predicate<String> predicateOne, Predicate<String> predicateTwo, Predicate<String> predicateThree) {
    
    //(predicateOne || predicateTwo||predicateThree)
    return predicateOne.or(predicateTwo).or(predicateThree).test(stringOne);
}
```

5. isEqual()
判断两个对象是否相等
```java
public static void main(String[] args) {

    PredicateIsEquals predicate = new PredicateIsEquals();

    String strNull = null;
    System.out.println(predicate.testMethodIsEquals("zhangsan","zhangsan"));
    System.out.println("~~~   ~~~   ~~~   ~~~");
    System.out.println(predicate.testMethodIsEquals("zhangsan","lisi"));
    System.out.println("~~~   ~~~   ~~~   ~~~");
    System.out.println(predicate.testMethodIsEquals(strNull,"zhangsan")); /* 我们来Debug一下这个程序*/

}


public boolean testMethodIsEquals(String strValue, String strValue2) {

    return Predicate.isEqual(strValue).test(strValue2);
}
```


> 摘自
     1. 《Java8实战》
     2. https://blog.csdn.net/qq_27416233/article/details/83418791
