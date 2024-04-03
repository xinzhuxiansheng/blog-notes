# JVM 元空间 Metaspace  

>Metaspace 为什么会溢出？     
**Metaspace溢出的两种情况：**       
1.metaspace小了，没有设置或者用了比较小的默认值 
2.使用了 Cglib 这类支持动态生成的方法，此时会大量生成类，如果控制不好就容易导致元空间溢出   


## 元空间如何存储类信息 



## 使用 Cglib 模拟元空间溢出    

### 添加 cglib相关依赖  
```bash
 <dependency>
    <groupId>cglib</groupId>
    <artifactId>cglib</artifactId>
    <version>3.3.0</version>
</dependency>
<dependency>
    <groupId>cglib</groupId>
    <artifactId>cglib-nodep</artifactId>
    <version>3.3.0</version>
</dependency>   
```

### 设置 JVM参数    
```bash
-Xms200M
-Xmx200M
-Xmn150M
-XX:SurvivorRatio=8
-XX:MetaspaceSize=10M
-XX:MaxMetaspaceSize=10M
-XX:+UseConcMarkSweepGC
-XX:+TraceClassLoading
-XX:+TraceClassUnloading
-XX:+PrintGC
-XX:+PrintGCDetails
-XX:+PrintGCDateStamps
```

### Code    
```java
public class CgLibTest {
    public static void main(String[] args) {
        while (true) {
            Enhancer enhancer = new Enhancer();
            enhancer.setSuperclass(ServiceTest.class);
            enhancer.setUseCache(false);
            enhancer.setCallback(new MethodInterceptor() {
                @Override
                public Object intercept(Object o, Method method, Object[] objects, MethodProxy methodProxy) throws Throwable {
                    return methodProxy.invokeSuper(o, objects);
                }
            });
            enhancer.create();
        }
    }
    static class ServiceTest {
    }
}
```
 
output log: 
```bash 
 Metaspace       used 9499K, capacity 10102K, committed 10240K, reserved 1058816K
  class space    used 800K, capacity 809K, committed 896K, reserved 1048576K
	at sun.reflect.GeneratedMethodAccessor1.invoke(Unknown Source)
	at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
	at java.lang.reflect.Method.invoke(Method.java:498)
	at net.sf.cglib.core.ReflectUtils.defineClass(ReflectUtils.java:459)
	at net.sf.cglib.core.AbstractClassGenerator.generate(AbstractClassGenerator.java:339)
	... 6 more
Caused by: java.lang.OutOfMemoryError: Metaspace
	at java.lang.ClassLoader.defineClass1(Native Method)
	at java.lang.ClassLoader.defineClass(ClassLoader.java:756)
	... 11 more
``` 


