# Flink 源码 - Standalone - 代码随笔集  


1. getClass()    
使用 getClass() 创建 log 
```java
public abstract class RestServerEndpoint implements RestService {

    protected final Logger log = LoggerFactory.getLogger(getClass());

    private final Object lock = new Object();
```

2.先决条件判断 
Preconditions.java  
```java
   public final void start() throws Exception {
        synchronized (lock) {
            Preconditions.checkState(
                    state == State.CREATED, "The RestServerEndpoint cannot be restarted.");

```