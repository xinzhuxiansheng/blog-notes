# Nested Loop Join (NLJ)    

## 介绍    
在 DB 存储中，使用Nested-Loop Join的算法思想去优化join，`Nested-Loop Join` 翻译成中文则是“嵌套循环连接”。     
举个例子：
select * from t1 inner join t2 on t1.id=t2.tid
（1）t1称为外层表，也可称为驱动表。 
（2）t2称为内层表，也可称为被驱动表。
   
NLJ是通过两层循环，用第一张表做Outter Loop，第二张表做Inner Loop，Outter Loop的每一条记录跟Inner Loop的记录作比较，符合条件的就输出。

在 DB 的实现中，Nested-Loop Join有3种实现的算法：
* Simple Nested-Loop Join：SNLJ，简单嵌套循环连接           
* Index Nested-Loop Join：INLJ，索引嵌套循环连接            
* Block Nested-Loop Join：BNLJ，缓存块嵌套循环连接      

在选择Join算法时，会有优先级，理论上会优先判断能否使用INLJ、BNLJ：
Index Nested-LoopJoin > Block Nested-Loop Join > Simple Nested-Loop Join  

### Simple Nested Loop Join（SNLJ） 

```java
// 伪代码
for (r in R) {
    for (s in S) {
        if (r satisfy condition s) {
            output <r, s>;
        }
    }
}
``` 
SNLJ就是两层循环全量扫描连接的两张表，得到符合条件的两条记录则输出，这也就是让两张表做笛卡尔积，比较次数是R * S，是比较暴力的算法，会比较耗时                

### Index Nested Loop Join（INLJ）
```java
// 伪代码
for (r in R) {
    for (si in SIndex) {
        if (r satisfy condition si) {
            output <r, s>;
        }
    }
}
```





refer   
1.https://blog.csdn.net/qq_42000661/article/details/108578997   

