# Flink on Kubernetes - Kubernetes Operator - 开发 Java程序提交 Job

>Operator version: 1.8，Flink version: 1.17         

## 修订   
在上一篇 Blog 'Flink on Kubernetes - Kubernetes Operator - Flink Ingress 配置 & Application Job 部署示例' 介绍使用 MetalLB的使用，当时的意图是为了 通过它进行集群外部访问，在本人实践案例中，metallb 确实已经启动的作业可支持 浏览器访问 Flink Web UI， 但这里存在意外， 当 `metallb.ip.yaml`的 address 指向 集群节点的IP段，当我手动删除一些 Pod, 预期会快速拉起，可现实是启动很慢，后来查看 `Pod Describe` 会存在大量的下面的日志：     
```shell
Warning  FailedCreatePodSandBox  48s               kubelet            Failed to create pod sandbox: rpc error: code = U
nknown desc = [failed to set up sandbox container "d384665c69292bad5f9ebd10d0cab13005cc66d6656051ab8313273f90672445" netw
ork for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to set up pod "flink-kubernetes-operato
r-59878dff7-d6cnw_flink" network: error getting ClusterInformation: Get "https://10.96.0.1:443/apis/crd.projectcalico.org
/v1/clusterinformations/default": net/http: TLS handshake timeout, failed to clean up sandbox container "d384665c69292bad
5f9ebd10d0cab13005cc66d6656051ab8313273f90672445" network for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlu
gin cni failed to teardown pod "flink-kubernetes-operator-59878dff7-d6cnw_flink" network: error getting ClusterInformatio
n: Get "https://10.96.0.1:443/apis/crd.projectcalico.org/v1/clusterinformations/default": net/http: TLS handshake timeout
]                                                                                                                        
  Warning  FailedCreatePodSandBox  37s               kubelet            Failed to create pod sandbox: rpc error: code = U
nknown desc = failed to set up sandbox container "bea1875d098ca1455dc3e1c8a0f6d91bbd35023118aaeab7347558a3896c3038" netwo
rk for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to set up pod "flink-kubernetes-operator
-59878dff7-d6cnw_flink" network: error getting ClusterInformation: Get "https://10.96.0.1:443/apis/crd.projectcalico.org/
v1/clusterinformations/default": net/http: TLS handshake timeout                                                         
  Normal   SandboxChanged          3s (x5 over 69s)  kubelet            Pod sandbox changed, it will be killed and re-cre
ated.                                                                                                                    
  Warning  FailedCreatePodSandBox  3s                kubelet            Failed to create pod sandbox: rpc error: code = U
nknown desc = [failed to set up sandbox container "f0025c162c332318263440d433ad6d0e8d17ee587131b8c1e03279f254dfd17b" netw
ork for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to set up pod "flink-kubernetes-operato
r-59878dff7-d6cnw_flink" network: Get "https://10.96.0.1:443/apis/crd.projectcalico.org/v1/ippools?limit=500": net/http: 
TLS handshake timeout, failed to clean up sandbox container "f0025c162c332318263440d433ad6d0e8d17ee587131b8c1e03279f254df
d17b" network for pod "flink-kubernetes-operator-59878dff7-d6cnw": networkPlugin cni failed to teardown pod "flink-kubern
etes-operator-59878dff7-d6cnw_flink" network: error getting ClusterInformation: Get "https://10.96.0.1:443/apis/crd.proje
ctcalico.org/v1/clusterinformations/default": net/http: TLS handshake timeout]
``` 

上面的情况是必现。 当卸载 metallb 或者 将`metallb.ip.yaml`的 address 设置为`非集群节点的IP段`，则恢复正常。（经过多次实践得到的规律），`以上的异常日志目前是怀疑 网络走了 metallb 代理，但又无法解决，所以修正这块`。        

所以，这里修订下这里，在 Blog 后面，会通过将`ingress-nginx`的 Service type 指定为 `NodePort` 对外暴露。         

## 修改 ingress-nginx 的 Service type 为 NodePort  
>注意，下面操作 是基于之前 Blog `Flink on Kubernetes - Kubernetes集群搭建 - 安装 Ingress` 基础上修改。      

```shell
vim deploy.yaml
# 修改 381 行 ，将 type 修改成 NodePort   
type: NodePort

# 执行 deploy.yaml，生效 Service  
kubectl apply -f deploy.yaml 
```

查看 ingress-nginx 的 service： 
```bash
ingress-nginx   ingress-nginx-controller                        NodePort      10.96.36.74                  http:80►32717 https:443►30075      
```

剩下的，修改宿主机 Hosts  将 flink.k8s.io 映射到 ingress-nginx-controller Pod 所在的虚机节点 IP上，例如：   
```shell
kubectl get pod -A -o wide |grep ingress-nginx-controller     
``` 
```bash
ingress-nginx    ingress-nginx-controller-65c995b75d-jbp8k                    1/1     Running   0               3d4h    192.166.89.137    k8s06   <none>           <none>
```

添加 `192.168.0.145 flink.k8s.io`, 浏览器访问 `http://flink.k8s.io:32717/flink/basic-application-deployment-only-ingress/#/overview`    

>注意:        
1.URL 地址需注意是否是 "/"结尾，这块在 之前 Blog有提到，特别重要的点，错误示例： http://flink.k8s.io:32717/flink/basic-application-deployment-only-ingress， 正确示例：http://flink.k8s.io:32717/flink/basic-application-deployment-only-ingress/           
2.宿主机配置 flink.k8s.io 域名映射的 IP 一定要是 ingress-nginx-controller Pod 所在的虚机节点 IP     

到此，修订部分算是完结， 接下来，我们回到该篇 Blog 主题 `Flink on Kubernetes - Kubernetes Operator - 开发 Java程序提交 Job`     

## 引言 
在之前 "Flink on Kubernetes" Blog 中介绍过手动部署 Session 集群、Application 集群，我想你应该也深有体会的是在整个 Flink Job 提交部署的过程中，从手动编写 yaml 到使用 kubectl 提交 yaml 到 Kubernetes。          

![k8sclientjar01](http://img.xinzhuxiansheng.com/blogimgs/flink/k8sclientjar01.png)        

那么，接下来开始探讨，如何通过 Java程序部署Flink Job？      

## 了解 Flink Kubernetes Operator   
在 Flink YAML 文件中会存在名为`kind: FlinkDeployment`的Kubernetes 资源定义。 当通过 Kubectl 提交 YAML给 K8s API Server后，是由 Flink Kubernetes Operator来处理 kind: FlinkDeployment 的。 所以 Github 中 `flink-kubernetes-operator` 是我们的突破口。       

### clone operator 代码 
```shell
git clone git@github.com:apache/flink-kubernetes-operator.git   
```

目录结构如下：  
![k8sclientjar02](http://img.xinzhuxiansheng.com/blogimgs/flink/k8sclientjar02.png)        

**其重点是 examples目录下的模块**       
![k8sclientjar03](http://img.xinzhuxiansheng.com/blogimgs/flink/k8sclientjar03.png)        

在 `kubernetes-client-examples`模块 下的 Basic.java， 已经明确给出 Kubernetes client 提交 Flink Job 示例。      

>参考 example 的示例，搭建 Java Maven 项目 ...         

## 构建 Maven 项目 

### 添加依赖    

>依赖什么jar，可参考 example的示例代码所依赖的jar，这里就不过多解释。   

1.添加 flink kubernetes operator Jar                
```xml
<!-- https://mvnrepository.com/artifact/org.apache.flink/flink-kubernetes-operator -->
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-kubernetes-operator</artifactId>
    <version>1.8.0</version>
</dependency>
```

2.添加 io.fabric8.kubernetes-client Jar             
```xml
<!-- https://mvnrepository.com/artifact/io.fabric8/kubernetes-client -->
<dependency>
    <groupId>io.fabric8</groupId>
    <artifactId>kubernetes-client</artifactId>
    <version>6.8.1</version>
</dependency>   
```

3.添加 yaml 打印工具类 
```xml
 <dependency>
      <groupId>com.fasterxml.jackson.core</groupId>
      <artifactId>jackson-databind</artifactId>
      <version>2.13.1</version>
  </dependency>
  <dependency>
      <groupId>com.fasterxml.jackson.dataformat</groupId>
      <artifactId>jackson-dataformat-yaml</artifactId>
      <version>2.13.1</version>
  </dependency>
```

### 配置 K8s 环境   
>注意：Kubernetes 环境搭建可参考之前的 Blog     

1）将 K8s集群的 ~/.kube/config 拷贝到 开发机的 ~/.kube/config 目录下（若目录不存在则创建），config的 masterurl地址需配置本地hosts 映射关系。例如： 192.168.0.149 master.k8s.io（在 之前的 Blog "Flink on Kubernetes - Kubernetes集群搭建 - 安装 docker & keepalive & haproxy" 中，介绍了集群规划，双 master 使用 `keepalive & haproxy` 做了双活， 而 192.168.0.149 是虚拟IP, 作为 两个master 的同一入口 ）   

## 示例代码     
我们将之前部署过的 Application Job 的YAML当作示例参考    

### basic-application-deployment-only-ingress.yaml
```yaml
apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  name: basic-application-deployment-only-ingress
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
  job:
    jarURI: local:///opt/flink/examples/streaming/StateMachineExample.jar
    parallelism: 2
    upgradeMode: stateless
```

### 代码    
```java
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import io.fabric8.kubernetes.api.model.ObjectMeta;
import io.fabric8.kubernetes.client.KubernetesClient;
import io.fabric8.kubernetes.client.KubernetesClientBuilder;
import org.apache.flink.kubernetes.operator.api.FlinkDeployment;
import org.apache.flink.kubernetes.operator.api.spec.*;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class Main {

    public static void main(String[] args) throws InterruptedException, IOException {

        // 拼接 YAML 文件
        FlinkDeployment flinkDeployment = new FlinkDeployment();
        flinkDeployment.setApiVersion("flink.apache.org/v1beta1");
        flinkDeployment.setKind("FlinkDeployment");

        ObjectMeta objectMeta = new ObjectMeta();
        objectMeta.setNamespace("flink");
        objectMeta.setName("basic-application-deployment-only-ingress");
        flinkDeployment.setMetadata(objectMeta);

        FlinkDeploymentSpec flinkDeploymentSpec = new FlinkDeploymentSpec();
        flinkDeploymentSpec.setFlinkVersion(FlinkVersion.v1_17);
        flinkDeploymentSpec.setImage("flink:1.17");

        IngressSpec ingressSpec = new IngressSpec();
        ingressSpec.setTemplate("flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)");
        ingressSpec.setClassName("nginx");
        Map<String, String> annotations = new HashMap<>();
        annotations.put("nginx.ingress.kubernetes.io/rewrite-target","/$2");
        ingressSpec.setAnnotations(annotations);
        flinkDeploymentSpec.setIngress(ingressSpec);

        Map<String, String> flinkConfiguration = new HashMap<>();
        flinkConfiguration.put("taskmanager.numberOfTaskSlots", "2");
        flinkDeploymentSpec.setFlinkConfiguration(flinkConfiguration);
        flinkDeploymentSpec.setServiceAccount("flink");
        JobManagerSpec jobManagerSpec = new JobManagerSpec();
        jobManagerSpec.setResource(new Resource(1.0, "2048m","2G"));
        flinkDeploymentSpec.setJobManager(jobManagerSpec);
        TaskManagerSpec taskManagerSpec = new TaskManagerSpec();
        taskManagerSpec.setResource(new Resource(1.0, "2048m","2G"));
        flinkDeploymentSpec.setTaskManager(taskManagerSpec);
        flinkDeployment.setSpec(flinkDeploymentSpec);
        flinkDeployment
                .getSpec()
                .setJob(
                        JobSpec.builder()
                                .jarURI(
                                        "local:///opt/flink/examples/streaming/StateMachineExample.jar")
                                .parallelism(2)
                                .upgradeMode(UpgradeMode.STATELESS)
                                .build());

        // 打印 内容
        String yaml = toYaml(flinkDeployment);
        System.out.println("打印 Flink Job YAML");
        System.out.println(yaml);

        // 提交 Job
        try (KubernetesClient kubernetesClient = new KubernetesClientBuilder().build()) {
            kubernetesClient.resource(flinkDeployment).createOrReplace();
        }

        System.out.println("Job 提交结束");
    }

    public static String toYaml(Object obj) throws IOException {
        ObjectMapper mapper = new ObjectMapper(new YAMLFactory());
        mapper.findAndRegisterModules();  // Registers all available modules including Java Time modules
        return mapper.writeValueAsString(obj);
    }
}
```

### 输出的 YAML 及 意外收货   
>注意：存在意外收货   

当 IDEA终端，打印的 YAML：  
```yaml
---
apiVersion: "flink.apache.org/v1beta1"
kind: "FlinkDeployment"
metadata:
  name: "basic-application-deployment-only-ingress"
  namespace: "flink"
spec:
  job:
    jarURI: "local:///opt/flink/examples/streaming/StateMachineExample.jar"
    parallelism: 2
    entryClass: null
    args: null
    state: null
    savepointTriggerNonce: null
    initialSavepointPath: null
    checkpointTriggerNonce: null
    upgradeMode: "stateless"
    allowNonRestoredState: null
    savepointRedeployNonce: null
  restartNonce: null
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
  image: "flink:1.17"
  imagePullPolicy: null
  serviceAccount: "flink"
  flinkVersion: "v1_17"
  ingress:
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
    labels: null
    tls: null
  podTemplate: null
  jobManager:
    resource:
      cpu: 1.0
      memory: "2048m"
      ephemeralStorage: "2G"
    replicas: 1
    podTemplate: null
  taskManager:
    resource:
      cpu: 1.0
      memory: "2048m"
      ephemeralStorage: "2G"
    replicas: null
    podTemplate: null
  logConfiguration: null
  mode: null
status:
  jobStatus:
    jobName: null
    jobId: null
    state: null
    startTime: null
    updateTime: null
    savepointInfo:
      lastSavepoint: null
      triggerId: null
      triggerTimestamp: null
      triggerType: null
      formatType: null
      savepointHistory: []
      lastPeriodicSavepointTimestamp: 0
    checkpointInfo:
      lastCheckpoint: null
      triggerId: null
      triggerTimestamp: null
      triggerType: null
      formatType: null
      lastPeriodicCheckpointTimestamp: 0
  error: null
  observedGeneration: null
  lifecycleState: "CREATED"
  clusterInfo: {}
  jobManagerDeploymentStatus: "MISSING"
  reconciliationStatus:
    reconciliationTimestamp: 0
    lastReconciledSpec: null
    lastStableSpec: null
    state: "UPGRADING"
  taskManager: null
```

看到这个，太惊喜了， 在 Flink Job 部署的时候，往往 YAML的构造是最辛苦的，甚至它有哪些其他配置以及配置层级关系，都让我们摸不着头脑，现在有了这个YAML，它将配置项都打印出来，剩下的，我们来慢慢搞懂，这些配置项的作用。    

>希望你能重视它 YAML 。       

到此，我们就完成了 Java 程序提交 Job 的案例演示，`别忘记继续探索 flink-kubernetes-operator 源码中的 Example 模块`。       

refer   
1.https://github.com/apache/flink-kubernetes-operator          