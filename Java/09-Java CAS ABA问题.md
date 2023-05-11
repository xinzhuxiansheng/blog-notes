## Java CAS ABA问题

>这篇Blog应嵌套CAS知识点里面介绍，但考虑ABA问题对于锁的学习有些承上启下的作用，故单独提出来

### 引言
CAS全名：Compare-and-Swap （比较并交换），CAS指令需要有三个操作数，分别是内存位置（在Java中可以简单地理解为变量的内存地址，用V表示）、旧的预期值（用A表示）和准备设置的新值（用B表示）。CAS指令执行时，当且仅当V符合A时，处理器才会用B更新V的值，否则它就不执行更新。但是，不管是否更新了V的值，都会返回V的旧值，上述的处理过程是一个原子操作，执行期间不会被其他线程中断。

下面我们通过一个多线程案例来分析：  
```java
public class AtomicTest {
    public static AtomicInteger race = new AtomicInteger(0);

    public static void increase() {
        race.incrementAndGet();
    }

    private static final int THREADS_COUNT = 20;

    public static void main(String[] args) {
        Thread[] threads = new Thread[THREADS_COUNT];
        for (int i = 0; i < THREADS_COUNT; i++) {
            threads[i] = new Thread(new Runnable() {
                @Override
                public void run() {
                    for (int i = 0; i < 10000; i++) {
                        increase();
                    }
                }
            });
            threads[i].start();
        }
        while (Thread.activeCount() > 1) {
            Thread.yield();
        }
        System.out.println(race);
    }
}
```
**输出结果**    
200000  

这段代码创建了20个线程来对race对象进行递增，通过多次测试，值一直都是`200000`,并没有因为多线程导致值被覆盖。这是因为`AtomicInteger类型的incrementAndGet()方法是通过CAS自旋（无锁）保证并发写入`。 下面我们来看下incrementAndGet()实现：  
```java
public final int getAndAddInt(Object var1, long var2, int var4) {
    int var5;
    do {
        var5 = this.getIntVolatile(var1, var2);
    } while(!this.compareAndSwapInt(var1, var2, var5, var5 + var4));

    return var5;
}
```




refer
1.《深入理解Java虚拟机》第三版
