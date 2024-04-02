# JVM 元空间 Metaspace  

>Metaspace 为什么会溢出？     
**Metaspace溢出的两种情况：**       
1.metaspace小了，没有设置或者用了比较小的默认值 
2.使用了 Cglib 这类支持动态生成的方法，此时会大量生成类，如果控制不好就容易导致元空间溢出   


## 


