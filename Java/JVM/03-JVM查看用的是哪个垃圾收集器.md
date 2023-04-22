## JVM查看用的是哪个垃圾收集器

在Java 8中，您可以使用以下几种方法来查看正在使用的垃圾收集器：  
**1.** 使用`-XX:+PrintCommandLineFlags`选项启动Java程序。这将在程序启动时打印传递给JVM的命令行参数。例如：  
```shell
java -XX:+UseG1GC -XX:+PrintCommandLineFlags -jar my_application.jar
```
在输出中，您将看到如下行（或类似的行，取决于所使用的垃圾收集器）：  
```shell
-XX:InitialHeapSize=2147483648 -XX:MaxHeapSize=2147483648 -XX:+PrintCommandLineFlags -XX:+UseCompressedClassPointers -XX:+UseCompressedOops -XX:+UseG1GC
``` 
这里，`-XX:+UseG1GC`表明程序使用的是G1垃圾收集器。

**2.** 使用`jstat`工具。首先，找到您的Java进程的ID。然后，运行以下命令：    
```shell
jstat -gc <process_id>
```

在输出中，您可以从各个内存池的名称中推断出使用的垃圾收集器。例如，G1垃圾收集器具有名为`S0C`、`S1C`、`EC`、`OC`等的内存池。  

**3.** 使用`jcmd`工具。首先，找到您的Java进程的ID。然后，运行以下命令：     
```shell
jcmd <process_id> VM.gcRun
```

这将显示有关垃圾收集器的信息。例如，对于G1垃圾收集器，输出可能包括以下行：  
```shell
Garbage-First (G1) garbage collector
```
这些方法中的任何一个都可以帮助您确定正在使用的垃圾收集器。