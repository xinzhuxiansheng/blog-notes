
## 反射

### 场景：如果我们知道 class的全限定名 com.xxxx.xxx ,如何加载，如何通过反射实例化

eg: com.xinzhuxiansheng.Reflect.example.Bootstrap
```java
//class的全限定名
String className="com.xinzhuxiansheng.Reflect.example.Bootstrap";
//类加载器
ClassLoader classLoader  = Thread.currentThread().getContextClassLoader();
if(classLoader == null){
    //当前方法在Test类下
    classLoader = Test.class.getClassLoader();
    if(classLoader == null){
        classLoader = ClassLoader.getSystemClassLoader();
    }
}
Class<?> clazz = Class.forName(className,false,classLoader);
if(!clazz.isInterface()){
    //通过反射来构建对象
    Constructor<?> constructor = cls.getDeclaredConstructor();
    //对所有属性设置访问权限，当类中的成员变量为private时，必须设置此项，即使是final关键字标示过得属性也可以有访问权限
    constructor.setAccessible(true);
    //实例化接口
    InitFunc initFunc = (InitFunc)constructor.newInstance();
}
```

### 利用反射调用private方法