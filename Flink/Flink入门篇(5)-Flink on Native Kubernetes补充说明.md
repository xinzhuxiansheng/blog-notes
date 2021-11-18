
## flink native kubernetes integration

`参考文档`  
flink:
https://ci.apache.org/projects/flink/flink-docs-release-1.13/docs/deployment/ha/kubernetes_ha/
https://ci.apache.org/projects/flink/flink-docs-release-1.13/docs/deployment/resource-providers/native_kubernetes/
https://ci.apache.org/projects/flink/flink-docs-release-1.13/docs/deployment/resource-providers/standalone/kubernetes/

blog:
https://segmentfault.com/a/1190000039198813
https://www.cnblogs.com/wynjauu/articles/9455229.html
https://blog.csdn.net/baichoufei90/article/details/102718487

运行一条命令就可以在k8s集群搭建起flink的弹性集群，这就是flink native Kubernetes integration

```shell
./bin/flink run-application \
    --target kubernetes-application \
    -Dkubernetes.cluster-id=first-application-cluster \
    -Dkubernetes.container.image=docker-dev/job:4.1.0-202107011136-e2e38 \
    -Dkubernetes.container.image.pull-secrets=arm-repo-secret \
    -Dkubernetes.rest-service.exposed.type=NodePort \
    -Dkubernetes.pod-template-file.jobmanager=./job-template.yaml \
    -Dhigh-availability=org.apache.flink.kubernetes.highavailability.KubernetesHaServicesFactory \
    -Dhigh-availability.storageDir=file:///tmp/flink \
    -Dclassloader.resolve-order=parent-first \
    -c com.ericsson.cv.reactor.job.ReactorJobApplication \
    local:///opt/flink/usrlib/job-4.1.0-202107011136-e2e38-with-dependency.jar --config.path "/opt/flink/usrconf/config.properties" --dispatcher.config.path "/opt/flink/usrconf/dispatcher-config.json"
``` 
下面是运行上面命令时的一些准备工作。

down flink
我们要运行flink的命令，所以要先下载好flink的发行包，flink-13.0以后的版本，下载好后解压。

K8s环境
要准备好k8s的环境，即运行flink命令行的机器要可以执行kuebctl，并且有权限创建pod等资源。或者有./kube/config也可以。

RBAC
需要pod有权限创建和管理k8s资源，因此需要考虑RBAC。

```shell
apiVersion: rbac.authorization.k8s.io/v1beta1
kind: ClusterRoleBinding
metadata:
  name: fabric8-rbac
subjects:
  - kind: ServiceAccount
    # Reference to upper's `metadata.name`
    name: default
    # Reference to upper's `metadata.namespace`
    namespace: default
roleRef:
  kind: ClusterRole
  name: cluster-admin
  apiGroup: rbac.authorization.k8s.io
Docker
```
从私服拉取docker image时要加imagePullSecrets，用下面命令创建好se
```shell
kubectl create secret generic arm-repo-secret \
    --from-file=.dockerconfigjson=/home/eyfnane/.docker/config.json \
    --type=kubernetes.io/dockerconfigjson
```
kubectl get secret可以查看到arm-repo-secret被创建好

使用pod-template
创建ConfigMap
```
apiVersion: v1
kind: ConfigMap
metadata:
  name: flink-config
data:
  config.properties: |+
    source.bootstrap.servers=190.117.146.5:9092
    source.auto.offset.reset=latest
    sink.bootstrap.servers=190.117.146.5:9092
    source.topics=raccoon.reactor.event
    source.group.id=raccoon.reactor.event.group_ft
    kafka.sink.topic=raccoon.reactor.sink
    watermarks.maxOutOfOrderness=1
    database.url=jdbc:postgresql://190.120.103.38:5432/job_db
    database.user=xxx
    database.password=xxx
    parallelism.max=64
    parallelism.default=1
    parallelism.vehicleKeyExtractor=1
    parallelism.vehiclePreEnrich=1
    parallelism.vehicleEnrich=1
    enrichment.vehicle.enabled=false
    event.source.type=predefined
    checkpoint.enable=true
    checkpoint.interval=95000
    checkpoint.minPause=500
    checkpoint.timeout=60000
    checkpoint.maxConcurrent=1
    checkpoint.enableExternalized=DELETE_ON_CANCELLATION
    sink.silence.period.max=2592000

  dispatcher-config.json: |+
    {
          "connections": [
          {
            "name": "dispatcher-flink-1",
            "type": "dispatcher-flink",
            "configs": {
              "auto_offset_reset": "earliest",
              "kafka_poll_interval_sec": 1
            }
          }
    }
```
上面内容命名为flink-cm.yaml kubectl apply -f flink-cm.yaml 创建出ConfigMap，创建成功后可以kubectl get cm查看到创建的ConfigMap。

创建和编辑jobmanager-pod-template
```
apiVersion: v1
kind: Pod
metadata:
  name: jobmanager-pod-template
spec:
  containers:
    # Do not change the main container name
    - name: flink-main-container
      resources:
        requests:
          ephemeral-storage: 2048Mi
        limits:
          ephemeral-storage: 2048Mi
      volumeMounts:
        - name: config-volume
          mountPath: /opt/flink/usrconf/config.properties
          subPath: config.properties
        - name: dispatcher-config-volume
          mountPath: /opt/flink/usrconf/dispatcher-config.json
          subPath: dispatcher-config.json
  volumes:
    - name: config-volume
      configMap:
        name: flink-config
        items:
          - key: config.properties
            path: config.properties
    - name: dispatcher-config-volume
      configMap:
        name: flink-config
        items:
          - key: dispatcher-config.json
            path: dispatcher-config.json
```
利用之前创建好的名为flink-config的ConfigMap来配置到pod-template中实现把自定义的配置挂载到docker想要的目录下。

上面的准备工作完成后来解释一下这条命令的各个参数

```shell
./bin/flink run-application \运行flink命令以application模式
    --target kubernetes-application \部署到k8s application模式
    -Dkubernetes.cluster-id=first-application-cluster \指定一个群集的id，不能重复
    -Dkubernetes.container.image=docker-dev/job:4.1.0-202107011136-e2e38 \docker镜像名字
    -Dkubernetes.container.image.pull-secrets=arm-repo-secret \从私服拉取要设置secret
    -Dkubernetes.rest-service.exposed.type=NodePort \暴露flink-web-ui的方式
    -Dkubernetes.pod-template-file.jobmanager=./job-template.yaml \使用pod-template,上面准备的文件
    -Dhigh-availability=org.apache.flink.kubernetes.highavailability.KubernetesHaServicesFactory \用k8s做HA
    -Dhigh-availability.storageDir=file:///tmp/flink \做HA时要存储的一些信息checkpoint等
    -Dclassloader.resolve-order=parent-first \kafka的classloader有问题时，显示指定来兼容
    -c com.ericsson.cv.reactor.job.ReactorJobApplication \指定启动类，main()方法所在的类
    local:///opt/flink/usrlib/job-4.1.0-202107011136-e2e38-with-dependency.jar --config.path "/opt/flink/usrconf/config.properties" --dispatcher.config.path "/opt/flink/usrconf/dispatcher-config.json" \指定自己的jar包，local协议是appliction模式支持的，这个包是在docker里面的jar包，不是本地， 后面是jar的main()方法的agrs参数， --key value --key value 的形式，四个参数组成两个键值被flink的ParameterTool使用。
```
还有很多参数可以设置如下面。。。

```
taskmanager.numberOfTaskSlots: 1
high-availability.storageDir: file:///usr/share/reactor/recovery/ft/
state.checkpoints.dir: file:///usr/share/reactor/checkpoints/ft/
state.savepoints.dir: file:///usr/share/reactor/savepoints/ft/
state.backend: rocksdb
state.backend.incremental: true
queryable-state.enable: true
```
命令参考
方便调试查询列出一些配置

```
flink - help
$ ./bin/flink -h
./flink <ACTION> [OPTIONS] [ARGUMENTS]

The following actions are available:

Action "run" compiles and runs a program.

  Syntax: run [OPTIONS] <jar-file> <arguments>
  "run" action options:
     -c,--class <classname>               Class with the program entry point
                                          ("main()" method). Only needed if the
                                          JAR file does not specify the class in
                                          its manifest.
     -C,--classpath <url>                 Adds a URL to each user code
                                          classloader  on all nodes in the
                                          cluster. The paths must specify a
                                          protocol (e.g. file://) and be
                                          accessible on all nodes (e.g. by means
                                          of a NFS share). You can use this
                                          option multiple times for specifying
                                          more than one URL. The protocol must
                                          be supported by the {@link
                                          java.net.URLClassLoader}.
     -d,--detached                        If present, runs the job in detached
                                          mode
     -n,--allowNonRestoredState           Allow to skip savepoint state that
                                          cannot be restored. You need to allow
                                          this if you removed an operator from
                                          your program that was part of the
                                          program when the savepoint was
                                          triggered.
     -p,--parallelism <parallelism>       The parallelism with which to run the
                                          program. Optional flag to override the
                                          default value specified in the
                                          configuration.
     -py,--python <pythonFile>            Python script with the program entry
                                          point. The dependent resources can be
                                          configured with the `--pyFiles`
                                          option.
     -pyarch,--pyArchives <arg>           Add python archive files for job. The
                                          archive files will be extracted to the
                                          working directory of python UDF
                                          worker. Currently only zip-format is
                                          supported. For each archive file, a
                                          target directory be specified. If the
                                          target directory name is specified,
                                          the archive file will be extracted to
                                          a directory with the specified name.
                                          Otherwise, the archive file will be
                                          extracted to a directory with the same
                                          name of the archive file. The files
                                          uploaded via this option are
                                          accessible via relative path. '#'
                                          could be used as the separator of the
                                          archive file path and the target
                                          directory name. Comma (',') could be
                                          used as the separator to specify
                                          multiple archive files. This option
                                          can be used to upload the virtual
                                          environment, the data files used in
                                          Python UDF (e.g., --pyArchives
                                          file:///tmp/py37.zip,file:///tmp/data.
                                          zip#data --pyExecutable
                                          py37.zip/py37/bin/python). The data
                                          files could be accessed in Python UDF,
                                          e.g.: f = open('data/data.txt', 'r').
     -pyexec,--pyExecutable <arg>         Specify the path of the python
                                          interpreter used to execute the python
                                          UDF worker (e.g.: --pyExecutable
                                          /usr/local/bin/python3). The python
                                          UDF worker depends on Python 3.6+,
                                          Apache Beam (version == 2.27.0), Pip
                                          (version >= 7.1.0) and SetupTools
                                          (version >= 37.0.0). Please ensure
                                          that the specified environment meets
                                          the above requirements.
     -pyfs,--pyFiles <pythonFiles>        Attach custom files for job. The
                                          standard resource file suffixes such
                                          as .py/.egg/.zip/.whl or directory are
                                          all supported. These files will be
                                          added to the PYTHONPATH of both the
                                          local client and the remote python UDF
                                          worker. Files suffixed with .zip will
                                          be extracted and added to PYTHONPATH.
                                          Comma (',') could be used as the
                                          separator to specify multiple files
                                          (e.g., --pyFiles
                                          file:///tmp/myresource.zip,hdfs:///$na
                                          menode_address/myresource2.zip).
     -pym,--pyModule <pythonModule>       Python module with the program entry
                                          point. This option must be used in
                                          conjunction with `--pyFiles`.
     -pyreq,--pyRequirements <arg>        Specify a requirements.txt file which
                                          defines the third-party dependencies.
                                          These dependencies will be installed
                                          and added to the PYTHONPATH of the
                                          python UDF worker. A directory which
                                          contains the installation packages of
                                          these dependencies could be specified
                                          optionally. Use '#' as the separator
                                          if the optional parameter exists
                                          (e.g., --pyRequirements
                                          file:///tmp/requirements.txt#file:///t
                                          mp/cached_dir).
     -s,--fromSavepoint <savepointPath>   Path to a savepoint to restore the job
                                          from (for example
                                          hdfs:///flink/savepoint-1537).
     -sae,--shutdownOnAttachedExit        If the job is submitted in attached
                                          mode, perform a best-effort cluster
                                          shutdown when the CLI is terminated
                                          abruptly, e.g., in response to a user
                                          interrupt, such as typing Ctrl + C.
  Options for Generic CLI mode:
     -D <property=value>   Allows specifying multiple generic configuration
                           options. The available options can be found at
                           https://ci.apache.org/projects/flink/flink-docs-stabl
                           e/ops/config.html
     -e,--executor <arg>   DEPRECATED: Please use the -t option instead which is
                           also available with the "Application Mode".
                           The name of the executor to be used for executing the
                           given job, which is equivalent to the
                           "execution.target" config option. The currently
                           available executors are: "remote", "local",
                           "kubernetes-session", "yarn-per-job", "yarn-session".
     -t,--target <arg>     The deployment target for the given application,
                           which is equivalent to the "execution.target" config
                           option. For the "run" action the currently available
                           targets are: "remote", "local", "kubernetes-session",
                           "yarn-per-job", "yarn-session". For the
                           "run-application" action the currently available
                           targets are: "kubernetes-application".

  Options for yarn-cluster mode:
     -m,--jobmanager <arg>            Set to yarn-cluster to use YARN execution
                                      mode.
     -yid,--yarnapplicationId <arg>   Attach to running YARN session
     -z,--zookeeperNamespace <arg>    Namespace to create the Zookeeper
                                      sub-paths for high availability mode

  Options for default mode:
     -D <property=value>             Allows specifying multiple generic
                                     configuration options. The available
                                     options can be found at
                                     https://ci.apache.org/projects/flink/flink-
                                     docs-stable/ops/config.html
     -m,--jobmanager <arg>           Address of the JobManager to which to
                                     connect. Use this flag to connect to a
                                     different JobManager than the one specified
                                     in the configuration. Attention: This
                                     option is respected only if the
                                     high-availability configuration is NONE.
     -z,--zookeeperNamespace <arg>   Namespace to create the Zookeeper sub-paths
                                     for high availability mode



Action "run-application" runs an application in Application Mode.

  Syntax: run-application [OPTIONS] <jar-file> <arguments>
  Options for Generic CLI mode:
     -D <property=value>   Allows specifying multiple generic configuration
                           options. The available options can be found at
                           https://ci.apache.org/projects/flink/flink-docs-stabl
                           e/ops/config.html
     -e,--executor <arg>   DEPRECATED: Please use the -t option instead which is
                           also available with the "Application Mode".
                           The name of the executor to be used for executing the
                           given job, which is equivalent to the
                           "execution.target" config option. The currently
                           available executors are: "remote", "local",
                           "kubernetes-session", "yarn-per-job", "yarn-session".
     -t,--target <arg>     The deployment target for the given application,
                           which is equivalent to the "execution.target" config
                           option. For the "run" action the currently available
                           targets are: "remote", "local", "kubernetes-session",
                           "yarn-per-job", "yarn-session". For the
                           "run-application" action the currently available
                           targets are: "kubernetes-application".



Action "info" shows the optimized execution plan of the program (JSON).

  Syntax: info [OPTIONS] <jar-file> <arguments>
  "info" action options:
     -c,--class <classname>           Class with the program entry point
                                      ("main()" method). Only needed if the JAR
                                      file does not specify the class in its
                                      manifest.
     -p,--parallelism <parallelism>   The parallelism with which to run the
                                      program. Optional flag to override the
                                      default value specified in the
                                      configuration.


Action "list" lists running and scheduled programs.

  Syntax: list [OPTIONS]
  "list" action options:
     -a,--all         Show all programs and their JobIDs
     -r,--running     Show only running programs and their JobIDs
     -s,--scheduled   Show only scheduled programs and their JobIDs
  Options for Generic CLI mode:
     -D <property=value>   Allows specifying multiple generic configuration
                           options. The available options can be found at
                           https://ci.apache.org/projects/flink/flink-docs-stabl
                           e/ops/config.html
     -e,--executor <arg>   DEPRECATED: Please use the -t option instead which is
                           also available with the "Application Mode".
                           The name of the executor to be used for executing the
                           given job, which is equivalent to the
                           "execution.target" config option. The currently
                           available executors are: "remote", "local",
                           "kubernetes-session", "yarn-per-job", "yarn-session".
     -t,--target <arg>     The deployment target for the given application,
                           which is equivalent to the "execution.target" config
                           option. For the "run" action the currently available
                           targets are: "remote", "local", "kubernetes-session",
                           "yarn-per-job", "yarn-session". For the
                           "run-application" action the currently available
                           targets are: "kubernetes-application".

  Options for yarn-cluster mode:
     -m,--jobmanager <arg>            Set to yarn-cluster to use YARN execution
                                      mode.
     -yid,--yarnapplicationId <arg>   Attach to running YARN session
     -z,--zookeeperNamespace <arg>    Namespace to create the Zookeeper
                                      sub-paths for high availability mode

  Options for default mode:
     -D <property=value>             Allows specifying multiple generic
                                     configuration options. The available
                                     options can be found at
                                     https://ci.apache.org/projects/flink/flink-
                                     docs-stable/ops/config.html
     -m,--jobmanager <arg>           Address of the JobManager to which to
                                     connect. Use this flag to connect to a
                                     different JobManager than the one specified
                                     in the configuration. Attention: This
                                     option is respected only if the
                                     high-availability configuration is NONE.
     -z,--zookeeperNamespace <arg>   Namespace to create the Zookeeper sub-paths
                                     for high availability mode



Action "stop" stops a running program with a savepoint (streaming jobs only).

  Syntax: stop [OPTIONS] <Job ID>
  "stop" action options:
     -d,--drain                           Send MAX_WATERMARK before taking the
                                          savepoint and stopping the pipelne.
     -p,--savepointPath <savepointPath>   Path to the savepoint (for example
                                          hdfs:///flink/savepoint-1537). If no
                                          directory is specified, the configured
                                          default will be used
                                          ("state.savepoints.dir").
  Options for Generic CLI mode:
     -D <property=value>   Allows specifying multiple generic configuration
                           options. The available options can be found at
                           https://ci.apache.org/projects/flink/flink-docs-stabl
                           e/ops/config.html
     -e,--executor <arg>   DEPRECATED: Please use the -t option instead which is
                           also available with the "Application Mode".
                           The name of the executor to be used for executing the
                           given job, which is equivalent to the
                           "execution.target" config option. The currently
                           available executors are: "remote", "local",
                           "kubernetes-session", "yarn-per-job", "yarn-session".
     -t,--target <arg>     The deployment target for the given application,
                           which is equivalent to the "execution.target" config
                           option. For the "run" action the currently available
                           targets are: "remote", "local", "kubernetes-session",
                           "yarn-per-job", "yarn-session". For the
                           "run-application" action the currently available
                           targets are: "kubernetes-application".

  Options for yarn-cluster mode:
     -m,--jobmanager <arg>            Set to yarn-cluster to use YARN execution
                                      mode.
     -yid,--yarnapplicationId <arg>   Attach to running YARN session
     -z,--zookeeperNamespace <arg>    Namespace to create the Zookeeper
                                      sub-paths for high availability mode

  Options for default mode:
     -D <property=value>             Allows specifying multiple generic
                                     configuration options. The available
                                     options can be found at
                                     https://ci.apache.org/projects/flink/flink-
                                     docs-stable/ops/config.html
     -m,--jobmanager <arg>           Address of the JobManager to which to
                                     connect. Use this flag to connect to a
                                     different JobManager than the one specified
                                     in the configuration. Attention: This
                                     option is respected only if the
                                     high-availability configuration is NONE.
     -z,--zookeeperNamespace <arg>   Namespace to create the Zookeeper sub-paths
                                     for high availability mode



Action "cancel" cancels a running program.

  Syntax: cancel [OPTIONS] <Job ID>
  "cancel" action options:
     -s,--withSavepoint <targetDirectory>   **DEPRECATION WARNING**: Cancelling
                                            a job with savepoint is deprecated.
                                            Use "stop" instead.
                                            Trigger savepoint and cancel job.
                                            The target directory is optional. If
                                            no directory is specified, the
                                            configured default directory
                                            (state.savepoints.dir) is used.
  Options for Generic CLI mode:
     -D <property=value>   Allows specifying multiple generic configuration
                           options. The available options can be found at
                           https://ci.apache.org/projects/flink/flink-docs-stabl
                           e/ops/config.html
     -e,--executor <arg>   DEPRECATED: Please use the -t option instead which is
                           also available with the "Application Mode".
                           The name of the executor to be used for executing the
                           given job, which is equivalent to the
                           "execution.target" config option. The currently
                           available executors are: "remote", "local",
                           "kubernetes-session", "yarn-per-job", "yarn-session".
     -t,--target <arg>     The deployment target for the given application,
                           which is equivalent to the "execution.target" config
                           option. For the "run" action the currently available
                           targets are: "remote", "local", "kubernetes-session",
                           "yarn-per-job", "yarn-session". For the
                           "run-application" action the currently available
                           targets are: "kubernetes-application".

  Options for yarn-cluster mode:
     -m,--jobmanager <arg>            Set to yarn-cluster to use YARN execution
                                      mode.
     -yid,--yarnapplicationId <arg>   Attach to running YARN session
     -z,--zookeeperNamespace <arg>    Namespace to create the Zookeeper
                                      sub-paths for high availability mode

  Options for default mode:
     -D <property=value>             Allows specifying multiple generic
                                     configuration options. The available
                                     options can be found at
                                     https://ci.apache.org/projects/flink/flink-
                                     docs-stable/ops/config.html
     -m,--jobmanager <arg>           Address of the JobManager to which to
                                     connect. Use this flag to connect to a
                                     different JobManager than the one specified
                                     in the configuration. Attention: This
                                     option is respected only if the
                                     high-availability configuration is NONE.
     -z,--zookeeperNamespace <arg>   Namespace to create the Zookeeper sub-paths
                                     for high availability mode



Action "savepoint" triggers savepoints for a running job or disposes existing ones.

  Syntax: savepoint [OPTIONS] <Job ID> [<target directory>]
  "savepoint" action options:
     -d,--dispose <arg>       Path of savepoint to dispose.
     -j,--jarfile <jarfile>   Flink program JAR file.
  Options for Generic CLI mode:
     -D <property=value>   Allows specifying multiple generic configuration
                           options. The available options can be found at
                           https://ci.apache.org/projects/flink/flink-docs-stabl
                           e/ops/config.html
     -e,--executor <arg>   DEPRECATED: Please use the -t option instead which is
                           also available with the "Application Mode".
                           The name of the executor to be used for executing the
                           given job, which is equivalent to the
                           "execution.target" config option. The currently
                           available executors are: "remote", "local",
                           "kubernetes-session", "yarn-per-job", "yarn-session".
     -t,--target <arg>     The deployment target for the given application,
                           which is equivalent to the "execution.target" config
                           option. For the "run" action the currently available
                           targets are: "remote", "local", "kubernetes-session",
                           "yarn-per-job", "yarn-session". For the
                           "run-application" action the currently available
                           targets are: "kubernetes-application".

  Options for yarn-cluster mode:
     -m,--jobmanager <arg>            Set to yarn-cluster to use YARN execution
                                      mode.
     -yid,--yarnapplicationId <arg>   Attach to running YARN session
     -z,--zookeeperNamespace <arg>    Namespace to create the Zookeeper
                                      sub-paths for high availability mode

  Options for default mode:
     -D <property=value>             Allows specifying multiple generic
                                     configuration options. The available
                                     options can be found at
                                     https://ci.apache.org/projects/flink/flink-
                                     docs-stable/ops/config.html
     -m,--jobmanager <arg>           Address of the JobManager to which to
                                     connect. Use this flag to connect to a
                                     different JobManager than the one specified
                                     in the configuration. Attention: This
                                     option is respected only if the
                                     high-availability configuration is NONE.
     -z,--zookeeperNamespace <arg>   Namespace to create the Zookeeper sub-paths
                                     for high availability mode
```

## k8s config
Key	Default	Type	Description
external-resource.<resource_name>.kubernetes.config-key	(none)	String	If configured, Flink will add "resources.limits." and "resources.requests." to the main container of TaskExecutor and set the value to the value of external-resource.<resource_name>.amount.
kubernetes.client.io-pool.size	4	Integer	The size of the IO executor pool used by the Kubernetes client to execute blocking IO operations (e.g. start/stop TaskManager pods, update leader related ConfigMaps, etc.). Increasing the pool size allows to run more IO operations concurrently.
kubernetes.cluster-id	(none)	String	The cluster-id, which should be no more than 45 characters, is used for identifying a unique Flink cluster. The id must only contain lowercase alphanumeric characters and "-". The required format is [a-z]([-a-z0-9]*[a-z0-9]). If not set, the client will automatically generate it with a random ID.
kubernetes.config.file	(none)	String	The kubernetes config file will be used to create the client. The default is located at ~/.kube/config
kubernetes.container.image	The default value depends on the actually running version. In general it looks like "flink:<FLINK_VERSION>-scala_<SCALA_VERSION>"	String	Image to use for Flink containers. The specified image must be based upon the same Apache Flink and Scala versions as used by the application. Visit here for the official docker images provided by the Flink project. The Flink project also publishes docker images to apache/flink DockerHub repository.
kubernetes.container.image.pull-policy	IfNotPresent	EnumPossible values: [IfNotPresent, Always, Never]	The Kubernetes container image pull policy (IfNotPresent or Always or Never). The default policy is IfNotPresent to avoid putting pressure to image repository.
kubernetes.container.image.pull-secrets	(none)	List	A semicolon-separated list of the Kubernetes secrets used to access private image registries.
kubernetes.context	(none)	String	The desired context from your Kubernetes config file used to configure the Kubernetes client for interacting with the cluster. This could be helpful if one has multiple contexts configured and wants to administrate different Flink clusters on different Kubernetes clusters/contexts.
kubernetes.entry.path	"/docker-entrypoint.sh"	String	The entrypoint script of kubernetes in the image. It will be used as command for jobmanager and taskmanager container.
kubernetes.env.secretKeyRef	(none)	List	The user-specified secrets to set env variables in Flink container. The value should be in the form of env:FOO_ENV,secret:foo_secret,key:foo_key;env:BAR_ENV,secret:bar_secret,key:bar_key.
kubernetes.flink.conf.dir	"/opt/flink/conf"	String	The flink conf directory that will be mounted in pod. The flink-conf.yaml, log4j.properties, logback.xml in this path will be overwritten from config map.
kubernetes.flink.log.dir	"/opt/flink/log"	String	The directory that logs of jobmanager and taskmanager be saved in the pod.
kubernetes.hadoop.conf.config-map.name	(none)	String	Specify the name of an existing ConfigMap that contains custom Hadoop configuration to be mounted on the JobManager(s) and TaskManagers.
kubernetes.jobmanager.annotations	(none)	Map	The user-specified annotations that are set to the JobManager pod. The value could be in the form of a1:v1,a2:v2
kubernetes.jobmanager.cpu	1.0	Double	The number of cpu used by job manager
kubernetes.jobmanager.labels	(none)	Map	The labels to be set for JobManager pod. Specified as key:value pairs separated by commas. For example, version:alphav1,deploy:test.
kubernetes.jobmanager.node-selector	(none)	Map	The node selector to be set for JobManager pod. Specified as key:value pairs separated by commas. For example, environment:production,disk:ssd.
kubernetes.jobmanager.owner.reference	(none)	List	The user-specified Owner References to be set to the JobManager Deployment. When all the owner resources are deleted, the JobManager Deployment will be deleted automatically, which also deletes all the resources created by this Flink cluster. The value should be formatted as a semicolon-separated list of owner references, where each owner reference is a comma-separated list of key:value pairs. E.g., apiVersion:v1,blockOwnerDeletion:true,controller:true,kind:FlinkApplication,name:flink-app-name,uid:flink-app-uid;apiVersion:v1,kind:Deployment,name:deploy-name,uid:deploy-uid
kubernetes.jobmanager.service-account	"default"	String	Service account that is used by jobmanager within kubernetes cluster. The job manager uses this service account when requesting taskmanager pods from the API server. If not explicitly configured, config option 'kubernetes.service-account' will be used.
kubernetes.jobmanager.tolerations	(none)	List	The user-specified tolerations to be set to the JobManager pod. The value should be in the form of key:key1,operator:Equal,value:value1,effect:NoSchedule;key:key2,operator:Exists,effect:NoExecute,tolerationSeconds:6000
kubernetes.namespace	"default"	String	The namespace that will be used for running the jobmanager and taskmanager pods.
kubernetes.pod-template-file	(none)	String	Specify a local file that contains the pod template definition. It will be used to initialize the jobmanager and taskmanager pod. The main container should be defined with name 'flink-main-container'. Notice that this can be overwritten by config options 'kubernetes.pod-template-file.jobmanager' and 'kubernetes.pod-template-file.taskmanager' for jobmanager and taskmanager respectively.
kubernetes.pod-template-file.jobmanager	(none)	String	Specify a local file that contains the jobmanager pod template definition. It will be used to initialize the jobmanager pod. The main container should be defined with name 'flink-main-container'. If not explicitly configured, config option 'kubernetes.pod-template-file' will be used.
kubernetes.pod-template-file.taskmanager	(none)	String	Specify a local file that contains the taskmanager pod template definition. It will be used to initialize the taskmanager pod. The main container should be defined with name 'flink-main-container'. If not explicitly configured, config option 'kubernetes.pod-template-file' will be used.
kubernetes.rest-service.annotations	(none)	Map	The user-specified annotations that are set to the rest Service. The value should be in the form of a1:v1,a2:v2
kubernetes.rest-service.exposed.type	LoadBalancer	EnumPossible values: [ClusterIP, NodePort, LoadBalancer]	The exposed type of the rest service (ClusterIP or NodePort or LoadBalancer). The exposed rest service could be used to access the Flink’s Web UI and REST endpoint.
kubernetes.secrets	(none)	Map	The user-specified secrets that will be mounted into Flink container. The value should be in the form of foo:/opt/secrets-foo,bar:/opt/secrets-bar.
kubernetes.service-account	"default"	String	Service account that is used by jobmanager and taskmanager within kubernetes cluster. Notice that this can be overwritten by config options 'kubernetes.jobmanager.service-account' and 'kubernetes.taskmanager.service-account' for jobmanager and taskmanager respectively.
kubernetes.taskmanager.annotations	(none)	Map	The user-specified annotations that are set to the TaskManager pod. The value could be in the form of a1:v1,a2:v2
kubernetes.taskmanager.cpu	-1.0	Double	The number of cpu used by task manager. By default, the cpu is set to the number of slots per TaskManager
kubernetes.taskmanager.labels	(none)	Map	The labels to be set for TaskManager pods. Specified as key:value pairs separated by commas. For example, version:alphav1,deploy:test.
kubernetes.taskmanager.node-selector	(none)	Map	The node selector to be set for TaskManager pods. Specified as key:value pairs separated by commas. For example, environment:production,disk:ssd.
kubernetes.taskmanager.service-account	"default"	String	Service account that is used by taskmanager within kubernetes cluster. The task manager uses this service account when watching config maps on the API server to retrieve leader address of jobmanager and resourcemanager. If not explicitly configured, config option 'kubernetes.service-account' will be used.
kubernetes.taskmanager.tolerations	(none)	List	The user-specified tolerations to be set to the TaskManager pod. The value should be in the form of key:key1,operator:Equal,value:value1,effect:NoSchedule;key:key2,operator:Exists,effect:NoExecute,tolerationSeconds:6000
kubernetes.transactional-operation.max-retries	5	Integer	Defines the number of Kubernetes transactional operation retries before the client gives up. For example, FlinkKubeClient#checkAndUpdateConfigMap.