
## state

## CLH队列(Craig, Landin, and Hagersten lock queue)



## 方法介绍

### unsafe.objectFieldOffset()
```java
// AbstractQueuedSynchronizer.java
stateOffset = unsafe.objectFieldOffset
    (AbstractQueuedSynchronizer.class.getDeclaredField("state"));
```

objectFieldOffset()方法用于获取某个字段相对Java对象的“起始地址”的偏移量，也提供了getInt、getLong、getObject之类的方法可以使用前面获取的偏移量来访问某个Java对象的某个字段




### unsafe.compareAndSwapInt()
```java
// AbstractQueuedSynchronizer.java
protected final boolean compareAndSetState(int expect, int update) {
    // See below for intrinsics setup to support this
    return unsafe.compareAndSwapInt(this, stateOffset, expect, update);
}
```



### AbstractQueuedSynchronizer.Node
```java
static final class Node {
    // SHARED 共享模式 , EXCLUSIVE 独占模式
    /** Marker to indicate a node is waiting in shared mode */
    static final Node SHARED = new Node();
    /** Marker to indicate a node is waiting in exclusive mode */
    static final Node EXCLUSIVE = null;

    /** waitStatus value to indicate thread has cancelled */
    static final int CANCELLED =  1; // 节点被取消
    /** waitStatus value to indicate successor's thread needs unparking */
    static final int SIGNAL    = -1; // 代表后续节点需要被阻塞
    /** waitStatus value to indicate thread is waiting on condition */
    static final int CONDITION = -2; // 在条件队列中
    /**
        * waitStatus value to indicate the next acquireShared should
        * unconditionally propagate
        */
    static final int PROPAGATE = -3; // 用于共享模式，表示无条件传播

    /**
        * Status field, taking on only the values:
        *   SIGNAL:     The successor of this node is (or will soon be)
        *               blocked (via park), so the current node must
        *               unpark its successor when it releases or
        *               cancels. To avoid races, acquire methods must
        *               first indicate they need a signal,
        *               then retry the atomic acquire, and then,
        *               on failure, block.
        *   CANCELLED:  This node is cancelled due to timeout or interrupt.
        *               Nodes never leave this state. In particular,
        *               a thread with cancelled node never again blocks.
        *   CONDITION:  This node is currently on a condition queue.
        *               It will not be used as a sync queue node
        *               until transferred, at which time the status
        *               will be set to 0. (Use of this value here has
        *               nothing to do with the other uses of the
        *               field, but simplifies mechanics.)
        *   PROPAGATE:  A releaseShared should be propagated to other
        *               nodes. This is set (for head node only) in
        *               doReleaseShared to ensure propagation
        *               continues, even if other operations have
        *               since intervened.
        *   0:          None of the above
        *
        * The values are arranged numerically to simplify use.
        * Non-negative values mean that a node doesn't need to
        * signal. So, most code doesn't need to check for particular
        * values, just for sign.
        *
        * The field is initialized to 0 for normal sync nodes, and
        * CONDITION for condition nodes.  It is modified using CAS
        * (or when possible, unconditional volatile writes).
        */
    volatile int waitStatus; // 节点状态，用的就是上面四个字段

    /**
        * Link to predecessor node that current node/thread relies on
        * for checking waitStatus. Assigned during enqueuing, and nulled
        * out (for sake of GC) only upon dequeuing.  Also, upon
        * cancellation of a predecessor, we short-circuit while
        * finding a non-cancelled one, which will always exist
        * because the head node is never cancelled: A node becomes
        * head only as a result of successful acquire. A
        * cancelled thread never succeeds in acquiring, and a thread only
        * cancels itself, not any other node.
        */
    volatile Node prev; // 节点的前驱节点

    /**
        * Link to the successor node that the current node/thread
        * unparks upon release. Assigned during enqueuing, adjusted
        * when bypassing cancelled predecessors, and nulled out (for
        * sake of GC) when dequeued.  The enq operation does not
        * assign next field of a predecessor until after attachment,
        * so seeing a null next field does not necessarily mean that
        * node is at end of queue. However, if a next field appears
        * to be null, we can scan prev's from the tail to
        * double-check.  The next field of cancelled nodes is set to
        * point to the node itself instead of null, to make life
        * easier for isOnSyncQueue.
        */
    volatile Node next; // 后续节点

    /**
        * The thread that enqueued this node.  Initialized on
        * construction and nulled out after use.
        */
    volatile Thread thread; // 节点代表的线程

    /**
        * Link to next node waiting on condition, or the special
        * value SHARED.  Because condition queues are accessed only
        * when holding in exclusive mode, we just need a simple
        * linked queue to hold nodes while they are waiting on
        * conditions. They are then transferred to the queue to
        * re-acquire. And because conditions can only be exclusive,
        * we save a field by using special value to indicate shared
        * mode.
        */
    Node nextWaiter;

    /**
        * Returns true if node is waiting in shared mode.
        */
    final boolean isShared() {
        return nextWaiter == SHARED;
    }

    /**
        * Returns previous node, or throws NullPointerException if null.
        * Use when predecessor cannot be null.  The null check could
        * be elided, but is present to help the VM.
        *
        * @return the predecessor of this node
        */
    final Node predecessor() throws NullPointerException {
        Node p = prev;
        if (p == null)
            throw new NullPointerException();
        else
            return p;
    }

    Node() {    // Used to establish initial head or SHARED marker
    }

    Node(Thread thread, Node mode) {     // Used by addWaiter
        this.nextWaiter = mode;
        this.thread = thread;
    }

    Node(Thread thread, int waitStatus) { // Used by Condition
        this.waitStatus = waitStatus;
        this.thread = thread;
    }
}
```



```java
/**
    * Creates and enqueues node for current thread and given mode.
    *
    * @param mode Node.EXCLUSIVE for exclusive, Node.SHARED for shared
    * @return the new node
    */
// 节点插入到队尾的方法，mode是节点模式，共享/独占
private Node addWaiter(Node mode) {
    Node node = new Node(Thread.currentThread(), mode);
    // Try the fast path of enq; backup to full enq on failure
    Node pred = tail;
    if (pred != null) {
        node.prev = pred;
        if (compareAndSetTail(pred, node)) {
            pred.next = node;
            return node;
        }
    }
    enq(node);
    return node;
}
```




查看对象
https://openjdk.java.net/projects/code-tools/jol/
https://github.com/openjdk/jol

个人BLOG
https://www.iteye.com/blog/user/rednaxelafx