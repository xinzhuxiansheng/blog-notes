## Chat2DB 关于MySQL查询过程讲解 - 元数据信息

>Chat2DB version: 1.0.9 

### 引言    
在上一篇《Chat2DB关于MySQL查询过程讲解 - 连接信息》中讲解了MySQL数据源配置信息在Chat2DB的H2Database的存储过程，当“连接”MySQL集群后，能看到以下图片内容：    
![metadataInfo01](http://img.xinzhuxiansheng.com/blogimgs/chat2db/metadataInfo01.png)    

`能看到以下MySQL元数据信息`: databases,tables,columns,keys,indexs。     

本篇主要来讲解以上涉及MySQL元数据信息的查询过程。   

### 构建查询流程
![metadataInfo02](http://img.xinzhuxiansheng.com/blogimgs/chat2db/metadataInfo02.png)    

`Rest API`      
```shell
http://127.0.0.1:10821/api/connection/datasource/connect?id=3 # id=3仅表示MySQL连接信息的存储Id

# GET请求
```     

**DataSourceController#attach()**
```java
@GetMapping("/datasource/connect")
public ListResult<DatabaseVO> attach(@Valid @NotNull DataSourceAttachRequest request) {
    ListResult<Database> databaseDTOListResult = dataSourceService.connect(request.getId());
    List<DatabaseVO> databaseVOS = dataSourceWebConverter.databaseDto2vo(databaseDTOListResult.getData());
    return ListResult.of(databaseVOS);
}
``` 

**DataSourceServiceImpl#connect()** 
```java
@Override
public ListResult<Database> connect(Long id) {
    DatabaseQueryAllParam queryAllParam = new DatabaseQueryAllParam();
    queryAllParam.setDataSourceId(id);
    List<String> databases = DbhubContext.getMetaSchema().databases();
    return ListResult.of(EasyCollectionUtils.toList(databases, name -> Database.builder().name(name).build()));
}
```
从以上代码了解到，databases数据查询都在`DbhubContext.getMetaSchema().databases()`返回。而`DbhubContext.CONNECT_INFO_THREAD_LOCAL`作为存放MySQL集群连接信息的容器，所以在get之前应尽可能已经放入对象了。所以到这一步我们将关注点放入了CONNECT_INFO_THREAD_LOCAL何时执行put()?      
```java
private static final ThreadLocal<ConnectInfo> CONNECT_INFO_THREAD_LOCAL = new ThreadLocal<>();  
```   

`DataSourceController`    
```java
@BusinessExceptionAspect
@ConnectionInfoAspect
@RequestMapping("/api/connection")
@RestController
@Slf4j
public class DataSourceController {

    @Autowired
    private DataSourceService dataSourceService;
```

Chat2DB在Controller层添加`@ConnectionInfoAspect`注解，并标记AOP执行周期为`环绕通知`，当请求执行到达Controller层时先执行切点方法，根据参数id=3，查询MySQL的连接信息，再拼装成ConnectInfo对象放入`CONNECT_INFO_THREAD_LOCAL`容器中。

**AOP执行顺序** 
![metadataInfo03](http://img.xinzhuxiansheng.com/blogimgs/chat2db/metadataInfo03.png) 


### 查询databases,tables,columns,keys,indexs    
根据以上内容，通过ConnectInfo对象构建JDBC Connection，通过api获取相应查询即可,具体JDBC查询细节，请参考`SqlExecutor`类。 


