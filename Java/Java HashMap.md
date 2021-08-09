# HashMap讲解

`JDK1.8`

## 1. 简介
HashMap是一个散列表，它存储的内容是键值对(key-value)映射，它实现了Map接口，根据key的HashCode进行位运算存储数据，支持key=null，value=null存储。HashMap是无序的且不是线程安全。

## 2. HashMap使用
以下代码示例 部分展示HashMap API使用
```java
Map<String,String> map = new HashMap<>();
// 添加元素
map.put("name","young");
// 查询元素
String name = map.get("name");
// 删除元素
map.remove("name");
// 获取map size大小
System.out.println("map size: "+map.size());
```

> HashMap讲解

### 2.1 HashMap构造方法
HashMap的构造方法中将`loadFactor`(负载因子)默认赋值是0.75。
```java
Map<String,String>  map = new HashMap<>();

public HashMap() {
    this.loadFactor = DEFAULT_LOAD_FACTOR; // all other fields defaulted
}
```

### 2.2 put()处理流程
```java
map.put("name","young");
```

`流程介绍`
**step01.** 判断table数组是否为null或者空，如果为null或者为空则调用resize()方法进行初始化    
**step02.** 根据key的hash计算其在数组中的位置
```java
// 1.利用高位与key的hash值进行异或运算
(h = key.hashCode()) ^ (h >>> 16) 

// 2.数组容量值进行与运算 (& 两者都为1，才为1)
(p = tab[i = (n - 1) & hash]) 
```
> p表示的是 map.put("name","young")，所要插入的数组下标的对象   

**step03.** 如果p为空则新建一个节点放入该数组下标   

**step04.** 如果p不为空，判断当前的hash、key所引用的内存地址或者key的值是否相等。 若相等，则将p赋值给临时节点e  

**step05.** 判断p是否是红黑树则插入树中，如果p是链表则插入队尾  
因为e=p.next， 
`新节点插入队尾`
```java
for (int binCount = 0; ; ++binCount) {
    if ((e = p.next) == null) {
        p.next = newNode(hash, key, value, null);
    }
    if (e.hash == hash &&
        ((k = e.key) == key || (key != null && key.equals(k))))
        break;
    p = e;
}
```

**step06.** 链表尾部插入完成后判断当前链表长度是否大于等于8，若是链表构建成红黑树。 

**step07.** 如果p.key已存在则返回旧的value。    

**step08.** 如果是新插入的key，则modCount++, 当map个数大于安全阈值,则进行扩容，再返回null。     


```java
final V putVal(int hash, K key, V value, boolean onlyIfAbsent,
                boolean evict) {
    Node<K,V>[] tab; Node<K,V> p; int n, i;
    // step01
    if ((tab = table) == null || (n = tab.length) == 0)
        n = (tab = resize()).length;
    // step02
    if ((p = tab[i = (n - 1) & hash]) == null)
        // step03
        tab[i] = newNode(hash, key, value, null);
    else {
        Node<K,V> e; K k;
        // step04
        if (p.hash == hash &&
            ((k = p.key) == key || (key != null && key.equals(k))))
            e = p;
        // step05
        else if (p instanceof TreeNode)
            e = ((TreeNode<K,V>)p).putTreeVal(this, tab, hash, key, value);
        else {
            for (int binCount = 0; ; ++binCount) {
                if ((e = p.next) == null) {
                    p.next = newNode(hash, key, value, null);
                    // step06
                    if (binCount >= TREEIFY_THRESHOLD - 1) // -1 for 1st
                        treeifyBin(tab, hash);
                    break;
                }
                if (e.hash == hash &&
                    ((k = e.key) == key || (key != null && key.equals(k))))
                    break;
                p = e;
            }
        }
        // step07
        if (e != null) { // existing mapping for key
            V oldValue = e.value;
            if (!onlyIfAbsent || oldValue == null)
                e.value = value;
            afterNodeAccess(e);
            return oldValue;
        }
    }
    // step08
    ++modCount;
    if (++size > threshold)
        resize();
    afterNodeInsertion(evict);
    return null;
}
```


### 2.1 Node<K,V>[] table数组初始化
```java
newCap = DEFAULT_INITIAL_CAPACITY;
newThr = (int)(DEFAULT_LOAD_FACTOR * DEFAULT_INITIAL_CAPACITY);

Node<K,V>[] newTab = (Node<K,V>[])new Node[newCap];
```

### 2.2 Hash表的索引
&运算是两位都为1才为1，


`(h = key.hashCode()) ^ (h >>> 16)` 这里是用 key 的 hashCode 的高 16 位与低 16 位进行异或操作。由于 1.8 中引入了红黑树来处理链表过长的情况，所以这里不像 1.7 需要进行多次扰动，减少性能损失。而且因为 map 里哈希表的大小限制高位是不会参与计算的，这样使用高 16 位与低 16 位进行异或可以混合高低位的信息使高位参与到计算中。
```java
(p = tab[i = (n - 1) & hash])

static final int hash(Object key) {
    int h;
    return (key == null) ? 0 : (h = key.hashCode()) ^ (h >>> 16);
}
```

### 2.2 Table初始化

```java
newCap = DEFAULT_INITIAL_CAPACITY;
newThr = (int)(DEFAULT_LOAD_FACTOR * DEFAULT_INITIAL_CAPACITY);
```


## Q&A
1. 为什么需要使用 2 次幂来作为哈希表的大小，并且扩容是扩容成原来的两倍呢？




https://tech.meituan.com/2016/06/24/java-hashmap.html

https://www.logicbig.com/tutorials/core-java-tutorial/java-collections/hash-map-equal-and-hash-code.html


https://cdgeass.github.io/2021/04/29/jdk-hashmap.html


https://javahungry.blogspot.com/2013/08/hashing-how-hash-map-works-in-java-or.html