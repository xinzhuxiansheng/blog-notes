## Java ConcurrentHashMap讲解  

>JDK version: 1.8   

### 引言    
在Java多线程开发场景中，`ConcurrentHashMap`替代了`HashMap`作为kv的存储容器，保证了多线程操作下的线程安全及效率，而ConcurrentHashMap选择了与HashMap相同的Node数组+链表+红黑树结构；在线程安全-锁的实现上，抛弃了原有的Segment分段锁，采用`CAS + synchronized`实现更加细粒度的锁，这样锁的级别控制在了更细粒度的哈希桶数组元素级别，也就是说只需要锁住这个链表头节点（红黑树的根节点）。  

### 内部属性及结构介绍  

**private transient volatile int sizeCtl;**     



### Put()



refer   
1.https://segmentfault.com/a/1190000039087868   
2.https://blog.csdn.net/programerxiaoer/article/details/80040090    