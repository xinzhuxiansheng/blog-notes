**`正文`**

[TOC]

## 用Optional取代null


## 应用Optional的几种模式
1. 创建Optional对象
    申明一个空的Optional，你可以通过静态工厂方法Optional.empty，创建一个空的Optional对象：
    ```java
    Optional<Car> optCar = Optional.empty();
    ```
2. 依据一个非空值创建Optional
    你还可以通过静态工厂方法Optional.of,依据一个非空值创建一个Optional对象：
    ```java
    Optional<Car> optCar = Optional.of(car);
    ```
    如果car是一个null，这段代码会立即抛出一个NullPointerException,而不是等到你试图访问car的属性值时才返回一个错误。
3. 可接受null的Optional
    最后，使用静态工厂方法Optional.ofNullable,你可以创建一个允许null值的Optional对象：
    ```shell
    Optional<Car> optCar = Optional.ofNullable(car);
    ```
    如果car是null，那么得到的Optional对象就是一个空对象。
    
