## Agent Load-Time 打印方法接受的参数           

### 预期目标    
借助于 JDK 内置的 ASM 打印出方法接收的参数，使用 Load-Time Instrumentation 的方式实现  


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
public class ASMTransformer implements ClassFileTransformer {
    @Override
    public byte[] transform(ClassLoader loader,
                            String className,
                            Class<?> classBeingRedefined,
                            ProtectionDomain protectionDomain,
                            byte[] classfileBuffer) {
        if (className == null) return null;
        if (className.startsWith("java")) return null;
        if (className.startsWith("javax")) return null;
        if (className.startsWith("jdk")) return null;
        if (className.startsWith("sun")) return null;
        if (className.startsWith("org")) return null;
        if (className.startsWith("com")) return null;
        //if (className.startsWith("javamain")) return null;

        System.out.println("candidate className: " + className);

        if (className.equals("yzhou/javamain/agent/printpid/HelloWorld")) {
            ClassReader cr = new ClassReader(classfileBuffer);
            ClassWriter cw = new ClassWriter(ClassWriter.COMPUTE_MAXS);
            ClassVisitor cv = new MethodInfoVisitor(cw);

            int parsingOptions = 0;
            cr.accept(cv, parsingOptions);

            return cw.toByteArray();
        }

        return null;
    }
}
```

编写测试程序 ：  
** Program.java **  
```java
public class Program {
    public static void main(String[] args) throws Exception {
        // (1) print process id
        String nameOfRunningVM = ManagementFactory.getRuntimeMXBean().getName();
        System.out.println(nameOfRunningVM);

        // (2) count down
        int count = 600;
        for (int i = 0; i < count; i++) {
            System.out.println(HelloWorld.add(i, i + 1));
            TimeUnit.SECONDS.sleep(1);
        }
    }
}
```

**HelloWorld**
```java
public class HelloWorld {
    public static int add(int a, int b) {
        return a + b;
    }

    public static int sub(int a, int b) {
        return a - b;
    }
}
```

#### ASM 相关
在这个部分，我们要借助于 JDK 内置的 ASM 类库（jdk.internal.org.objectweb.asm），来实现打印方法参数的功能。      
在 ParameterUtils.java 文件当中，主要是定义了各种类型的 print 方法：      
**ParameterUtils.java**  
```java
public class ParameterUtils {
    private static final DateFormat fm = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    public static void printValueOnStack(boolean value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(byte value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(char value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(short value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(int value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(float value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(long value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(double value) {
        System.out.println("    " + value);
    }

    public static void printValueOnStack(Object value) {
        if (value == null) {
            System.out.println("    " + value);
        }
        else if (value instanceof String) {
            System.out.println("    " + value);
        }
        else if (value instanceof Date) {
            System.out.println("    " + fm.format(value));
        }
        else if (value instanceof char[]) {
            System.out.println("    " + Arrays.toString((char[]) value));
        }
        else if (value instanceof Object[]) {
            System.out.println("    " + Arrays.toString((Object[]) value));
        }
        else {
            System.out.println("    " + value.getClass() + ": " + value.toString());
        }
    }

    public static void printText(String str) {
        System.out.println(str);
    }

    public static void printStackTrace() {
        Exception ex = new Exception();
        ex.printStackTrace(System.out);
    }
}
``` 

**Const.java**
在 Const.java 文件当中，主要是定义了 ASM_VERSION 常量，它标识了使用的 ASM 的版本：  
```java
public class Const {
    public static final int ASM_VERSION = Opcodes.ASM5;
}
``` 

**MethodInfoAdapter.java**  
```java
public class MethodInfoAdapter extends MethodVisitor {
    private final String owner;
    private final int methodAccess;
    private final String methodName;
    private final String methodDesc;

    public MethodInfoAdapter(MethodVisitor methodVisitor, String owner,
                             int methodAccess, String methodName, String methodDesc) {
        super(Const.ASM_VERSION, methodVisitor);
        this.owner = owner;
        this.methodAccess = methodAccess;
        this.methodName = methodName;
        this.methodDesc = methodDesc;
    }

    @Override
    public void visitCode() {
        if (mv != null) {
            String line = String.format("Method Enter: %s.%s:%s", owner, methodName, methodDesc);
            printMessage(line);

            int slotIndex = (methodAccess & Opcodes.ACC_STATIC) != 0 ? 0 : 1;
            Type methodType = Type.getMethodType(methodDesc);
            Type[] argumentTypes = methodType.getArgumentTypes();
            for (Type t : argumentTypes) {
                int sort = t.getSort();
                int size = t.getSize();
                int opcode = t.getOpcode(Opcodes.ILOAD);
                super.visitVarInsn(opcode, slotIndex);

                if (sort >= Type.BOOLEAN && sort <= Type.DOUBLE) {
                    String desc = t.getDescriptor();
                    printValueOnStack("(" + desc + ")V");
                }
                else {
                    printValueOnStack("(Ljava/lang/Object;)V");
                }
                slotIndex += size;
            }
        }

        super.visitCode();
    }

    private void printMessage(String str) {
        super.visitLdcInsn(str);
        super.visitMethodInsn(Opcodes.INVOKESTATIC, "yzhou/javamain/agent/utils/ParameterUtils", "printText", "(Ljava/lang/String;)V", false);
    }

    private void printValueOnStack(String descriptor) {
        super.visitMethodInsn(Opcodes.INVOKESTATIC, "yzhou/javamain/agent/utils/ParameterUtils", "printValueOnStack", descriptor, false);
    }

    private void printStackTrace() {
        super.visitMethodInsn(Opcodes.INVOKESTATIC, "yzhou/javamain/agent/utils/ParameterUtils", "printStackTrace", "()V", false);
    }
}
```

**MethodInfoVisitor.java**    
```java
public class MethodInfoVisitor extends ClassVisitor {
    private String owner;

    public MethodInfoVisitor(ClassVisitor classVisitor) {
        super(Const.ASM_VERSION, classVisitor);
    }

    @Override
    public void visit(int version, int access, String name, String signature, String superName, String[] interfaces) {
        super.visit(version, access, name, signature, superName, interfaces);
        this.owner = name;
    }

    @Override
    public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
        MethodVisitor mv = super.visitMethod(access, name, descriptor, signature, exceptions);
        if (mv != null && !name.equals("<init>") && !name.equals("<clinit>")) {
            mv = new MethodInfoAdapter(mv, owner, access, name, descriptor);
        }
        return mv;
    }
}
```


>注意： 该篇 Blog的目的是 借助于 JDK 内置的 ASM 打印出方法接收的参数，使用 Load-Time Instrumentation 的方式实现     
Program 调用了 HelloWorld 的静态方法, 这里涉及到一些类的限定名的判断， 例如：   
1.LoadTimeAgent.java 只针对 HelloWorld的方法 进行打印参数，所以需注意 `if (className.equals("yzhou/javamain/agent/printpid/HelloWorld")) `     
2.MethodInfoAdapter#visitCode()方法中的 `printMessage(line);` 会调用 `ParameterUtils`方法 , 所以 需要注意  printMessage中 ParameterUtils 的限定名    

再 使用 `mvn assembly:assembly` 命令，对agent 进行打包。 在 Idea 中 配置 `Program` 启动项，在 `VM Options` 参数中添加 -javaagent:path/your/agentjar 即可。    

那么，Agent.jar 到底做了一件什么事情呢？                     
在一般情况下，我们先编写 HelloWorld.java 文件，然后编译生成 HelloWorld.class 文件，最后加载到 JVM 当中运行。            
当 Instrumentation 发生的时候，它是将原有的 HelloWorld.class 的内容进行修改（bytecode transformation），生成一个新的 HelloWorld.class，最后将这个新的 HelloWorld.class 加载到 JVM 当中运行。                

refer   
1.https://lsieun.github.io/java-agent/s01ch01/quick-start-example-02.html             