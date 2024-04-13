--In Blog
--Tags: OpenJDK,Java

# Idea 配置OpenJDK(非 Native,JVM)常用调试环境

>Idea 本来就可以调试OpenJDK代码，例如 java.util.* 下面的代码

# 添加代码注释

**1. IDEA显示源码出现'锁'标识**
![Idea搭建OpenJDK源码调试环境01](http://img.xinzhuxiansheng.com/blogimgs/jdk/Idea搭建OpenJDK源码调试环境01.png)


**2. 修改Sourcepath路径**
* 解压OpenJDK目录中的`src.zip`，并重命名`source` （重命名可选）
* 配置`Project Structure`的`SDKs`的 Sourcepath
![Idea搭建OpenJDK源码调试环境-配置Project-Structure](http://img.xinzhuxiansheng.com/blogimgs/jdk/Idea搭建OpenJDK源码调试环境-配置Project-Structure.png)


**3. Idea配置 Do not step into the classes**
* 去掉`Do not step into the classes`
![Idea搭建OpenJDK源码调试环境-配置Project-Structure](http://img.xinzhuxiansheng.com/blogimgs/jdk/Idea搭建OpenJDK源码调试环境-idea-stepping配置.png)

**4. Idea的'锁'标识已经不存在了**