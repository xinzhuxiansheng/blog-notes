# Maven - Maven 支持多 JDK 版本打包程序    

## 背景          
在学习和工作中开发的 Java 项目，它们集成了一些框架和类库相关使用。在使用的过程，我经常会在自己学习的项目中来验证框架或者类库的调用是否满足开发需求，例如我下面的两个测试项目：      
* ``


```bash
[ERROR] Failed to execute goal org.apache.maven.plugins:maven-compiler-plugin:3.8.0:compile (default-compile) on project flink-cdc: Fatal error compiling: invalid target release: 11 -> [Help 1]
[ERROR] 
[ERROR] To see the full stack trace of the errors, re-run Maven with the -e switch.
[ERROR] Re-run Maven using the -X switch to enable full debug logging.
```