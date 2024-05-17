# Flink on Kubernetes - Kubernetes Operator - FlinkResourceListener Plugin 监听 Job Status 实战                         

>Operator version: 1.8    

## 引言            
大家可能会对 `Operator FlinkResourceListener Plugin`有些不熟悉，下面来看，官网对它的介绍(https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/plugins/#custom-flink-resource-listeners)。          

> #### Custom Flink Resource Listeners          
```bash
The Flink Kubernetes Operator allows users to listen to events and status updates triggered for the Flink Resources managed by the operator. This feature enables tighter integration with the user’s own data platform.

By implementing the FlinkResourceListener interface users can listen to both events and status updates per resource type (FlinkDeployment / FlinkSessionJob). These methods will be called after the respective events have been triggered by the system. Using the context provided on each listener method users can also get access to the related Flink resource and the KubernetesClient itself in order to trigger any further events etc on demand.

Similar to custom validator implementations, resource listeners are loaded via the Flink Plugins(https://nightlies.apache.org/flink/flink-docs-master/docs/deployment/filesystems/plugins/) mechanism.

In order to enable your custom FlinkResourceListener you need to:

    Implement the interface
    Add your listener class to org.apache.flink.kubernetes.operator.api.listener.FlinkResourceListener in META-INF/services
    Package your JAR and add it to the plugins directory of your operator image (/opt/flink/plugins)  

```

根据以上描述，可知, Operator 提供 Plugin 机制，其目录在 Docker Images的 `/opt/flink/plugins`, 若针对监听 FlinkDeployment、FlinkSessionJob 的 `Flink Job events and status updates`, 可通过实现 `FlinkResourceListener interface`来实现该需求。      

估计看到这，你肯定也想蠢蠢欲试了。因为这对于我们解决实际生产环境中实时获取作业状态，提供了很大便利，当然这也带来一些不稳定的因素，下面我们就分析下：          
![resourcelisteners01](http://img.xinzhuxiansheng.com/blogimgs/flink/resourcelisteners01.png) 

* Job Status、Event `主动 Push` 到`自建平台` 比 `轮询查询` Flink REST API 或者 Query Prometheus（上报 Flink Metrics）**实时性强、开发成本低**。   
* FlinkResourceListener Plugin 开发，`增加了 Operator JVM 运行不稳定因素`（开发人员技术层次不齐），其实我个人真的不偏向于这点，Operator 机制就是为了简化用户使用、开发成本。      

接下来，`探索 FlinkResourceListener Plugin 监听 Job Status 的实现`。             

## 开发 FlinkResourceListener Plugin    

### 搭建项目  
**项目目录结构：**    
![resourcelisteners02](http://img.xinzhuxiansheng.com/blogimgs/flink/resourcelisteners02.png)    

**pom.xml:**    
```xml    

... 省略部分代码 
 <dependencies>
        <!-- LOGGING begin -->
        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>slf4j-api</artifactId>
            <version>1.7.24</version>
        </dependency>
        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>slf4j-ext</artifactId>
            <version>1.7.24</version>
        </dependency>
        <!-- 代码直接调用commons-logging会被桥接到slf4j -->
        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>jcl-over-slf4j</artifactId>
            <version>1.7.24</version>
        </dependency>

        <!-- 代码直接调用java.util.logging会被桥接到slf4j -->
        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>jul-to-slf4j</artifactId>
            <version>1.7.24</version>
        </dependency>
        <dependency>
            <groupId>org.apache.logging.log4j</groupId>
            <artifactId>log4j-api</artifactId>
            <version>2.8.2</version>
        </dependency>
        <dependency>
            <groupId>org.apache.logging.log4j</groupId>
            <artifactId>log4j-1.2-api</artifactId>
            <version>2.8.2</version>
        </dependency>
        <dependency>
            <groupId>org.apache.logging.log4j</groupId>
            <artifactId>log4j-core</artifactId>
            <version>2.8.2</version>
        </dependency>
        <!-- log4j-slf4j-impl(用于log4j2与slf4j集成) -->
        <dependency>
            <groupId>org.apache.logging.log4j</groupId>
            <artifactId>log4j-slf4j-impl</artifactId>
            <version>2.8.2</version>
        </dependency>

        <!-- LOGGING end -->
        <dependency>
            <groupId>org.apache.flink</groupId>
            <artifactId>flink-kubernetes-operator</artifactId>
            <version>1.8.0</version>
            <scope>provided</scope>
        </dependency>
        <dependency>
            <groupId>io.fabric8</groupId>
            <artifactId>kubernetes-client</artifactId>
            <version>6.8.1</version>
            <scope>provided</scope>
        </dependency>

    </dependencies>

    <build>
        <plugins>
            <plugin>
                <artifactId>maven-compiler-plugin</artifactId>
                <configuration>
                    <source>${java.version}</source>
                    <target>${java.version}</target>
                    <encoding>UTF-8</encoding>
                </configuration>
            </plugin>

        </plugins>

    </build>
</project>
```

### 创建 MyFlinkResourceListener 实现 Flink ResourceListener 接口  
```java
package com.javamain.flink;
import org.apache.flink.kubernetes.operator.api.FlinkDeployment;
import org.apache.flink.kubernetes.operator.api.FlinkSessionJob;
import org.apache.flink.kubernetes.operator.api.listener.FlinkResourceListener;
import org.apache.flink.kubernetes.operator.api.status.FlinkDeploymentStatus;
import org.apache.flink.kubernetes.operator.api.status.FlinkSessionJobStatus;
import org.apache.flink.kubernetes.operator.listener.AuditUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import java.time.Instant;
public class MyFlinkResourceListener implements FlinkResourceListener {
    private static final Logger logger = LoggerFactory.getLogger(MyFlinkResourceListener.class);

    @Override
    public void onDeploymentStatusUpdate(StatusUpdateContext<FlinkDeployment, FlinkDeploymentStatus> statusUpdateContext) {
        FlinkDeployment flinkResource = statusUpdateContext.getFlinkResource();
        FlinkDeploymentStatus previousStatus = statusUpdateContext.getPreviousStatus();
        FlinkDeploymentStatus newStatus = statusUpdateContext.getNewStatus();
        Instant timestamp = statusUpdateContext.getTimestamp();

        printlnFlinkDeploymentJobStatus("FlinkDeployment previousStatus", flinkResource, previousStatus);
        printlnFlinkDeploymentJobStatus("FlinkDeployment newStatus", flinkResource, newStatus);
    }

    private void printlnFlinkDeploymentJobStatus(String tag, FlinkDeployment flinkResource, FlinkDeploymentStatus jobStatus) {
        String namespace = flinkResource.getMetadata().getNamespace();
        String jobName = flinkResource.getMetadata().getName(); // NAME
        String state = jobStatus.getJobStatus().getState(); // JOB STATUS
        String lifecycleState = jobStatus.getLifecycleState().name(); // LIFECYCLE STATE
        logger.info("TAG: {}, NAMESPACE: {}, NAME: {}, JOB STATUS: {}, LIFECYCLE STATE: {}",
                tag,
                namespace,
                jobName,
                state,
                lifecycleState);
    }


    @Override
    public void onDeploymentEvent(ResourceEventContext<FlinkDeployment> resourceEventContext) {
        AuditUtils.logContext(resourceEventContext);
    }

    @Override
    public void onSessionJobStatusUpdate(StatusUpdateContext<FlinkSessionJob, FlinkSessionJobStatus> statusUpdateContext) {
        FlinkSessionJob flinkResource = statusUpdateContext.getFlinkResource();
        FlinkSessionJobStatus previousStatus = statusUpdateContext.getPreviousStatus();
        FlinkSessionJobStatus newStatus = statusUpdateContext.getNewStatus();
        Instant timestamp = statusUpdateContext.getTimestamp();

        printlnFlinkSessionJobStatus("FlinkSessionJob previousStatus", flinkResource, previousStatus);
        printlnFlinkSessionJobStatus("FlinkSessionJob newStatus", flinkResource, newStatus);
    }

    private void printlnFlinkSessionJobStatus(String tag, FlinkSessionJob flinkResource, FlinkSessionJobStatus jobStatus) {
        String namespace = flinkResource.getMetadata().getNamespace();
        String jobName = flinkResource.getMetadata().getName(); // NAME
        String state = jobStatus.getJobStatus().getState(); // JOB STATUS
        String lifecycleState = jobStatus.getLifecycleState().name(); // LIFECYCLE STATE
        logger.info("TAG: {}, NAMESPACE: {}, NAME: {}, JOB STATUS: {}, LIFECYCLE STATE: {}",
                tag,
                namespace,
                jobName,
                state,
                lifecycleState);
    }

    @Override
    public void onSessionJobEvent(ResourceEventContext<FlinkSessionJob> resourceEventContext) {
        AuditUtils.logContext(resourceEventContext);
    }
}
```

### 定义 SPI 实现类   
在 `resources`目录下 创建 `META-INF\services` 目录， 并创建 以`org.apache.flink.kubernetes.operator.api.listener.FlinkResourceListener` 命名的文件，内容是 MyFlinkResourceListener 的`全限定名` eg: com.javamain.flink.MyFlinkResourceListener    

### 配置 Operator Plugin Listener 参数    
此处有些一波三折，目前 Flink Operator 官网（https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/configuration/ ）并没有对 Operator Plugin Listener参数做介绍，看了源码之后，本人也给 Flink 社区提了 jira（https://issues.apache.org/jira/browse/FLINK-35357），目前工单已 assigned 给我，我后续给 Flink Operator的文档 提一个PR。     

修改 `flink-kubernetes-operator/conf/flink-conf.yaml` 添加以下内容：  
```shell
# plugin listener 参数格式：
kubernetes.operator.plugins.listeners.<listener-name>.class: <fully-qualified-class-name>  

# 在 flink-config.yaml，添加以下参数 
kubernetes.operator.plugins.listeners.yzhoulistener.class: com.javamain.flink.MyFlinkResourceListener
```

>注意：根据官网介绍（https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/configuration/#dynamic-operator-configuration） Operator `flink-operator-config configMap`其 内部 `flink-conf.yaml` 配置，是支持动态加载的，可以直接修改 configMap YAML, 但注意它默认的加载时间间隔是 5分钟。可查看 Operator Log：  
```bash
2024-05-17 01:11:55,469 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: parallelism.default, 1
2024-05-17 01:11:55,470 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: taskmanager.numberOfTaskSlots, 1
2024-05-17 01:11:55,470 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.reconcile.interval, 15 s
2024-05-17 01:11:55,470 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.metrics.reporter.slf4j.interval, 5 MINUTE
2024-05-17 01:11:55,470 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.observer.progress-check.interval, 5 s
2024-05-17 01:11:55,470 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.health.probe.enabled, true
2024-05-17 01:11:55,471 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.health.probe.port, 8085
2024-05-17 01:11:55,471 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.plugins.listeners.yzhoulistener.class, com.javam
ain.flink.MyFlinkResourceListener
2024-05-17 01:11:55,471 INFO  org.apache.flink.configuration.GlobalConfiguration           [] - Loading configuration property: kubernetes.operator.metrics.reporter.slf4j.factory.class, org.apache
.flink.metrics.slf4j.Slf4jReporterFactory
```   

>注意：完整的项目示例，可访问 `https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-flinkOperatorPlugin`   

### 重新打包 Operator Docker Image & 配置镜像 

vim Dockerfile      
```yaml
FROM ghcr.io/apache/flink-kubernetes-operator:91d67d9
RUN mkdir -p /opt/flink/plugins/yzhou
COPY target/javamain-flinkOperatorPlugin-1.0-SNAPSHOT.jar /opt/flink/plugins/yzhou/javamain-flinkOperatorPlugin-1.0-SNAPSHOT.jar
```

>注意：路径要符合 plugin path的规则， /opt/flink/plugins/[自定义]/xxx.jar ，例如 `RUN mkdir -p /opt/flink/plugins/yzhou`    

```shell
# 打包镜像
docker build -t harbor01.io/yzhou/flink-kubernetes-operator:91d67d9-04 .    

# push镜像
docker push harbor01.io/yzhou/flink-kubernetes-operator:91d67d9-04    
```

vim flink-kubernetes-operator/myvalues.yaml 
```shell
# 将 “ghcr.io/apache/flink-kubernetes-operator” 修改为 “harbor01.io/yzhou/flink-kubernetes-operator”         
# 将 “91d67d9” 修改为 “91d67d9-04”      
image:
  repository: harbor01.io/yzhou/flink-kubernetes-operator # 修改 自己的私服地址  
  pullPolicy: IfNotPresent
  tag: "91d67d9-04" # 修改 版本号
```

### 重新部署 Operator 
>注意：在之前的 Blog 涉及到多次对Operator 进行卸载、安装，后续会简化这部分的介绍    
```bash
# 卸载  
helm uninstall flink-kubernetes-operator -n flink

# 安装
helm install -f myvalues.yaml flink-kubernetes-operator . --namespace flink
```

### 查看 listener plugin 是否在 /opt/flink/plugin下     
自定义 Plugin jar 是否存在 `/opt/flink/plugins/yzhou`目录下        
```bash
# 检查 plugins/yzhou 目录是否存在
root@flink-kubernetes-operator-5597c49d78-tnq42:/opt/flink/plugins# ls
flink-metrics-datadog  flink-metrics-graphite  flink-metrics-influxdb  flink-metrics-jmx  flink-metrics-prometheus  flink-metrics-slf4j  flink-metrics-statsd  yzhou

# 查看 listener plugin jar 是否存在 
root@flink-kubernetes-operator-5597c49d78-tnq42:/opt/flink/plugins# ls yzhou/
javamain-flinkOperatorPlugin-1.0-SNAPSHOT.jar
```

## 通过部署 Flink Job 验证  

### Flink Job YAML  
vim basic-application-deployment-only-ingress-tz.yaml 
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-application-deployment-only-ingress-tz
spec:
  image: flink:1.17
  flinkVersion: v1_17
  ingress: 
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
  serviceAccount: flink
  jobManager:
    resource:
      memory: "2048m"
      cpu: 1
  taskManager:
    resource:
      memory: "2048m"
      cpu: 1
  podTemplate:
    spec:
      containers:
        - name: flink-main-container
          env:
            - name: TZ  # 设置容器运行的时区
              value: Asia/Shanghai
  job:
    jarURI: local:///opt/flink/examples/streaming/StateMachineExample.jar
    parallelism: 2
    upgradeMode: stateless
```

```shell
# 部署
kubectl -n flink apply -f basic-application-deployment-only-ingress-tz.yaml   

# 删除
kubectl -n flink delete -f basic-application-deployment-only-ingress-tz.yaml   
```


### 查看 Listener Log   
>通过部署、删除 可观察到 `MyFlinkResourceListener` log的输出    

**部署 Job的 log输出**
```shell
2024-05-17 01:07:05,290 INFO  org.apache.flink.kubernetes.operator.listener.AuditUtils     [] - >>> Event  | Info    | JOBSTATUSCHANGED | Job status changed from CREATED to RUNNING
2024-05-17 01:07:05,291 INFO  org.apache.flink.kubernetes.operator.listener.AuditUtils     [] - >>> Event  | Info    | JOBSTATUSCHANGED | Job status changed from CREATED to RUNNING
2024-05-17 01:07:05,409 INFO  com.javamain.flink.MyFlinkResourceListener                   [] - TAG: FlinkDeployment previousStatus, NAMESPACE: flink, NAME: basic-application-deployment-only-ing
ress-tz, JOB STATUS: CREATED, LIFECYCLE STATE: DEPLOYED
2024-05-17 01:07:05,410 INFO  com.javamain.flink.MyFlinkResourceListener                   [] - TAG: FlinkDeployment newStatus, NAMESPACE: flink, NAME: basic-application-deployment-only-ingress-tz
, JOB STATUS: RUNNING, LIFECYCLE STATE: STABLE
2024-05-17 01:07:05,410 INFO  org.apache.flink.kubernetes.operator.listener.AuditUtils     [] - >>> Status | Info    | STABLE          | The resource deployment is considered to be stable and won’
t be rolled back
```

**删除 Job的 log输出**  
```bash
2024-05-17 01:10:49,095 INFO  org.apache.flink.kubernetes.operator.controller.FlinkDeploymentController [] - Cleaning up FlinkDeployment
2024-05-17 01:10:49,195 INFO  org.apache.flink.kubernetes.operator.listener.AuditUtils     [] - >>> Event  | Info    | CLEANUP         | Cleaning up FlinkDeployment
2024-05-17 01:10:49,195 INFO  org.apache.flink.kubernetes.operator.listener.AuditUtils     [] - >>> Event  | Info    | CLEANUP         | Cleaning up FlinkDeployment
2024-05-17 01:10:49,252 INFO  org.apache.flink.autoscaler.JobAutoScalerImpl                [] - Cleaning up autoscaling meta data
2024-05-17 01:10:49,254 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Job is running, cancelling job.
2024-05-17 01:10:49,276 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Job successfully cancelled.
2024-05-17 01:10:49,276 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Deleting cluster with Foreground propagation
2024-05-17 01:10:49,277 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Scaling JobManager Deployment to zero with 300 seconds timeout...
2024-05-17 01:10:51,717 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Completed Scaling JobManager Deployment to zero
2024-05-17 01:10:51,718 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Deleting JobManager Deployment with 297 seconds timeout...
2024-05-17 01:10:53,657 INFO  org.apache.flink.kubernetes.operator.service.AbstractFlinkService [] - Completed Deleting JobManager Deployment
```

>注意：通过 log，总结规律，目前 `MyFlinkResourceListener` 案例，仅打印的log，并没有将 Job Status 、Event 数据调用接口请求给自建平台，我想这部分的实现不算多麻烦， 大家可以实践起来。     

## 总结    
FlinkResourceListener Plugin 很好的弥补了轮询监控带来的延迟性， `如果直接通过 K8s Client Watch Flink Job Status，也是可以的，这也体现了 Operator的优势`, 因为 Operator 将Flink Job Status 也暴露在 CRD中，例如：   
```bash
[root@k8s01 job_yaml]# kubectl -n flink get flinkdeployments    
NAME                                           JOB STATUS   LIFECYCLE STATE
basic-application-deployment-only-ingress-tz   RUNNING      STABLE
```

后面，我会再介绍一篇 “Akka Cluster 集成 K8s Client 监听 Flink Job Status”;        

refer              
1.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/plugins/        
2.https://nightlies.apache.org/flink/flink-kubernetes-operator-docs-release-1.8/docs/operations/configuration/  
