## Dynamic Agent 打印方法接受的参数  

### 预期目标  
借助于 JDK 内置的 ASM 打印出方法接收的参数，使用 Dynamic Instrumentation 的方式实现。      

在之前的 Blog “Java Agent 开发项目搭建” 讲解过 attach 方式启动。  

所以在基于 Blog “Agent Load-Time 打印方法接受的参数”中的代码，大部分无需改动，下面我们重点讲解需调整的部分


### 1.MANIFEST.MF 修改    
```
Premain-Class: yzhou.javamain.agent.loadtime02.LoadTimeAgent
Agent-Class: yzhou.javamain.agent.attach01.LoadTimeAgent
Can-Retransform-Classes: true
```

### 2.LoadTimeAgent类添加 agentmain()
```java
public static void agentmain(String agentArgs, Instrumentation inst) {
    ClassFileTransformer transformer = new ASMTransformer();
    try {
        inst.addTransformer(transformer, true);
        Class<?> targetClass = Class.forName("yzhou.javamain.agent.printpid.HelloWorld");
        inst.retransformClasses(targetClass);
    } catch (Exception ex) {
        ex.printStackTrace();
    }
    finally {
        inst.removeTransformer(transformer);
    }
}
```

### 3.编写 attach 测试入口 


我们仅添加以下测试代码：            

```java
public class VMAttach {
    public static void main(String[] args)
            throws IOException, AgentLoadException, AgentInitializationException, AttachNotSupportedException {
        // attach方法参数为目标应用程序的进程号
        VirtualMachine vm = VirtualMachine.attach("25667");
        // 请用你自己的agent绝对地址，替换这个
        vm.loadAgent("/Users/a/Code/Java/javamain-services/javamain-javaagent/target/javamain-javaagent-1.0-SNAPSHOT-jar-with-dependencies.jar");
    }
}
```

### 4.启动流程  
1.启动 Program#main()           
2.将 step1 应用程序的 PID ，配置到 VMAttach#main() 中，启动 VMAtttach#main()。      

refer  
1.https://lsieun.github.io/java-agent/s01ch01/quick-start-example-03.html   