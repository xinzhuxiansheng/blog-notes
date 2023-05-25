## Java System Properties
Java维护了一组对系统属性设置的操作。每个Java系统属性都是一个键值对(k-v,String-String)，例如`“java.version”=”1.7.0_09“`。你可以通过`System.getProperties()`获取到系统的全部属性值,也可以通过'System.getProperty(key)' 检索到单个属性值。

>注意：
系统属性的访问会受到Java安全管理器(security manager)和策略文件(prolicy file)限制,默认情况下，Java程序对所有系统属性都无限制访问。


### 重要的 Java System Properties
1. JRE相关的系统属性值

| key      |    value |
| :-------- | :-------- |
| java.home  | JRE homg目录，例如:` “C:\Program Files\Java\jdk1.7.0_09\jre“` |
| java.library.path     | 用于搜索本地类库的JRE库的搜索路径。它通常不一定取环境变量路径 | 
| java.class.path      |    JRE classpath 例如:`'.'` 表示当前工作目录 |
| java.ext.dirs      |    JRE 扩展类库地址 例如:`“C:\Program Files\Java\jdk1.7.0_09\jre\lib\ext;C:\Windows\Sun\Java\lib\ext“` |
| java.version      |    JDK版本 例如:`1.7.0_09` |
| java.runtime.version      |    JRE 版本 例如:`1.7.0_09-b05` |

2. File相关的系统属性值

| key      |    value |
| :-------- | :-------- |
| file.separator  | 文件目录分隔符的符号。例如:`d:\test\test.java`,Windows默认是 `\`, Unix/Mac默认是`/`|
| path.separator     | 分割路径条目的符号。例如:`在路径或者CLASSPATH`,Windows默认是 `;`, Unix/Mac默认是`：` | 
| line.separator      | 行尾符号或换号符号 Windows默认是 `\r\n`, Unix/Mac默认是`\n`|


3. 用户相关的系统属性值

| key      |    value |
| :-------- | :-------- |
| user.name  | 用户名称|
| user.home     | 用户home路径 | 
| user.dir      | 用户当前工作目录|


4. 系统相关的系统属性值

| key      |    value |
| :-------- | :-------- |
| os.name  | 系统名称 例如:`windows 10`|
| os.version     | 系统版本 例如:`10`| 
| os.arch      | 操作系统架构 例如:`x86`|


### 获取系统属性值
在文中开头提到，你可以通过`System.getProperties()`获取到系统的全部属性值,也可以通过'System.getProperty(key)' 检索到单个属性值
```java
import java.util.Properties;
public class PrintSystemProperties
{
   public static void main(String[] a)
   {
      // List all System properties
      Properties pros = System.getProperties();
      pros.list(System.out);
  
      // Get a particular System property given its key
      // Return the property value or null
      System.out.println(System.getProperty("java.home"));
      System.out.println(System.getProperty("java.library.path"));
      System.out.println(System.getProperty("java.ext.dirs"));
      System.out.println(System.getProperty("java.class.path"));
   }
}
```

### 设置系统属性值
在java里，你可以用命令行工具或 在java代码中设置
1.在命令行中使用 `-D` 参数 指定key
```shell
java -Dcustom_key="custom_value" application_launcher_class
```

2.使用System.setProperty()设置属性值
```java
System.setProperty("custom_key", "custom_value");
```


>翻译原文地址：https://howtodoinjava.com/java/basics/java-system-properties/