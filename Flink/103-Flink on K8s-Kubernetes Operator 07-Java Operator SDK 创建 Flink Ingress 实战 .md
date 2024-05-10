# Flink on Kubernetes - Kubernetes Operator - Java Operator SDK 创建 Flink Ingress 实战              

## 引言     
为了后面更好介绍 `Flink Kubernetes Operator` 实现的细节，我们通过开发一个 Java Operator，了解 Operator 的执行过程。      

>#### What Is a Kubernetes Operator?     
```bash
In Kubernetes parlance, an Operator is a software component, usually deployed in a cluster, that manages the lifecycle of a set of resources. It extends the native set of controllers, such as replicaset and job controllers, to manage complex or interrelated components as a single-managed unit.          
Let’s look at a few common use cases where operators are used:              

    Enforce best practices when deploying applications to a cluster         
    Keep track and recover from accidentally removing/changing resources used by an application     
    Automate housekeeping tasks associated with an application, such as regular backups and cleanups        
    Automate off-cluster resource provisioning — for example, storage buckets and certificates      
    Improve application developers’ experience when interacting with Kubernetes in general      
    Improve overall security by allowing users to manage only application-level resources instead of low-level ones such as pods and deployments        
    Expose application-specific resources (a.k.a. Custom Resource Definitions) as Kubernetes resources      

This last use case is quite interesting. It allows a solution provider to leverage the existing practices around regular Kubernetes resources to manage application-specific resources. The main benefit is that anyone adopting this application can use existing infrastructure-as-code tools.            

To give us an idea of the different kinds of available operators, we can check the OperatorHub.io site(https://operatorhub.io/). There, we’ll find operators for popular databases, API managers, development tools, and others.      
```     

## 需求介绍        
在之前的 Blog “Flink on Kubernetes - Kubernetes Operator - Flink Ingress 配置 & Application Job 部署示例” 介绍过 `Flink Kubernetes Operator` 提供的自动创建 Ingress的 YAML 配置，示例内容如下(该篇重点介绍的核心是这里):                
```yaml
metadata:
  namespace: default
  name: advanced-ingress
spec:
  image: flink:1.17
  flinkVersion: v1_17
  ingress:
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
```

**为了让大家不脱离上下文，我再回溯下原来的内容：**    
在使用 `Flink Kubernetes Operator` 部署 Flink Job后，会帮我们创建 2个 Service（Type：ClusterIP），但这并不能让我们从集群外部能访问 Flink Web UI           
```bash
[root@k8s01 k8s_yaml]# kubectl get svc -n flink  
NAME                                                TYPE        CLUSTER-IP     EXTERNAL-IP   PORT(S)             AGE
basic-application-deployment-only-ingress-tz        ClusterIP   None           <none>        6123/TCP,6124/TCP   18h
basic-application-deployment-only-ingress-tz-rest   ClusterIP   10.96.17.230   <none>        8081/TCP            18h
flink-operator-webhook-service                      ClusterIP   10.96.139.50   <none>        443/TCP             19h
```

若我们在 Flink Job YAML 中添加 ingress 相关配置，Operator 会根据我们提供的 `ingress.template`配置项来创建 Ingress 资源，可参考 Flink Operator 的架构图：           

![operatoringress01](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress01.png)              

```bash  
[root@k8s01 k8s_yaml]# kubectl get ingress -n flink    
NAME                                           CLASS   HOSTS          ADDRESS       PORTS   AGE
basic-application-deployment-only-ingress-tz   nginx   flink.k8s.io   10.96.36.74   80      18h
```

Operator 创建 Ingress，我们也可以使用 YAML来 手动创建。下面单独给出 Flink Job Ingress YAML                
>注意：手动创建 Ingress YAML，需根据 Job name, namespace, service name 调整参数     

vim basic-application-deployment-only-ingress-tz-ingress.yaml  内容如下：        
```bash
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  annotations:
    nginx.ingress.kubernetes.io/rewrite-target: /$2
  name: basic-application-deployment-only-ingress-tz
  namespace: flink
spec:
  ingressClassName: nginx
  rules:
  - host: flink.k8s.io
    http:
      paths:
      - backend:
          service:
            name: basic-application-deployment-only-ingress-tz-rest
            port:
              number: 8081
        path: /flink/basic-application-deployment-only-ingress-tz(/|$)(.*)
        pathType: ImplementationSpecific
```

背景铺垫差不多了，那么现在的需求是：            
* 不使用 Flink Kubernetes Operator 创建 Ingress 资源情况下   
* 开发一个 `FlinkIngressEye Operator` 来创建 Flink Job Ingress        
* 监听 Flink Job Service, 若没有 Ingress，则创建 Ingress，若 Service 删除后，Ingress 也需删除        

![operatoringress02](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress02.png)          


## 了解 java operator sdk     
>若对 Kubernetes Operator 之前没有了解过的，建议大家可以去B站学习下（可学习 Go 开发 Operator的编码流程，这对理解 java operator sdk 处理逻辑有一定好处）, 也可访问:  
1.https://blog.container-solutions.com/kubernetes-operators-explained,    
2.https://blog.container-solutions.com/a-deep-dive-into-the-java-operator-sdk,      
3.https://developers.redhat.com/articles/2022/02/15/write-kubernetes-java-java-operator-sdk#what_about_framework_support_,      
4.https://www.youtube.com/watch?v=CvftaV-xrB4 (非常建议看)  了解 它的基本运作。     

(英文啃的好，技术差不了 :)      

选择 `java operator sdk`, 是因为 Flink Kubernetes Operator 使用了它，所以方便后续 Operator 代码的学习，选择它进行案例实践，可加深对它的理解 。      
```xml
<operator.sdk.version>4.8.3</operator.sdk.version>
<dependency>
    <groupId>io.javaoperatorsdk</groupId>
    <artifactId>operator-framework</artifactId>
    <version>${operator.sdk.version}</version>
</dependency>    
```

可访问 `https://javaoperatorsdk.io/docs/getting-started` 了解它的 Features， 从阅读文档开始：     

### 必知词汇表  
**Primary Resource** - the resource that represents the desired state that the controller is working to achieve. While this is often a Custom Resource, it can be also be a Kubernetes native resource (Deployment, ConfigMap,…).     

**Secondary Resource** - any resource that the controller needs to manage the reach the desired state represented by the primary resource. These resources can be created, updated, deleted or simply read depending on the use case. For example, the Deployment controller manages ReplicaSet instances when trying to realize the state represented by the Deployment. In this scenario, the Deployment is the primary resource while ReplicaSet is one of the secondary resources managed by the Deployment controller.     

**Dependent Resource** - a feature of JOSDK, to make it easier to manage secondary resources. A dependent resource represents a secondary resource with related reconciliation logic.     

**Low-level API** - refers to the SDK APIs that don’t use any of features (such as `Dependent Resources` or `Workflows`) outside of the core
Reconciler interface. See the WebPage sample . The same logic is also implemented using Dependent Resource and Workflows

### 功能点  

### 实现 Reconciler and/or Cleaner      
从 Operator 的角度来看，`Kubernetes 资源的生命周期可以根据资源是被创建或更新，还是被标记为删除，清楚地分为两个阶段`(这句话非常重要, 与FlinkIngressEye Operator的处理逻辑完美贴合 )。                      

框架会自动处理与此相关的逻辑。框架总是会调用 `reconcile()` 方法，除非自定义资源被标记为删除。另一方面，如果资源被标记为删除且 调解器实现了 `Cleaner` 接口，那么会调用 `cleanup()` 方法。实现 `Cleaner` 接口可使开发者能够让 SDK 清理相关状态（例如，集群外的资源）。 因此，SDK 将自动添加与你的 Reconciler 关联的 Finalizer，以便 Kubernetes 服务器在你的 Reconciler 有机会清理之前不会删除您的资源。有关更多详细信息，请参阅 `Finalizer Support`。       

![operatoringress03](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress03.png)        
实现 `Reconciler 接口` 可执行资源创建或更新，实现 `Cleaner 接口` 可执行资源删除， 后面会介绍 finalizers，它可资源实际被删除之前，在 Reconciler 执行其他操作。        

### Reconciliation     
```bash
在 Kubernetes 中，**调解**（Reconciliation）是指一种持续、周期性地检查和修复资源状态的过程，以确保实际状态符合用户期望的状态。这个过程通常由自定义控制器或操作员（Operator）实现，最终的目的是让实际的集群状态与声明性配置保持一致。            

调解过程主要涉及以下几点：          
1. **获取当前状态**：控制器从 Kubernetes 集群中获取资源的当前状态（通常是通过观察相应的自定义资源）。                
2. **与期望状态进行比较**：比较当前状态与期望状态之间的差异。
3. **采取行动**：基于差异，采取必要的行动来使实际状态符合期望状态，例如创建、更新或删除资源等。             

在 Kubernetes Operator SDK 中，`Reconciler` 接口的 `reconcile` 方法用于执行这种调解操作。当 Kubernetes API 服务器检测到自定义资源（CR）的变更时，框架会调用 `reconcile` 方法来调整资源状态。            

**例子**：      
- 如果一个自定义控制器负责管理一个应用程序的部署资源，调解器会在检测到自定义资源的期望副本数与当前实际副本数不同步时，调整部署的副本数，使其达到预期的配置。                    
```   

![operatoringress04](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress04.png)      

>### Finalizer Support          
```bash
Kubernetes finalizers 确保在资源标记为删除后，在资源实际被删除之前，你的 Reconciler 有机会执行操作。如果没有 finalizers，资源将直接被 Kubernetes 服务器删除。                 

根据你的使用情况，你可能需要或不需要使用 finalizers 。特别是，如果你的 operator 不需要清理任何`不会被 Kubernetes 集群自动管理的状态`（例如，外部资源），你可能不需要使用 Finalizer。你应该尽可能使用 Kubernetes 垃圾回收机制，通过为你的 `secondary resources` 设置所有者引用，这样当关联的`primary resource`被删除时，集群就会自动删除它们。需要注意的是，设置所有者引用是 Reconciler 实现的责任，尽管依赖资源使该过程变得更容易。     

如果确实需要清理这种状态，你需要使用 finalizers ，以防止 Kubernetes 集群在你的 operator 准备好允许删除资源之前将其删除。这样，即使`你的 operator 在用户“删除”资源时处于停机状态，清理仍然可以进行`。         

JOSDK 通过在需要时自动处理管理 finalizers ，以这种方式清理资源变得更容易。你唯一需要做的就是让 SDK 知道你 operator 的 `primary resources` 实现 `Cleaner<P>` 接口。如果你的 Reconciler 没有实现 `Cleaner` 接口，SDK 将认为你在资源删除时不需要执行任何清理，并因此不会启用 finalizers support 。换句话说，只有当你的 Reconciler 实现了 `Cleaner` 接口时，才会添加 finalizers support。                 

框架会在创建资源之后，第一次协调之前，自动添加 finalizers 。finalizers 是通过单独的 Kubernetes API 调用添加的。由于这次更新，finalizers 将出现在资源上。然后，reconciliation 可以像往常一样进行。   

在 Reconciler 上执行清理后，自动添加的 finalizers 也将被删除。如上所述，这种行为是可自定义的，当我们讨论使用 DeleteControl 时有解释。     
```

### 使用 UpdateControl and DeleteControl          
这两个类用于`控制 reconciliation 后的结果`或`期望的行为`。    
`UpdateControl` 可以指示框架更新资源的 `status` 子资源，并/或以所需的时间延迟重新安排一次 reconciliation 。           

注意，尽管如此，应该优先使用 `EventSources` 而不是重新调度，因为这样 reconciliation 操作将只会在需要时触发，而不是定期触发。    

以上是资源更新的典型用例，但在某些情况下，controller 可能想要更新资源本身（例如添加注释），或不执行任何更新，这也是支持的。   

也可以使用 `updateResourceAndStatus()` 方法同时更新状态和资源。在这种情况下，资源首先被更新，然后更新状态，使用两个单独的请求与 Kubernetes API 通信。   

你应该始终使用 `UpdateControl` 来声明意图，并让 SDK 处理实际的更新，而不是直接使用 Kubernetes 客户端执行这些更新，这样 SDK 可以相应地更新其内部状态。   

资源更新使用`optimistic version control`进行保护，以确保不会覆盖同时在服务器上发生的其他更新。这是通过在处理的资源上设置 `resourceVersion` 字段来确保的。                     

`DeleteControl` 通常指示框架在清理依赖资源后，在 `cleanup` 实现中移除 finalizers 。               

## 需求案例开发    
创建 Ingress Reconciler，会监听 Kubernetes Reconciliation 的 Service 资源， 在 reconcile() 创建 ingress资源， 在 cleanup() 删除 ingress 资源        
![operatoringress05](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress05.png)     

在判断 Service 是否是 Flink Service时，是根据 YAML的 Labels.type = 'flink-native-kubernetes', 注意 Flink Kubernetes Operator 与 Flink Native Kubernetes 两种方式部署的 Flink Job，它们所创建的 Service Labels.type 都是 `flink-native-kubernetes`。         

当 Flink Job 删除时，2个Service 也会被删除，cleanup() 也会被执行2遍，可参考 reconcile() 方法，去掉一个 Service Name 不包含 “-rest”。      


>不知道大家是否有像我一样的疑惑？   
```bash
1.搭建 Java Operator 项目，是否有项目脚手架 ? ，像 `Go Kubebuilder Cli` 那样。        
有项目脚手架，但没必要，因为它跟普通的java main 项目一样           
2.Java Operator 项目如何调试?     
像 java main 项目一样，可直接在 Idea 中启动 main() 即可。  
```
![operatoringress06](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress06.png)      

**pom.xml k8s 相关依赖如下：**  
```xml
<dependency>
    <groupId>io.fabric8</groupId>
    <artifactId>kubernetes-client</artifactId>
    <version>6.8.1</version>
</dependency>
<dependency>
    <groupId>io.javaoperatorsdk</groupId>
    <artifactId>operator-framework</artifactId>
    <version>4.8.3</version>
</dependency>
```

**FlinkIngressReconciler.java 内容如下：**  
```java
import com.javamain.operator.flinkingresseye.K8sClientSingleton;
import io.fabric8.kubernetes.api.model.ObjectMeta;
import io.fabric8.kubernetes.api.model.ObjectMetaBuilder;
import io.fabric8.kubernetes.api.model.Service;
import io.fabric8.kubernetes.api.model.networking.v1.*;
import io.fabric8.kubernetes.client.dsl.Resource;
import io.javaoperatorsdk.operator.api.reconciler.*;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.text.MessageFormat;
import java.util.HashMap;
import java.util.Map;

@ControllerConfiguration(namespaces = Constants.WATCH_ALL_NAMESPACES)
public class FlinkIngressReconciler implements Reconciler<Service>, ErrorStatusHandler<Service>, Cleaner<Service> {
    private static final Logger logger = LoggerFactory.getLogger(FlinkIngressReconciler.class);

    @Override
    public UpdateControl<Service> reconcile(Service service, Context<Service> context) {
        if (service == null) {
            return UpdateControl.noUpdate();
        }
        String serviceName = service.getMetadata().getName();
        String namespace = service.getMetadata().getNamespace();
        /*
            因为 Flink Job 会创建 2个 service，所以，根据 “-rest” 特征, 去掉一个
            为啥选用，-rest ，因为 在 Ingress 绑定 rest service, 还有 它绑定时 Flink Web UI
         */
        if (!serviceName.contains("-rest")) {
            return UpdateControl.noUpdate();
        }
        if (!isFlinkServiceAndCheckLabelsAppIsValid(service)) {
            return UpdateControl.noUpdate();
        }
        String jobName = getServiceLabelValue(service, "app");

        Ingress existingIngress = getIngressByName(namespace, jobName).get();
        if (existingIngress != null) {
            // 若存在，也不去更新
            return UpdateControl.noUpdate();
        }
        logger.info("Creating Ingress for Service: {}", serviceName);
        // 构建或更新 Ingress 资源
        Ingress ingress = buildIngressFromService(service);
        K8sClientSingleton.getKubernetesClient().
                network().v1().ingresses().inNamespace(namespace).create(ingress);
        return UpdateControl.noUpdate();
    }

    @Override
    public ErrorStatusUpdateControl<Service> updateErrorStatus(Service service, Context<Service> context, Exception e) {
        // 错误处理逻辑
        return ErrorStatusUpdateControl.noStatusUpdate();
    }

    /*
        检查是否是 Flink Service
        type: flink-native-kubernetes
     */
    private boolean isFlinkServiceAndCheckLabelsAppIsValid(Service service) {
        Map<String, String> labels = getServiceLabels(service);
        boolean isFlinkService = labels != null &&
                labels.containsKey("type") &&
                "flink-native-kubernetes".equalsIgnoreCase(labels.get("type"));
        /*
            观察 Flink Kubernetes Operator 的 ingress 创建的 name 与 Flink Job name 一样，
            所以取 labels 的 app 值 赋值给 ingress name
         */
        boolean appIsValid = labels.containsKey("app") && StringUtils.isNotBlank(labels.get("app"));
        return isFlinkService && appIsValid;
    }

    private Map<String, String> getServiceLabels(Service service) {
        return service.getMetadata().getLabels();
    }

    private Resource<Ingress> getIngressByName(String namespace, String jobName) {
        Resource<Ingress> ingressResource = K8sClientSingleton.getKubernetesClient().
                network().
                v1().
                ingresses().
                inNamespace(namespace).
                withName(jobName);
        return ingressResource;
    }

    /**
     * 获取给定 Service 的某个标签值
     */
    public String getServiceLabelValue(Service service, String labelKey) {
        Map<String, String> labels = getServiceLabels(service);
        return labels != null ? labels.get(labelKey) : null;
    }

    private Ingress buildIngressFromService(Service service) {
        String serviceName = service.getMetadata().getName();
        String namespace = service.getMetadata().getNamespace();
        Map<String, String> labels = service.getMetadata().getLabels();
        String jobName = labels.get("app");

        // 设置注释
        Map<String, String> annotations = new HashMap<>();
        annotations.put("nginx.ingress.kubernetes.io/rewrite-target", "/$2");

        // 构建 Ingress metadata
        ObjectMeta ingressMetadata = new ObjectMetaBuilder()
                .withName(jobName)
                .withNamespace(namespace)
                .withAnnotations(annotations)
                .build();

        String path = MessageFormat.format("/{0}/{1}(/|$)(.*)", namespace, jobName);
        ServiceBackendPort port = new ServiceBackendPort();
        port.setNumber(8081);

        // 构建 Ingress spec
        IngressSpec ingressSpec = new IngressSpecBuilder()
                .withIngressClassName("nginx")
                .withRules(new IngressRuleBuilder()
                        .withHost("flink.k8s.io")
                        .withHttp(new HTTPIngressRuleValueBuilder()
                                .withPaths(new HTTPIngressPathBuilder()
                                        .withPath(path)
                                        .withPathType("ImplementationSpecific")
                                        .withNewBackend()
                                        .withService(new IngressServiceBackendBuilder()
                                                .withName(serviceName)
                                                .withPort(port)
                                                .build())
                                        .endBackend()
                                        .build())
                                .build())
                        .build())
                .build();

        // 构建 Ingress
        return new IngressBuilder()
                .withMetadata(ingressMetadata)
                .withSpec(ingressSpec)
                .build();
    }

    @Override
    public DeleteControl cleanup(Service service, Context<Service> context) {
        if (isFlinkServiceAndCheckLabelsAppIsValid(service)) {
            String namespace = service.getMetadata().getNamespace();
            String serviceName = service.getMetadata().getName();
            if (!serviceName.contains("-rest")) {
                return DeleteControl.defaultDelete();
            }
            String jobName = getServiceLabelValue(service, "app");
            Ingress existingIngress = getIngressByName(namespace, jobName).get();
            K8sClientSingleton.getKubernetesClient().
                    network().
                    v1().ingresses().inNamespace(namespace)
                    .withName(jobName)
                    .delete(existingIngress);
            logger.info("ingress was deleted");
        }
        // 注意：若不符合过滤条件，则按照 默认删除方式，确保移除 finalizer
        return DeleteControl.defaultDelete();
    }
}
```

**Runner.java 内容如下：**
```java
import io.javaoperatorsdk.operator.Operator;

public class Runner {
    public static void main(String[] args) {
            // 创建 Operator 实例
            Operator operator = new Operator();
            // 注册 Reconciler
            operator.register(new FlinkIngressReconciler());
            // 启动 Operator
            operator.start();
    }
}
``` 

**K8sClientSingleton.java 内容如下：**  
```java
import io.fabric8.kubernetes.client.KubernetesClient;
import io.fabric8.kubernetes.client.KubernetesClientBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class K8sClientSingleton {
    private static final Logger logger = LoggerFactory.getLogger(K8sClientSingleton.class);
    // 暂不考虑 config
    private static final KubernetesClient KUBERNETES_CLIENT = new KubernetesClientBuilder().build();

    private K8sClientSingleton() {
    }

    public static KubernetesClient getKubernetesClient() {
        return KUBERNETES_CLIENT;
    }
}
```

以上是完整的 FlinkIngressEye Operator 的全部实现，也可访问 https://github.com/xinzhuxiansheng/javamain-services/tree/main/javamain-k8sOperator/src/main/java/com/javamain/operator/flinkingresseye/way1 查看完整项目。    


### Flink Job YAML 测试 FlinkIngressEye Operator    

#### 部署 Flink Job
1.启动 `Runner#main()`方法。    

2.部署一个不带有 ingress.template 配置 `basic-application-deployment-only.yaml` 
```yaml 
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-application-deployment-only
spec:
  image: flink:1.17
  flinkVersion: v1_17
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
  job:
    jarURI: local:///opt/flink/examples/streaming/StateMachineExample.jar
    parallelism: 2
    upgradeMode: stateless
```

执行 flink job yaml 
```shell  
[root@k8s01 job_yaml]# kubectl -n flink apply -f  basic-application-deployment-only.yaml 
flinkdeployment.flink.apache.org/basic-application-deployment-only created
``` 

3.观察 ingress 
```shell  
[root@k8s01 job_yaml]# kubectl get ingress -n flink 
NAME                                CLASS   HOSTS          ADDRESS       PORTS   AGE
basic-application-deployment-only   nginx   flink.k8s.io   10.96.36.74   80      2m50s
```

4.观察 idea console log 
![operatoringress07](http://img.xinzhuxiansheng.com/blogimgs/flink/operatoringress07.png)    

可访问 Flink Web UI，例如：http://flink.k8s.io:32717/flink/basic-application-deployment-only/   

#### 删除 Flink Job   
```shell
[root@k8s01 job_yaml]# kubectl -n flink delete -f basic-application-deployment-only.yaml 
flinkdeployment.flink.apache.org "basic-application-deployment-only" deleted      
```

1.观察 ingress    
无 ingress        

2.idea console log     
```bash
[ FlinkIngressReconciler.java:162 ] - [ INFO ]  ingress was deleted   
```   

从测试结果来看，是符合需求设计的      


## 打包 & 部署    

### 打包 jar
```shell
mvn clean package
``` 

### 打包 docker image 
vim Dockerfile  
```bash
FROM openjdk:11-jre-slim
WORKDIR /opt
COPY target/javamain-k8sOperator-1.0-SNAPSHOT.jar /opt/javamain-k8sOperator-1.0-SNAPSHOT.jar
ENTRYPOINT ["java", "-jar", "/opt/javamain-k8sOperator-1.0-SNAPSHOT.jar"]
``` 

```shell
# 打包镜像（符合 harbor 私服路径）
docker build -t harbor01.io/yzhou/flink-ingresseye-operator:0.0.1 .     

# 推送到 仓库
docker push harbor01.io/yzhou/flink-ingresseye-operator:0.0.1
```

### 创建 ServiceAccount 和 RoleBindings 
为 Operator 创建一个专门的 ServiceAccount，并且为其分配适当的权限：  

vim service-account.yaml  
```yaml
apiVersion: v1
kind: ServiceAccount
metadata:
  name: flink-ingresseye-operator-sa
  namespace: flink
```

vim role-binding.yaml  
```yaml
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: flink-ingresseye-operator-role
rules:
  - apiGroups: ["", "networking.k8s.io"]
    resources: ["services", "ingresses"]
    verbs: ["get", "list", "watch", "create", "update", "delete"]

---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: flink-ingresseye-operator-role-binding
subjects:
  - kind: ServiceAccount
    name: flink-ingresseye-operator-sa
    namespace: flink
roleRef:
  kind: ClusterRole
  name: flink-ingresseye-operator-role
  apiGroup: rbac.authorization.k8s.io
```

执行：    
```shell
[root@k8s01 operator]# kubectl -n flink apply -f service-account.yaml 
serviceaccount/flink-ingresseye-operator-sa created
[root@k8s01 operator]# kubectl -n flink apply -f role-binding.yaml 
clusterrole.rbac.authorization.k8s.io/flink-ingresseye-operator-role created
clusterrolebinding.rbac.authorization.k8s.io/flink-ingresseye-operator-role-binding created
```

### 部署 Operator 到 Kubernetes     

vim deployment.yaml       
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: flink-ingresseye-operator
  namespace: flink
spec:
  replicas: 1
  selector:
    matchLabels:
      app: flink-ingresseye-operator
  template:
    metadata:
      labels:
        app: flink-ingresseye-operator
    spec:
      serviceAccountName: flink-ingresseye-operator-sa
      containers:
        - name: flink-ingresseye-operator
          image: harbor01.io/yzhou/flink-ingresseye-operator:0.0.1
          imagePullPolicy: Always
```

执行：  
```shell
[root@k8s01 operator]# kubectl -n flink apply -f deployment.yaml 
deployment.apps/flink-ingresseye-operator created   
```   

查看 Operator pod：
```shell
[root@k8s01 operator]# kubectl get pod -n flink |grep ingress
flink-ingresseye-operator-56d7458559-ckrc7   1/1     Running   0               92s    
```

查看 Operator log：   
```shell
[root@k8s01 operator]# kubectl logs -n flink --tail=500 --follow flink-ingresseye-operator-56d7458559-ckrc7
2024-05-09 18:06:22  [ BaseConfigurationService.java:61 ] - [ WARN ]  Configuration for reconciler 'flinkingressreconciler' was not found. Known reconcilers: None.
2024-05-09 18:06:22  [ BaseConfigurationService.java:83 ] - [ INFO ]  Created configuration for reconciler com.javamain.operator.flinkingresseye.way1.FlinkIngressReconciler with name flinkingressreconciler
2024-05-09 18:06:22  [ Operator.java:243 ] - [ INFO ]  Registered reconciler: 'flinkingressreconciler' for resource: 'class io.fabric8.kubernetes.api.model.Service' for namespace(s): [all namespaces]
2024-05-09 18:06:22  [ Operator.java:147 ] - [ INFO ]  Operator SDK 4.8.3 (commit: 8d100b8) built on 2024-04-15T14:15:19.000+0000 starting...
2024-05-09 18:06:22  [ Operator.java:153 ] - [ INFO ]  Client version: 6.8.1
2024-05-09 18:06:22  [ Controller.java:336 ] - [ INFO ]  Starting 'flinkingressreconciler' controller for reconciler: com.javamain.operator.flinkingresseye.way1.FlinkIngressReconciler, resource: io.fabric8.kubernetes.api.model.Service
2024-05-09 18:06:23  [ Controller.java:348 ] - [ INFO ]  'flinkingressreconciler' controller started      
```

### 根据之前的测试步骤，再做一遍测试  

测试后，查看 Operator log, 符合预期结果 
```bash
2024-05-09 18:11:29  [ FlinkIngressReconciler.java:46 ] - [ INFO ]  Creating Ingress for Service: basic-application-deployment-only-rest
2024-05-09 18:12:18  [ FlinkIngressReconciler.java:162 ] - [ INFO ]  ingress was deleted
```

## 总结   
以上所有，完成了 Flink ingress 根据 Service 的生命周期进行 被创建、被删除。 后面让我们接着探索 Flink Kubernetes Operator 奥义 。      
Java Operator SDK 有不少优秀项目正在使用，可访问 https://github.com/operator-framework/java-operator-sdk?tab=readme-ov-file#projects-using-josdk 查看。 其中包含 `Strimzi Access operator.`, 这里值得关注的是“CNCF 孵化 Strimzi 以简化 Kubernetes 上的Kafka”， 所以后面，我也会关注Kafka on Kubernetes 方向。    

refer       
1.https://javaoperatorsdk.io/docs/getting-started               
2.https://github.com/eugenp/tutorials/tree/master/kubernetes-modules/k8s-operator                   
3.https://www.baeldung.com/java-kubernetes-operator-sdk       
4.https://sgitario.github.io/java-sdk-operator-getting-started/         
5.https://blog.container-solutions.com/kubernetes-operators-explained          
6.https://blog.container-solutions.com/a-deep-dive-into-the-java-operator-sdk   
