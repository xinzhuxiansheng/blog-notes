## Regular Join(普通Join)  

### 介绍    
* 普通 Join 其实就是常规的双流 Join，通过关联条件去关联两条实时数据流,对应的其实就是Flink SOL中的两个动态表的Join   

* 普通 Join 支特常规的 Inner Join 和 Outer Join。 Outer Join 对应的就是 Left Join, Right Join 和 Full Join。        

### Inner Join 
`Inner Join在SQL语句中可以简写为Join, 在双流Join时，只有左右两边数据流中的数据都关联上了,才会输出结果 +[L,R], 结果中会包含这两条流中的数据。`      

#### 执行过程   
Inner Join 的执行流程,看下面示例图，这里面有三个流表或者说是动态表，表A和表B属于两个业务表，

### Out Join    
* Left Join(Left Outer Join) 它属于简写形式，完整写法是Left Outer Join      

在双流 Join 时,只要左边数据流中的数据到达了。`无论是否关联到右边数据流中的数据都会输出结果`。
1.如果关联到右边数据流中的数据了,则输出完整的结果 +[L,R]。      
2.如果没有关联到右边数据流中的数据, 则会使用 null 进行补全, 然后输出 +[L,null] 。        
3.当右边数据流中的数据到达之后，也会到左边的数据流中进行关联 , 如果发现左边数据流之前输出过没有关联到的数据则会产生回徹流,将之前使用 null 补全的数据回撤掉 -[L,null] ,最后再重新输出关联后的数据 +[L,R]         

* Right Join(Right Outer Join)  它属于简写形式，完整写法是Right Outer Join  

在双流Join时, 他的执行逻辑和Left Join 正好相反。 

* Full Join(Full Outer Join) 它属于简写形式，完整写法是Full Quter Join。        

双流Join时。左边或者右边数据流中数据到达了之后,无论是否关联到另一侧数据流中的数据，都会输出结果。 针对左边的数据流来说, 如果关联到了右边数据流中的数据，则输出 +[L,R] , 如果没有关联到。则输出 +[L,null]，针对右边的数据流来说，如果关联到了左边数据流中的数据, 则输出 +[L,R], 如果没有关联到，则输出 +[null,R], 针对这些没有关联的数据, 后期当数据到达之后，会产生回撤流, 将之前包含 null 的数据回撤掉, 再补全输出完整结果。   

































