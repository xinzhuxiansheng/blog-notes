## 管理你的config

> 这里是讨论一些Config的处理，看到StreamX项目中用scala编写的ConfiHolder和ConfigOptions等处理config，通过register()，convert()等 进行校验，注册，从*.properties设值，再读取。

文中简单描述了Spring Boot的Config处理方式，这样也好理解StreamX的舒服的处理。

config的设置内容需要包含以下内容:   
* key : 
* default.value
* describe
* type eg: String,Integer (可选)
* intput.format (可选)

### Spring Boot config value的获取方式  
Spring Boot是Java语言中常用的Web框架，项目中resources文件夹下的*.yaml或者*.properties是用来设置 config value。  Spring Boot提供了多个注解方式来帮助开发者简化config value的获取成本，它内部会自动扫描并读取 `application-{profile}.properties and YAML variants` 的属性值并会覆盖`缺省值`。（Spring Boot 2.6.7 doc: https://docs.spring.io/spring-boot/docs/2.6.7/reference/htmlsingle/），

#### 1. @Value
举一个具体的示例 ，假设服务需要连接Mysql的config， 开发创建了`MysqlConfig`，添加了`@Component`注解，它有 url、username、password 三个属性以及 initConn()方法，仅仅这些，config就足以工作了。
```java
@Component
public class MysqlConfig {

    @Value("{mysql.url}")
    public String url;
    @Value("{mysql.username}")
    public String username;
    @Value("mysql.password")
    public String password;

    public Connection initConn() throws SQLException {
        return DriverManager.getConnection(url, username, password);
    }
}
```

>你只需要在 resources 文件夹下的application-{profile}.properties 添加k=v形式内容，并且bean要添加注解，将其注入在Spring管理的生命周期内。 

例如:
```
#mysql
mysql.url=jdbc:mysql://localhost:3306/db_test?useUnicode=true&characterEncoding=utf8&allowMultiQueries=true&useSSL=false&serverTimezone=Asia/Shanghai 
mysql.username=root
mysql.password=123456
```

根据官网的描述，也可以使用`java -jar app.jar --mysql.url="xxxxxxx"`形式来赋值   
```
On your application classpath (for example, inside your jar) you can have an application.properties file that provides a sensible default property value for name. When running in a new environment, an application.properties file can be provided outside of your jar that overrides the name. For one-off testing, you can launch with a specific command line switch (for example, java -jar app.jar --name="Spring").
```

**默认值**  
支持在未定义的k=v的属性提供默认值，形式如下:   ${ ... : ...}
```java
@Component
public class MysqlConfig {
    // ...省略其他部分

    @Value("${mysql.password : default-pwd-value }")
    public String password;
}
```


refer to https://docs.spring.io/spring-boot/docs/2.6.7/reference/htmlsingle/#features.external-config


#### 2. @ConfigurationProperties
Spring Boot提供 @ConfigurationProperties 可以快速绑定 *.properties与bean的属性关系，在字段较多情况下，使用@Value逐一配置，增加了配置成本。所以在通常我们会将
拿@Value的示例改造后, 这里有个比较不易发现的区别，`AutoMysqlConfig`添加了lombok @Data注解(也可以添加get、set方法)，否则属性值都是null。
```java
@Component
@ConfigurationProperties(prefix = "mysql2")
@Data
public class AutoMysqlConfig {

    public String url;
    public String username;
    public String password;
    
    public Connection initConn() throws SQLException {
        return DriverManager.getConnection(url, username, password);
    }
}
```


#### 实现Config管理

> 参考StreamX开源的代码(https://github.com/streamxhub/streamx),  tiny版本 https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-springboot/config-manager 同样可以查看

实现流程    
![01configmanager01](http://img.xinzhuxiansheng.com/blogimgs/springboot/01configmanager01.jpg)

**1.** 首先创建EnvInitializer 实现ApplicationRunner，在项目刚运行时，进行Config处理

**2.** 调用EnvInitializer.initInternalConfig()， 利用Spring Boot的Environment来获取*.yaml或者 application-{profile}.properties的内容集，将与InternalConfigHolder里已注册的config keys，进行遍历判断，配置文件中存在的key，才会对InternalConfigHolder的config key赋值

>配置文件中的config集合 与 InternalConfigHolder的config集合做containsProperty()过滤，那 InternalConfigHolder的config的key 如何存在集合中？  

**3.** 以K8sFlinkConfig为例，它是scala object，它里面有多个InternalOption类型的属性，看case class InternalOption的主构造函数中调用了 InternalConfigHolder.register(this)。 会将config信息存放在集合中 confOptions，confData。

>K8sFlinkConfig类又是如何通知整个服务，启动后，将我主动构造出来，不然InternalConfigHolder 怎么会知道呢？

**4.** 查看InternalConfigHolder类，它有这样一段code
{
	Seq(K8sFlinkConfig)
}

所以，当在EnvInitializer的run() 调用InternalConfigHolder，首先会执行它的“静态块”。  这里我先简称下，博主scala技术有限，这里涉及scala object的反编译和执行顺序，可以先记住{ Seq ....} 先执行。



### 总结(仅个人观点)
StreamX的ConfigHolder实现不复杂，用register()来注册config 等。 对于config较多情况，仅一次处理，后面无需再添加注解等。 省事。

**1.** 对于我来说是 ConfigHolder.convert() 用scala编码，这太丝滑了
**2.** case class（样例类）构造函数的使用
**3.** InternalConfigHolder，InternalOption 是scala的object类，在学习scala时候，知道它类似于Java 的static。 通过这次学习，发现两者差的太多。


