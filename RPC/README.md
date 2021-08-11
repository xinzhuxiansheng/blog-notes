
## 1. guide-rpc-framework
https://github.com/Snailclimb/guide-rpc-framework

### 1.1 服务注册
* Server注册的命名规范:
```java
// Zookeeper节点的CreateMode.PERSISTENT
// eg: /my-rpc/github.javaguide.HelloService/127.0.0.1:9999
String servicePath = CuratorUtils.ZK_REGISTER_ROOT_PATH + "/" + rpcServiceName + inetSocketAddress.toString();
CuratorFramework zkClient = CuratorUtils.getZkClient();
CuratorUtils.createPersistentNode(zkClient, servicePath);

// rpcServiceName
serviceRegistry.registerService(rpcServiceConfig.getRpcServiceName(), new InetSocketAddress(host, NettyRpcServer.PORT));

// addShutdownHook 清楚Zookeeper数据
Runtime.getRuntime().addShutdownHook(new Thread(() -> {
    try {
        InetSocketAddress inetSocketAddress = new InetSocketAddress(InetAddress.getLocalHost().getHostAddress(), NettyRpcServer.PORT);
        CuratorUtils.clearRegistry(CuratorUtils.getZkClient(), inetSocketAddress);
    } catch (UnknownHostException ignored) {
    }
    ThreadPoolFactoryUtils.shutDownAllThreadPool();
}));
```


### 1.2 引用注释，自动装配服务实现类

`RpcReference`  
```java
/**
 * RPC reference annotation, autowire the service implementation class
 *
 * @author smile2coder
 * @createTime 2020年09月16日 21:42:00
 */
@Documented
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.FIELD})
@Inherited
public @interface RpcReference {

    /**
     * Service version, default value is empty string
     */
    String version() default "";

    /**
     * Service group, default value is empty string
     */
    String group() default "";

}
```

`SpringBeanPostProcessor implements BeanPostProcessor`  





### 1.3 SPI机制

`ExtensionLoader`  
```java
private final RpcRequestTransport rpcClient;
this.rpcClient = ExtensionLoader.getExtensionLoader(RpcRequestTransport.class).getExtension("netty");

// 利用本地变量缓存，保证只加载一次
private Map<String, Class<?>> getExtensionClasses() {
    // get the loaded extension class from the cache
    Map<String, Class<?>> classes = cachedClasses.get();
    // double check
    if (classes == null) {
        synchronized (cachedClasses) {
            classes = cachedClasses.get();
            if (classes == null) {
                classes = new HashMap<>();
                // load all extensions from our extensions directory
                loadDirectory(classes);
                cachedClasses.set(classes);
            }
        }
    }
    return classes;
}

// 加载 META-INF/extensions/
private void loadDirectory(Map<String, Class<?>> extensionClasses) {
    String fileName = ExtensionLoader.SERVICE_DIRECTORY + type.getName();
    try {
        Enumeration<URL> urls;
        ClassLoader classLoader = ExtensionLoader.class.getClassLoader();
        urls = classLoader.getResources(fileName);
        if (urls != null) {
            while (urls.hasMoreElements()) {
                URL resourceUrl = urls.nextElement();
                loadResource(extensionClasses, classLoader, resourceUrl);
            }
        }
    } catch (IOException e) {
        log.error(e.getMessage());
    }
}
// 获取当前类的classloader，并存入缓存中
private void loadResource(Map<String, Class<?>> extensionClasses, ClassLoader classLoader, URL resourceUrl) {
    try (BufferedReader reader = new BufferedReader(new InputStreamReader(resourceUrl.openStream(), UTF_8))) {
        String line;
        // read every line
        while ((line = reader.readLine()) != null) {
            // get index of comment
            final int ci = line.indexOf('#');
            if (ci >= 0) {
                // string after # is comment so we ignore it
                line = line.substring(0, ci);
            }
            line = line.trim();
            if (line.length() > 0) {
                try {
                    final int ei = line.indexOf('=');
                    String name = line.substring(0, ei).trim();
                    String clazzName = line.substring(ei + 1).trim();
                    // our SPI use key-value pair so both of them must not be empty
                    if (name.length() > 0 && clazzName.length() > 0) {
                        Class<?> clazz = classLoader.loadClass(clazzName);
                        extensionClasses.put(name, clazz);
                    }
                } catch (ClassNotFoundException e) {
                    log.error(e.getMessage());
                }
            }

        }
    } catch (IOException e) {
        log.error(e.getMessage());
    }
}

```

### 1.4 Netty心跳机制