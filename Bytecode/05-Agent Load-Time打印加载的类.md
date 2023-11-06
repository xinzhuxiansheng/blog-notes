## Agent Load-Time 打印加载的类       

### 预期目标    
打印正在加载的类    


### 开发 Agent Jar 

#### Agent Jar 项目搭建 
请参考上一篇 Blog “Java Agent 开发项目搭建”，完成工程搭建。         

#### 开发流程 
**LoadTimeAgent 定义 agent 入口方法** 
```java
public class LoadTimeAgent {

    public static void premain(String agentArgs, Instrumentation inst) {
        System.out.println("Premain-Class: " + LoadTimeAgent.class.getName());
        ClassFileTransformer transformer = new InfoTransformer();
        inst.addTransformer(transformer);
    }
}
``` 

**在 ClassFileTransformer 转换过程中 打印 class 信息**
```java
public class InfoTransformer implements ClassFileTransformer {
    @Override
    public byte[] transform(ClassLoader loader,
                            String className,
                            Class<?> classBeingRedefined,
                            ProtectionDomain protectionDomain,
                            byte[] classfileBuffer) {
        StringBuilder sb = new StringBuilder();
        Formatter fm = new Formatter(sb);
        fm.format("ClassName: %s%n", className);
        fm.format("    ClassLoader: %s%n", loader);
        fm.format("    ClassBeingRedefined: %s%n", classBeingRedefined);
        fm.format("    ProtectionDomain: %s%n", protectionDomain);
        System.out.println(sb.toString());

        return null;
    }
}
```

**编写 测试程序 ** 
```java
public class Program {
    public static void main(String[] args) throws Exception {
        // (1) print process id
        String nameOfRunningVM = ManagementFactory.getRuntimeMXBean().getName();
        System.out.println(nameOfRunningVM);

        // (2) count down
        int count = 600;
        for (int i = 0; i < count; i++) {
            System.out.println(i);
            TimeUnit.SECONDS.sleep(1);
        }
    }
}
```

使用 `mvn assembly:assembly` 命令，对agent 进行打包。 在 Idea 中 配置 `Program` 启动项，在 `VM Options` 参数中添加 -javaagent:path/your/agentjar。 

**Output**  
```
Premain-Class: com.javamain.agent.loadtime.LoadTimeAgent
ClassName: com/intellij/rt/debugger/agent/DebuggerAgent
    ClassLoader: null
    ClassBeingRedefined: null
    ProtectionDomain: null

ClassName: com/intellij/rt/debugger/agent/CaptureAgent
    ClassLoader: null
    ClassBeingRedefined: null
    ProtectionDomain: null

ClassName: com/intellij/rt/debugger/agent/CaptureAgent$ParamKeyProvider
    ClassLoader: null
    ClassBeingRedefined: null
    ProtectionDomain: null

ClassName: com/intellij/rt/debugger/agent/CaptureAgent$KeyProvider
    ClassLoader: null
    ClassBeingRedefined: null
    ProtectionDomain: null

......

```

另外，在使用 java 命令时，可以添加 `-verbose:class` 选项，它可以显示每个已加载类的信息。     
```
[0.006s][info][class,load] opened: /Library/Java/JavaVirtualMachines/jdk-11.jdk/Contents/Home/lib/modules
[0.006s][info][class,load] opened: /Users/a/Library/Caches/JetBrains/IntelliJIdea2023.1/captureAgent/debugger-agent.jar
[0.009s][info][class,load] java.lang.Object source: jrt:/java.base
[0.009s][info][class,load] java.io.Serializable source: jrt:/java.base
[0.009s][info][class,load] java.lang.Comparable source: jrt:/java.base
[0.009s][info][class,load] java.lang.CharSequence source: jrt:/java.base
[0.009s][info][class,load] java.lang.String source: jrt:/java.base
[0.009s][info][class,load] java.lang.reflect.AnnotatedElement source: jrt:/java.base
[0.009s][info][class,load] java.lang.reflect.GenericDeclaration source: jrt:/java.base
[0.009s][info][class,load] java.lang.reflect.Type source: jrt:/java.base
[0.009s][info][class,load] java.lang.Class source: jrt:/java.base
[0.009s][info][class,load] java.lang.Cloneable source: jrt:/java.base
[0.009s][info][class,load] java.lang.ClassLoader source: jrt:/java.base

......

```

refer   
1.https://lsieun.github.io/java-agent/s01ch01/quick-start-example-01.html      