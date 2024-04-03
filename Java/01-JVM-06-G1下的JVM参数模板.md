# G1下的JVM参数模板 

## G1 参数模板  
-Xmx4096m -Xms2048m -Xmn1024m -Xloggc:/home/shared/log/gc-server.log    
-XX:ParallelGCThreads=2 -XX:+UseG1GC    
-XX:MetaspaceSize=64M -XX:MaxMetaspaceSize=128m
-XX:+UseFastAccessorMethods -XX:+PrintGCDetails
-XX:+PrintGCApplicationStoppedTime -XX:+PrintGCDateStamps -XX:+PrintHeapAtGC
-XX:+UseGCLogFileRotation -XX:NumberOfGCLogFiles=10 -XX:GCLogFileSize=50M
-XX:HeapDumpOnOutOfMemoryError -XX:HeapDumpPath=/usr/local/oom  


## 如何优化YGC  

* 停顿时间参数 -XX:MaxGCPauseMillis 的设置  

* 如果在对象比较大的场景，可以通过下面的参数，手动将 Region调大，这样可以避免短周期大对象进入老年代 
-XX:G1HeapRegionSize=4M

