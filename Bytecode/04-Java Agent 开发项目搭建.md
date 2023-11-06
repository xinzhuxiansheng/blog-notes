## Java Agent 开发项目搭建  

### 创建 Maven项目  
Idea 创建Maven项目 无需特别说明， 在 agent 开发其入口类 premain() 或 agetnmain()， 还得看 agent 启动方式。  
* jvm 参数形式： 调用 premain 方法      
* attach 方式： 调用 agentmain 方法     

**入口示例：**    
```java
public class SimpleAgent {

    /**
     * jvm 参数形式启动，运行此方法
     *
     * @param agentArgs
     * @param inst
     */
    public static void premain(String agentArgs, Instrumentation inst) {
        System.out.println("premain");
    }

    /**
     * 动态 attach 方式启动，运行此方法
     *
     * @param agentArgs
     * @param inst
     */
    public static void agentmain(String agentArgs, Instrumentation inst) {
        System.out.println("agentmain");
    }
}
```

### MANIFEST打包        
使用 mvn 命令对其项目进行打包，这与一般工程没有什么特殊的，不过需注意 项目工程下的 `MANIFEST.MF`的定义（这是 agent 需要的） ,以下分享 两种方式：    

**方式一** 使用 manifestEntries 参数定义 MANIFEST.MF
```xml
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-assembly-plugin</artifactId>
    <configuration>
        <descriptorRefs>
            <descriptorRef>jar-with-dependencies</descriptorRef>
        </descriptorRefs>
        <archive>
            <manifestEntries>
                <Premain-Class>com.javamain.agent.loadtime.LoadTimeAgent</Premain-Class>
                <Agent-Class>com.javamain.agent.loadtime.LoadTimeAgent</Agent-Class>
                <Can-Redefine-Classes>true</Can-Redefine-Classes>
                <Can-Retransform-Classes>true</Can-Retransform-Classes>
            </manifestEntries>
        </archive>
    </configuration>

    <executions>
        <execution>
            <goals>
                <goal>attached</goal>
            </goals>
            <phase>package</phase>
        </execution>
    </executions>
</plugin>
```

**方式二** 使用 manifestFile 参数定义 MANIFEST.MF 路径文件
```xml
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-assembly-plugin</artifactId>
    <configuration>
        <descriptorRefs>
            <descriptorRef>jar-with-dependencies</descriptorRef>
        </descriptorRefs>
        <archive>
            <manifestFile>
                src/main/resources/META-INF/MANIFEST.MF
            </manifestFile>
        </archive>
    </configuration>

    <executions>
        <execution>
            <goals>
                <goal>attached</goal>
            </goals>
            <phase>package</phase>
        </execution>
    </executions>
</plugin>
```
再通过 `mvn assembly:assembly` 命令打包即可。           

### 编写 Demo 测试 agent   

#### javaagent 方式启动

通过简单的 Demo 测试 Java Agent程序。 代码如下：
**Program.java**
```java
public class Program {
    public static void main(String[] args) throws Exception {
        // 打印 PID 标识
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

测试Idea 启动，并 `VM Options 中添加 -javaagent:path/your/agentjar`


#### attach 方式启动  
在使用 attach 方式时， 可以理解成 我们将 agent 注入到目标应用程序中，所以我们需自己启动一个程序来完成，示例代码如下：   
```java
public class AttachMain {

    public static void main(String[] args)
            throws IOException, AgentLoadException, AgentInitializationException, AttachNotSupportedException {
        // attach方法参数为目标应用程序的进程号
        VirtualMachine vm = VirtualMachine.attach("your app PID");
        // 请用你自己的agent绝对地址，替换这个
        vm.loadAgent("path/your/agentjar");
    }
}
```

测试流程， 先启动应用程序， 使用 ps 查看应用程序的 PID， 将 PID 参数 写入 ArrachMain代码中去，再启动 AttachMain#main()。   
其结果，AttachMain#main() 会很快执行结束，此时，我们查看 应用程序的log中，可看到之前在 agentmain()方法打印的内容：    
```java
public static void agentmain(String agentArgs, Instrumentation inst) {
    System.out.println("agentmain");
}
``` 

**Output**
```
19
20
21
22
agentmain  
23
24
25
26
27
28
```


以上就是该篇 Blog 的全部内容。      

-agentlib:jdwp="transport=dt_socket,server=y,suspend=y,address=*:5005"  