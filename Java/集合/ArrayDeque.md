## ArrayDeque

ArrayDeque集合它是一个基于数据（可扩容的数组）结构实现的双端队列。与普通的数组结构相比，这种数组结构是一种可循环使用的数据结构，可以有效减少数据扩容的次数。`ArrayDeque集合时线程不安全的,不能在多线程场景使用`。   

ArrayDeque集合既有队列、双端队列的操作特点，又有栈结构的操作特点。因此在JDK1.6发布之后，ArrayDeque集合时官方推荐的继Stack集合和LinkedList集合后，用于进行栈结构操作的新集合。


### ArrayDeque集合的主要结构及相关方法
ArrayDeque集合的内部结构是一个循环数组，该循环数组在ArrayDeque集合中的变量名为elements；ArrayDeque集合中有一个名为`head`的属性，主要用于标识下一次进行移除操作的数据对象索引位（队列头部的索引位）；ArrayDeque集合中还有一个名为`tail`的属性，主要用于标识下一次进行添加操作的数据对象索引位（队列尾部的索引位）。head属性和tail属性所标识的有效数据范围在不停地变化，甚至有时tail属性记录的索引值会小于head属性记录的索引值，但这丝毫不影响他们对有效数据范围的标识。



**将ArrayDeque集合作为队列结构使用**
只允许在ArrayDeque集合的尾部添加数据对象，在ArrayDeque集合的头部移除或读取数据对象：    

| 方法名      |    方法意义 | 
| :-------- |:--------|
| add(e)  | 在队列尾部添加新的数据对象，新数据对象不能为null，在操作成功后，该方法会返回true |
| offer(e)   | 该方法效果与add(e)方法的效果一致，都是在队列尾部添加新的数据对象，新数据对象不能为null，在操作成功后，该方法会返回true |
| remove()      | 队列头部移除数据对象，在集合中没有数据对象的情况下，会抛出NoSuchElementException异常 |
| poll()      | 从队列头部移除数据对象，在集合中没有数据对象的情况下，不会抛出异常，只会返回null |
| element()      | 试图从队列头部获取数据对象，但不会试图从头部移除这个数据对象。在集合中没有数据对象的情况下，会抛出 NoSuchElementException异常 |
| peek()      | 试图从队列头部获取数据对象，但不会试图从头部移除这个数据对象。在集合中没有数据对象的情况下，不会抛出异常，只会返回null |


**将ArrayDeque集合作为双端队列结构使用**    
ArrayDeque集合实现了java.util.Deque接口    

| 方法名      |    方法意义 |  
| :-------- |:--------| 
| addLast(e) |在双端队列尾部添加新的数据对象，注意插入的新数据对象不能为 null，否则会抛出NullPointerException异常 |
| addFirst(e) |在双端队列头部添加新的数据对象，注意插入的新数据对象不能为null，否则会抛出NullPointerException异常  |
|offerLast(e) |在双端队列尾部添加新的数据对象，注意插入的新数据对象不能为null，否则会抛出NullPointerException 异常。该方法会在操作完成后返回true  |
|offerFirst(e) | 在双端队列头部添加新的数据对象，注意插入的新数据对象不能为 null，否则会抛出NullPointerException异常。该方法会在操作完成后返回true |
|removeFirst() | 从双端队列头部移除数据对象，如果当前双端队列中已没有任何数据对象可移除，则抛出NoSuchElementException异常   |
|removeLast() | 双端队列尾部移除数据对象，如果当前双端队列中已没有任何数据对象可移除，则抛出NoSuchElementException异常 |
|pollFirst() | 从双端队列头部移除数据对象，如果当前双端队列中已没有任何数据对象可移除，则返回null  |
|pollLast() |  从双端队列尾部移除数据对象，如果当前双端队列中已没有任何数据对象可移除，则返回null  |
|getFirst() | 在双端队列头部获取数据对象，但不会移除数据对象，如果当前双端队列中已没有任何数据对象可获取，则抛出NoSuchElementException异常 |
| getLast()| 在双端队列尾部获取数据对象，但不会移除数据对象，如果当前双端队列中已没有任何数据对象可获取，则抛出NoSuchElementException异常 |
|peekFirst() |在双端队列头部获取数据对象，但不会移除数据对象，如果当前双端队列中已没有任何数据对象可获取，则返回null |
|peckLast()| 在双端队列尾部获取数据对象，但不会移除数据对象，如果当前双端队列中已没有任何数据对象可获取，则返回null |


**将ArrayDeque集合作为栈结构使用**    
java.util.Deque接口所代表的双端队列具有栈结构的操作特性。    

| 方法名      |    方法意义 |  
| :-------- |:--------| 
| push(e) |在栈结构头部添加新的数据对象，该数据对象不能为null |
| pop() |从栈结构头部移除数据对象，如果当前栈结构中没有任何数据可移除，则抛出NoSuchElmentException异常  |



refer
1. 《Java高兵法与集合框架》