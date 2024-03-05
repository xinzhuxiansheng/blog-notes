## Flink 源码 CompletableFuture 知识补充    

>在Flink 源码中 大量涉及 CompletableFuture 使用。所以需重点介绍  

### 1.简介    

#### 1.1 什么是CompletableFuture？
CompletableFuture是Java中⽤于异步编程的⼯具，它提供了⼀种⽅便的⽅式来处理异步任务的结果。       

#### 1.2 CompletableFuture的特点    
* 异步执⾏： 可以在新线程或现有线程池中异步执⾏任务。       
* 组合操作： ⽀持多个CompletableFuture的组合操作，实现更复杂的异步流程。        
* 异常处理： 提供了丰富的异常处理⽅法，使得异步任务出现异常时能够灵活处理。         


### 2.使用  

#### 2.1 CompletableFuture.supplyAsync()  
```java
// supplyAsync 有返回值 
// 提交异步任务stage时，可以指定执行用的线程池，如果不指定，则用默认的ForkJoinPool
CompletableFuture<String> ctf1 = CompletableFuture.supplyAsync(() -> {

    try {
        System.out.println(Thread.currentThread().getName() + ", task1 开始执行");
        Thread.sleep(2000);
        System.out.println(Thread.currentThread().getName() + ", task1 执行完毕");

    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }
    return "task1-res";
}/*,Executors.newFixedThreadPool(2)*/);
``` 

#### 2.2 CompletableFuture.supplyAsync()
```java
// runAsync 没有返回值 
CompletableFuture<Void> future =
CompletableFuture.runAsync(() -> {
// 异步执⾏的任务
});
```

#### 2.3 CompletableFuture.thenApply()
```java
// 对this_stage的结果 （ps: 2.1结果 ），进行变换
// apply stage 是沿用  操作stage:ctf1 的线程池来执行
CompletableFuture<String> ctf2 = ctf1.thenApply((s1) -> {
    try {
        System.out.println(Thread.currentThread().getName() + ", apply stage 开始执行");
        Thread.sleep(2000);
        System.out.println(Thread.currentThread().getName() + ", apply stage 执行完毕");
    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }
    return s1.toUpperCase();
});
```     

#### 2.4 CompletableFuture.thenApplyAsync() 
```java
// 带Async后缀： apply stage可以放在另行指定的与ctf1不同的线程池运行
// 如果不传线程池，则会放入 ForkJoinPool中执行 
CompletableFuture<String> applyAsync = ctf1.thenApplyAsync((s1) -> {
    try {
        System.out.println(Thread.currentThread().getName() + ", apply stage 开始执行");
        Thread.sleep(2000);
        System.out.println(Thread.currentThread().getName() + ", apply stage 执行完毕");
    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }
    return s1.toUpperCase();
});
```   

```java
// async后缀，提供传入线程池的功能，它会把apply stage中的任物逻辑放在传入的线程池中执行
// 如果不传入线程池，则apply stage是在操作stage原有的线程池中执行
CompletableFuture<String> applyAsync2 = ctf1.thenApplyAsync((s1) -> {
    try {
        System.out.println(Thread.currentThread().getName() + ", apply stage 开始执行");
        Thread.sleep(2000);
        System.out.println(Thread.currentThread().getName() + ", apply stage 执行完毕");
    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }
    return s1.toUpperCase();
}, Executors.newSingleThreadExecutor());      
```

#### 2.5 CompletableFuture.thenAccept() 
```java
// accept功能：接收上一个stage的结果，然后执行一个没有返回值的新stage
CompletableFuture<Void> future = ctf1.thenAccept((s) -> {
    try {
        System.out.println(Thread.currentThread().getName() + ", accept stage 开始执行");
        Thread.sleep(2000);
        System.out.println(Thread.currentThread().getName() + ", accept stage 执行完毕");

    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }
});
``` 

#### 2.6 CompletableFuture.thenRun() 
```java
// thenRun功能：消费上一个stage的完成“事件”，不会消费上一个stage的计算结果;
ctf1.thenRun(()->{
    try {
        System.out.println(Thread.currentThread().getName() + ", run stage 开始执行");
        Thread.sleep(2000);
        System.out.println(Thread.currentThread().getName() + ", run stage 执行完毕");
    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }
});
``` 

#### 2.7 CompletableFuture.thenCombine()  
```java
// thenCombine是对自身stage和传入的other stage进行合并，将它们俩的计算结果，作为入参，传给新的stage加工并返回最终结果
CompletableFuture<String> future = ctf1.thenCombine(ctf2, (s1, s2) -> {
    System.out.println(Thread.currentThread().getName() + ", combine stage 开始执行");
    return s1 + "----" + s2;
});
```     

#### 2.8 CompletableFuture.thenCompose()    


### 3.异常处理   

#### 3.1 CompletableFuture.whenComplete() 
```java
// whenComplete 可以接收上一个stage的正常结果和异常
// 然后 whenComplete 本身返回的future，装的是上一个stage的正常结果
CompletableFuture<String> future = ctf1.whenComplete((r, ex) -> {
    if (ex != null) {
        System.out.println("ctf1发生了异常");
    } else {
        System.out.println("ctf1正常完成了");
    }
});

if (!future.isCompletedExceptionally()) {
    System.out.println(future.get());
}
```

#### 3.2 CompletableFuture.handle()     
```java
CompletableFuture<String> future = ctf1.handle((r, ex) -> {
    if (ex == null) {
        return r.toUpperCase();
    } else {
        System.out.println("出了异常");
        return "异常后的默认值";
    }
});
System.out.println(future.get());
```