
# 自旋锁(spinlock)

## 1. 介绍
尝试获取锁的线程不会立即阻塞，而是采用循环的方式去尝试获取锁，这样的好处是减少线程上下文切换的消耗，缺点是循环会消耗CPU。

参考 `CountDownLatch`源码，采用自旋锁

`AbstractQueuedSynchronizer`
```java
/**
    * Release action for shared mode -- signals successor and ensures
    * propagation. (Note: For exclusive mode, release just amounts
    * to calling unparkSuccessor of head if it needs signal.)
    */
private void doReleaseShared() {
    /*
        * Ensure that a release propagates, even if there are other
        * in-progress acquires/releases.  This proceeds in the usual
        * way of trying to unparkSuccessor of head if it needs
        * signal. But if it does not, status is set to PROPAGATE to
        * ensure that upon release, propagation continues.
        * Additionally, we must loop in case a new node is added
        * while we are doing this. Also, unlike other uses of
        * unparkSuccessor, we need to know if CAS to reset status
        * fails, if so rechecking.
        */
    for (;;) {
        Node h = head;
        if (h != null && h != tail) {
            int ws = h.waitStatus;
            if (ws == Node.SIGNAL) {
                if (!compareAndSetWaitStatus(h, Node.SIGNAL, 0))
                    continue;            // loop to recheck cases
                unparkSuccessor(h);
            }
            else if (ws == 0 &&
                        !compareAndSetWaitStatus(h, 0, Node.PROPAGATE))
                continue;                // loop on failed CAS
        }
        if (h == head)                   // loop if head changed
            break;
    }
}
```

## 2. 实现自旋锁
```java
public class SpinLock {
    private static final Logger logger = LoggerFactory.getLogger(SpinLock.class);

    AtomicReference<Thread> atomicReference = new AtomicReference<>();

    public void myLock(){
        Thread thread = Thread.currentThread();
        logger.info("{} invoke myLock()", Thread.currentThread());
        while(!atomicReference.compareAndSet(null,thread)){

        }
    }

    public void myUnlock(){
        Thread thread = Thread.currentThread();
        atomicReference.compareAndSet(thread,null);
        logger.info("{} invoke myUnlock()");
    }

    public static void main(String[] args) {
        SpinLock spinLock = new SpinLock();
        new Thread(()->{
            spinLock.myLock();
            // do somethings

            spinLock.myUnlock();
        });
    }
}
```


## 3. Reference
https://www.geeksforgeeks.org/lock-free-stack-using-java/