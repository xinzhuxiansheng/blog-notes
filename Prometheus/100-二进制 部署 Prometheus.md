## 二进制安装 Prometheus    

>CentOS7环境

### 下载 Prometheus安装包   
先从 `https://prometheus.io/download/` 下载 `prometheus-2.46.0.linux-amd64.tar.gz` 。   
```shell
wget https://github.com/prometheus/prometheus/releases/download/v2.46.0/prometheus-2.46.0.darwin-amd64.tar.gz   
```

### prometheus.yml 介绍   
```shell
tar -zxf prometheus-2.46.0.linux-amd64.tar.gz   
``` 

`prometheus.yml` 配置文件   
```yml
# my global config
global:
  scrape_interval: 15s # Set the scrape interval to every 15 seconds. Default is every 1 minute.
  evaluation_interval: 15s # Evaluate rules every 15 seconds. The default is every 1 minute.
  # scrape_timeout is set to the global default (10s).

# Alertmanager configuration
alerting:
  alertmanagers:
    - static_configs:
        - targets:
          # - alertmanager:9093

# Load rules once and periodically evaluate them according to the global 'evaluation_interval'.
rule_files:
  # - "first_rules.yml"
  # - "second_rules.yml"

# A scrape configuration containing exactly one endpoint to scrape:
# Here it's Prometheus itself.
scrape_configs:
  # The job name is added as a label `job=<job_name>` to any timeseries scraped from this config.
  - job_name: "prometheus"

    # metrics_path defaults to '/metrics'
    # scheme defaults to 'http'.

    static_configs:
      - targets: ["localhost:9090"]
```

**介绍**    
`prometheus.yml` 配置文件中的几个主要配置项，以及它们的功能：

1. **`global`**:
    这部分配置包含了全局参数，适用于整个 Prometheus 实例。

    - `scrape_interval`: 指定 Prometheus 从配置的目标收集数据的时间间隔。这是默认的采集间隔，但可以在特定的 `scrape_config` 中重写。
    
    - `evaluation_interval`: 指定 Prometheus 对警告和记录规则进行评估的时间间隔。

2. **`alerting`**:
    这部分用于配置 Prometheus 如何与 Alertmanager 集成。Alertmanager 负责管理警报，包括合并重复的警报、发送通知等。

    - `alertmanagers`: 定义 Prometheus 将发送警报的 Alertmanager 实例。在这个例子中，该部分被注释掉了，所以 Prometheus 不会发送警报。

3. **`rule_files`**:
    这部分指定了 Prometheus 需要加载的规则文件的位置。这些规则定义了基于当前指标数据应该触发的警报条件。
    
    - 在列表中指定的每个文件都包含了一组警报和/或记录规则。

4. **`scrape_configs`**:
    这是 Prometheus 采集指标数据所需的配置。对于每一个`scrape_configs`条目，Prometheus 定期从配置的目标收集指标。

    - `job_name`: 这是一个标识符，用于识别特定的采集任务。它在内部用于标签，帮助组织和过滤指标数据。
    
    - `static_configs`: 定义 Prometheus 将从哪些目标收集指标的配置。可以有多种方式定义目标，包括静态配置、服务发现等。在这个例子中，它是静态配置，Prometheus 从其自身的 `/metrics` 端点采集指标。

简而言之：  
- `global` 配置定义了 Prometheus 实例的全局行为。
- `alerting` 配置定义了如何与 Alertmanager 交互。
- `rule_files` 指定了哪些规则文件应该被加载和评估。
- `scrape_configs` 定义了 Prometheus 从哪里和如何收集指标数据。



### ./prometheus --help 介绍    
```shell
[root@yzhou prometheus-2.46.0.linux-amd64]# ./prometheus --help
usage: prometheus [<flags>]

The Prometheus monitoring server


Flags:
  -h, --[no-]help                Show context-sensitive help (also try --help-long and --help-man).
      --[no-]version             Show application version.
      --config.file="prometheus.yml"  
                                 Prometheus configuration file path.
      --web.listen-address="0.0.0.0:9090"  
                                 Address to listen on for UI, API, and telemetry.
      --web.config.file=""       [EXPERIMENTAL] Path to configuration file that can enable TLS or authentication.
      --web.read-timeout=5m      Maximum duration before timing out read of the request, and closing idle
                                 connections.
      --web.max-connections=512  Maximum number of simultaneous connections.
      --web.external-url=<URL>   The URL under which Prometheus is externally reachable (for example,
                                 if Prometheus is served via a reverse proxy). Used for generating relative
                                 and absolute links back to Prometheus itself. If the URL has a path portion,
                                 it will be used to prefix all HTTP endpoints served by Prometheus. If omitted,
                                 relevant URL components will be derived automatically.
      --web.route-prefix=<path>  Prefix for the internal routes of web endpoints. Defaults to path of
                                 --web.external-url.
      --web.user-assets=<path>   Path to static asset directory, available at /user.
      --[no-]web.enable-lifecycle  
                                 Enable shutdown and reload via HTTP request.
      --[no-]web.enable-admin-api  
                                 Enable API endpoints for admin control actions.
      --[no-]web.enable-remote-write-receiver  
                                 Enable API endpoint accepting remote write requests.
      --web.console.templates="consoles"  
                                 Path to the console template directory, available at /consoles.
      --web.console.libraries="console_libraries"  
                                 Path to the console library directory.
      --web.page-title="Prometheus Time Series Collection and Processing Server"  
                                 Document title of Prometheus instance.
      --web.cors.origin=".*"     Regex for CORS origin. It is fully anchored. Example:
                                 'https?://(domain1|domain2)\.com'
      --storage.tsdb.path="data/"  
                                 Base path for metrics storage. Use with server mode only.
      --storage.tsdb.retention=STORAGE.TSDB.RETENTION  
                                 [DEPRECATED] How long to retain samples in storage. This flag has been
                                 deprecated, use "storage.tsdb.retention.time" instead. Use with server mode only.
      --storage.tsdb.retention.time=STORAGE.TSDB.RETENTION.TIME  
                                 How long to retain samples in storage. When this flag is set it overrides
                                 "storage.tsdb.retention". If neither this flag nor "storage.tsdb.retention"
                                 nor "storage.tsdb.retention.size" is set, the retention time defaults to 15d.
                                 Units Supported: y, w, d, h, m, s, ms. Use with server mode only.
      --storage.tsdb.retention.size=STORAGE.TSDB.RETENTION.SIZE  
                                 Maximum number of bytes that can be stored for blocks. A unit is required,
                                 supported units: B, KB, MB, GB, TB, PB, EB. Ex: "512MB". Based on powers-of-2,
                                 so 1KB is 1024B. Use with server mode only.
      --[no-]storage.tsdb.no-lockfile  
                                 Do not create lockfile in data directory. Use with server mode only.
      --storage.tsdb.head-chunks-write-queue-size=0  
                                 Size of the queue through which head chunks are written to the disk to be
                                 m-mapped, 0 disables the queue completely. Experimental. Use with server mode
                                 only.
      --storage.agent.path="data-agent/"  
                                 Base path for metrics storage. Use with agent mode only.
      --[no-]storage.agent.wal-compression  
                                 Compress the agent WAL. Use with agent mode only.
      --storage.agent.retention.min-time=STORAGE.AGENT.RETENTION.MIN-TIME  
                                 Minimum age samples may be before being considered for deletion when the WAL is
                                 truncated Use with agent mode only.
      --storage.agent.retention.max-time=STORAGE.AGENT.RETENTION.MAX-TIME  
                                 Maximum age samples may be before being forcibly deleted when the WAL is
                                 truncated Use with agent mode only.
      --[no-]storage.agent.no-lockfile  
                                 Do not create lockfile in data directory. Use with agent mode only.
      --storage.remote.flush-deadline=<duration>  
                                 How long to wait flushing sample on shutdown or config reload.
      --storage.remote.read-sample-limit=5e7  
                                 Maximum overall number of samples to return via the remote read interface,
                                 in a single query. 0 means no limit. This limit is ignored for streamed response
                                 types. Use with server mode only.
      --storage.remote.read-concurrent-limit=10  
                                 Maximum number of concurrent remote read calls. 0 means no limit. Use with server
                                 mode only.
      --storage.remote.read-max-bytes-in-frame=1048576  
                                 Maximum number of bytes in a single frame for streaming remote read response
                                 types before marshalling. Note that client might have limit on frame size as
                                 well. 1MB as recommended by protobuf by default. Use with server mode only.
      --rules.alert.for-outage-tolerance=1h  
                                 Max time to tolerate prometheus outage for restoring "for" state of alert.
                                 Use with server mode only.
      --rules.alert.for-grace-period=10m  
                                 Minimum duration between alert and restored "for" state. This is maintained only
                                 for alerts with configured "for" time greater than grace period. Use with server
                                 mode only.
      --rules.alert.resend-delay=1m  
                                 Minimum amount of time to wait before resending an alert to Alertmanager.
                                 Use with server mode only.
      --alertmanager.notification-queue-capacity=10000  
                                 The capacity of the queue for pending Alertmanager notifications. Use with server
                                 mode only.
      --query.lookback-delta=5m  The maximum lookback duration for retrieving metrics during expression
                                 evaluations and federation. Use with server mode only.
      --query.timeout=2m         Maximum time a query may take before being aborted. Use with server mode only.
      --query.max-concurrency=20  
                                 Maximum number of queries executed concurrently. Use with server mode only.
      --query.max-samples=50000000  
                                 Maximum number of samples a single query can load into memory. Note that queries
                                 will fail if they try to load more samples than this into memory, so this also
                                 limits the number of samples a query can return. Use with server mode only.
      --enable-feature= ...      Comma separated feature names to enable. Valid options: agent, exemplar-storage,
                                 expand-external-labels, memory-snapshot-on-shutdown, promql-at-modifier,
                                 promql-negative-offset, promql-per-step-stats, remote-write-receiver
                                 (DEPRECATED), extra-scrape-metrics, new-service-discovery-manager,
                                 auto-gomaxprocs, no-default-scrape-port, native-histograms. See
                                 https://prometheus.io/docs/prometheus/latest/feature_flags/ for more details.
      --log.level=info           Only log messages with the given severity or above. One of: [debug, info, warn,
                                 error]
      --log.format=logfmt        Output format of log messages. One of: [logfmt, json]
```

**介绍**    
Prometheus 的命令行参数帮助手册的简要解释：

1. **General Flags**:
    - `-h, --[no-]help`: 显示上下文相关的帮助。
    - `--[no-]version`: 显示应用版本。

2. **Configuration Flags**:
    - `--config.file`: 指定 Prometheus 的配置文件路径，默认为 "prometheus.yml"。

3. **Web Flags**:
    - `--web.listen-address`: 定义 Prometheus UI、API 和遥测的监听地址。
    - `--web.config.file`: 可以启用 TLS 或身份验证的配置文件路径（实验性质）。
    - `--web.read-timeout`: 请求的最大读取超时时间。
    - `--web.max-connections`: 同时的最大连接数。
    - `--web.external-url`: Prometheus 对外部可达的 URL。
    - `--web.route-prefix`: 内部 web 端点的路径前缀。
    - `--web.user-assets`: 用户静态资源的路径。
    - `--[no-]web.enable-lifecycle`: 通过 HTTP 请求启用关闭和重新加载。
    - `--[no-]web.enable-admin-api`: 启用管理员控制动作的 API 端点。
    - `--[no-]web.enable-remote-write-receiver`: 启用接受远程写入请求的 API 端点。
    - `--web.console.templates`: 控制台模板目录的路径。
    - `--web.console.libraries`: 控制台库目录的路径。
    - `--web.page-title`: Prometheus 实例的文档标题。
    - `--web.cors.origin`: CORS 源的正则表达式。

4. **Storage Flags**:
    - `--storage.tsdb.path`: 指定存储指标的基本路径。
    - `--storage.tsdb.retention`: 存储中的样本保留时间（已弃用）。
    - `--storage.tsdb.retention.time`: 在存储中保留样本的时间。
    - `--storage.tsdb.retention.size`: 可以为块存储的最大字节数。
    - `--[no-]storage.tsdb.no-lockfile`: 在数据目录中不创建锁文件。
    - `--storage.tsdb.head-chunks-write-queue-size`: 头块写入磁盘的队列大小。
    - `--storage.agent.path`: 指定代理模式下的存储基本路径。
    - `--[no-]storage.agent.wal-compression`: 压缩代理 WAL。
    - `--storage.remote.flush-deadline`: 关闭或重新加载配置时，等待清空样本的时间。
    - `--storage.remote.read-sample-limit`: 通过远程读取接口返回的最大样本总数。
    - `--storage.remote.read-concurrent-limit`: 并发远程读取调用的最大数量。
    - `--storage.remote.read-max-bytes-in-frame`: 在封送之前流式远程读取响应类型的单个帧中的最大字节数。

5. **Rule Flags**:
    - `--rules.alert.for-outage-tolerance`: 最大的宽容 Prometheus 停机时间，以恢复警报的 "for" 状态。
    - `--rules.alert.for-grace-period`: 警报和恢复 "for" 状态之间的最小持续时间。
    - `--rules.alert.resend-delay`: 向 Alertmanager 重新发送警报之前的最小等待时间。
    - `--alertmanager.notification-queue-capacity`: 待处理的 Alertmanager 通知的队列容量。

6. **Query Flags**:
    - `--query.lookback-delta`: 在表达式评估和联合期间检索指标的最大回溯持续时间。
    - `--query.timeout`: 查询可能需要的最长时间，超过后将被中止。
    - `--query.max-concurrency`: 可并发执行的查询的最大数量。
    - `--query.max-samples`: 单个查询可以加载到内存中的最大样本数。

7. **Feature Flags**:
    - `--enable-feature`: 启用的特性名，用逗号分隔。

8. **Logging Flags**:
    - `--log.level`: 指定日志的严重性级别。
    - `--log.format`: 指定日志消息的输出格式。  


### Prometheus 启动 
在 Prometheus 启动时，可指定两种不同参数，一是通过 `prometheus.yml`,二是通过`--xxx` 命令行参数（参考 --help） 。

```shell
# --storage.tsdb.path 建议指定
# --web.max-connections 建议修改
# --storage.tsdb.retention.time 保留时长
./prometheus
```


refer   
1.https://prometheus.io/docs/introduction/first_steps/  
2.https://prometheus.io/download/   
