# Flink 源码 - Native Kubernetes - 探索 Flink Native Kubernetes Application Model Job 动态调节 Log4j log 级别

>Flink version: 1.15.4, Flink Job Model: Native Kubernetes Application, Kubernetes version: 1.30.8        

## 扩展       
在上一篇公众号文章 `Flink - 探索 Flink Cli 脚本提交 Application Job 时的参数读取和 Log4j 配置 ` (https://mp.weixin.qq.com/s/_xUN4DD4le3YhAi_IlbK6g) 内容中，我们提到 Flink Cli 部署的 Native Kubernetes Application Model Job，它通过事先定义好的`FLINK_CONF_DIR` 路径读取`配置参数` 和 `log 配置文件` 创建 ConfigMap 挂载到 `opt/flink/conf` 目录下的。     

![adjustloglevel02](http://img.xinzhuxiansheng.com/blogimgs/flink/adjustloglevel02.jpg)      

>注意，`FLINK_CONF_DIR`环境变量是在 bin/config.sh 脚本导入的，所以在 Flink Cli 场景下是符合的。   

假设：你现在需要通过反射调用 Cli 的 main() 再去提交 Flink Native Kubernetes Application Model Job。这种场景在自研的平台中经常遇到。那对于 Job 的配置参数和 log 配置文件读取路径的可选项就增加了。    

查询 Flink conf/ 路径的逻辑，在 Flink Cli 源码中会有多个 路径的判断， 所以 `FLINK_CONF_DIR` 并不是绝对的。 大家可以查看 `CliFrontend#getConfigurationDirectoryFromEnv()` 和 `FlinkConfMountDecorator#getLocalLogConfFiles()` 方法。   


>接下来，我们回到该篇 Blog 的主题: `动态调节 Log4j log 级别`。      


## 动态调节 Flink Job Log 级别  
这部分的实践细节，需要依赖1个非常重要的部分, 是 `log4j-console.properties` 默认开启定期扫描文件变更，并在必要时调整日志记录行为。默认情况下30秒检查一次，监测间隔可以通过 Log4j 配置文件的 monitorInterval 配置项进行设置。       

![adjustloglevel01](http://img.xinzhuxiansheng.com/blogimgs/flink/adjustloglevel01.jpg)           
https://nightlies.apache.org/flink/flink-docs-release-1.15/zh/docs/deployment/advanced/logging/   

可以在 `log4j-console.properties` 中看到 `monitorInterval` 配置。     

![adjustloglevel03](http://img.xinzhuxiansheng.com/blogimgs/flink/adjustloglevel03.jpg)   

部署 Flink Native Kubernetes Application Model Job时，使用 `kubectl edit configmap flink-config-flink-application-test -n flink` 命令修改存储 Flink Conf 的 ConfigMap。   
修改 `log4j-console.properties` 的 rootLogger.level 从 INFO 改成 DEBUG 。 

经过 30s，我们可以查看 JobManager,TaskManager Log 打印了 DEBUG 级别的日志。  
![adjustloglevel04](http://img.xinzhuxiansheng.com/blogimgs/flink/adjustloglevel04.jpg)    

![adjustloglevel05](http://img.xinzhuxiansheng.com/blogimgs/flink/adjustloglevel05.jpg)   

动态调节 Log 级别，对于我们排查问题，至关重要。 这里我还存在一些实践问题，我想针对某个包名下的 Log 调整 Log 级别 或者 针对某个类调整 Log 级别，可是我测试的结果是不成功的。后续有任何进展会再同步给大家。   

refer   
1.https://nightlies.apache.org/flink/flink-docs-release-1.15/docs/deployment/advanced/logging/              
