**`正文`**

[TOC]

## JarURLConnection
```java
public abstract class JarURLConnection extends URLConnection{
    ...
}
```

### syntax
URL Connection是一种连接到jar文件或jar文件中的一个入口连接定位
JAR URL语法：
```java
jar:<url>!/{entry}
```

#### for example
```java
jar:http://www.foo.com/bar/baz.jar!/COM/foo/Quux.class
```
URLs应该用于引用jar文件或jar文件中的条目。上面的列子是一个JAR URL，它引用了一个JAR文件的条目。 如果去掉条目名称，则URL引用是整个jar文件 jar:http://www.foo.com/bar/baz.jar!/
当用创建一个JAR URL，需要调用jar文件中某个特定的方法： for example:
```java
 URL url = new URL("jar:file:/home/duke/duke.jar!/");
 JarURLConnection jarConnection = (JarURLConnection)url.openConnection();
 Manifest manifest = jarConnection.getManifest();
```
JarURLConnection实例只能用于读取jar文件，无法通过`OutputStream`修改或者写入jar文件的条目

### URL规则:
```java
// A Jar entry
jar:http://www.foo.com/bar/baz.jar!/COM/foo/Quux.class

// A Jar file
jar:http://www.foo.com/bar/baz.jar!/

// A Jar directory
jar:http://www.foo.com/bar/baz.jar!/COM/foo/
```
`!/`  被称为分隔符

>通过URL 构建JAR URL时，请参考以下规则：

1.如果没有上下文URL，并且传递给URL构造函数的规范不包含分隔符，则认为该URL引用了JAR文件  
2.如果存在上下文URL，则假定上下文URL引用JAR文件或目录  
3.如果规范以"/"开头，则忽略JAR目录，并且规范被认为是JAR文件的根目录  

```java
    Examples:

    context: jar:http://www.foo.com/bar/jar.jar!/, spec:baz/entry.txt
        url:jar:http://www.foo.com/bar/jar.jar!/baz/entry.txt 
    context: jar:http://www.foo.com/bar/jar.jar!/baz, spec:entry.txt
        url:jar:http://www.foo.com/bar/jar.jar!/baz/entry.txt 
    context: jar:http://www.foo.com/bar/jar.jar!/baz, spec:/entry.txt
        url:jar:http://www.foo.com/bar/jar.jar!/entry.txt 
```



参考地址：https://docs.oracle.com/javase/7/docs/api/java/net/JarURLConnection.html
