
# AOP讲解

## 1. 什么是AOP?
“面向切面编程” 也被称为 “Aop” 是目前非常活跃的一个开发思想。利用 AOP 可以对业务逻辑的各个部分进行隔离，从而使得业务逻辑各部分之间的耦合度降低，提高程序的 可重用性，同时提高了开发的效率。  

Aop 编程的目的是将例如日志记录、性能统计、安全控制、事务、异常处理等代码从业 务逻辑代码中划分出来。 

比方说我有一个查询用户信息的接口，现在要为这个接口添加记录的功能。每当执行一 次查询都记录下查询消耗时间。如果我要实现这个功能，一般情况下需要在接口实现类的每 一个方法前后都要安插代码来收集数据。如果这样做的话会比较繁琐，但是通过 Aop 的方式 就显得非常优雅。    

实现 Aop 编程模型分为（静态代理、动态代理）两种方式，其中： 
* 静态代理多以代理模式（Proxy Pattern）的形式出现。
* 动态代理则花样繁多，常见的有：Java 原生的 Propxy、 CGLib、JBossAOP、等等  

## 2. 静态代理
* 假设有一个工厂，工厂里的工人上下班每次都需要打卡。那么这个工厂的工人可以抽象为 Worker 接口、工作可以被抽象成为 doWork 方法。

一个对象化的工人如下：
```java
public interface Worker {
    public void doWork();
}
```

打卡分为上班打卡和下班打卡，为此抽象一个打卡机，并将上下班打卡使用 beforeWork 和 afterWork 方法表示。如下：
```java
public class Machine {
    public void beforeWork() {
        ...
    }
 
    public void afterWork() {
        ...
    }
}
```

工厂规定每个员工只要来到工厂就视为上班打卡、当离开工厂就被认为下班打卡。为了 人性化考勤，公司使用了一种现代化的技术可以让员工不必自己动手去打卡，犹如配备了一 名贴身小秘书。其实不难看出这项新技术仅仅是围绕着工人（Worker）在工作（doWork）前后实现了自动打卡。下面是这个技术的抽象：
```java
public class WorkerProxy implements Worker {
    private Machine machine;
    private Worker targetWorker;
 
    public void doWork() {
        this.machine.beforeWork();
        this.targetWorker.doWork();
        this.machine.afterWork();
    }
}
```

## 3. 动态代理
* 在静态代理中所有类型都是衡定的。

在程序执行时，代理类(WorkerProxy)的 class 文件已经预先存在。在动态代理中这却恰恰相反的，代理类不会预先存在，当需要它的时候通过一些专门的类库创建这个代理程序。

比方说一个程序中有多种不同的 Servies 类。我们要打印出调用每个业务方法所占用的 时间。如果使用静态代理方式会发现，程序中根本不存在衡定的“doWorker”方法。

虽然不存在衡定的“doWorker”方法，但是调用行为是存在的。而且可以将其行为抽象 出来这就是 Aop 中的“切面”，负责执行这个切面的类就叫“拦截器”。下面这个代码展示 了如何用 Java 的原生支持实现动态代理。

### 方法级拦截器

### 类级别拦截器

### 全局拦截器

### 复合拦截器

### 自定义拦截器

### 忽略动态代理