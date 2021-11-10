## JDK命令行工具
在JDK的开发包中，除了大家熟知的java和javac外，还有一系列辅助工具。这些辅助工具位于JDK安装目录下的bin目录中，可以帮助开发人员很好地解决Java应用程序的一些"疑难杂症"。    
这些工具都是可执行文件，在windows系统的扩展是.exe，但事实上它们只是Java程序的一层包装，其真正的实现是在tools.jar中。


## javac使用
* -d 指定编译地址
```shell
javac -d . xxxx.java #会在当前路径，生成包路径文件
java xx.xxx.xxxx.xxxx #执行代码
```

