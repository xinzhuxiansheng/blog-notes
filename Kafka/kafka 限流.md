# kafka 配额

## 介绍
Kafka集群可以对请求强制执行配额，以控制客户端使用的代理资源。卡夫卡经纪人可以为共享配额的每组客户强制执行两种类型的客户配额：

* 网络带宽配额定义字节速率阈值（从0.9开始）
* 请求速率配额将CPU使用率阈值定义为网络和I / O线程的百分比（从0.11开始）

## 为什么需要配额？
生产者和消费者有可能生产/消费大量数据或以非常高的速率生成请求，从而垄断kafka broker资源，导致网络饱和，并且通常阻塞其他客户端和broker通信，kafka 配额可以避免这些问题，并且在大型多租户群集中尤为重要，在该群集中，一小组行为​​不佳的客户端可能会降低行为良好的客户端的用户体验。实际上，当将Kafka作为服务运行时，甚至可以根据约定的合同强制执行A​​PI限制。

## 客户群
Kafka客户端的身份是用户主体，它代表安全集群中已通过身份验证的用户。在支持未经身份验证的客户端的群集中，用户主体是由代理使用可配置的方式选择的未经身份验证的用户的分组PrincipalBuilder。客户端ID是客户端的逻辑分组，其中客户端应用程序选择了有意义的名称。元组（用户，客户端ID）定义了共享用户主体和客户端ID的客户端安全逻辑组。
配额可以应用于（用户，客户端ID），用户或客户端ID组。对于给定的连接，将应用与该连接匹配的最具体的配额。配额组的所有连接共享为该组配置的配额。例如，如果（user =“ test-user”，client-id =“ test-client”）的生产配额为10MB /秒，则该配额在用户“ test-user”的所有生产者实例中与客户端共享- id“测试客户端”。

## 配额配置
可以为（用户，客户端ID），用户和客户端ID组定义配额配置。可以在需要更高（甚至更低）配额的任何配额级别上覆盖默认配额。该机制类似于按主题的日志配置替代。用户和（用户，客户端ID）配额替代写入/ config / users下的ZooKeeper，而客户端ID配额替代写入/ config / clients下。所有代理均会读取这些替代，并立即生效。这使我们可以更改配额，而不必滚动重启整个集群。有关详细信息，请参见此处。每个组的默认配额也可以使用相同的机制动态更新。

## 配额配置的优先顺序为：
```shell
/ config /用户/ <用户> /客户端/ <客户端ID>
/ config /用户/ <用户> /客户端/ <默认>
/ config / users / <用户>
/ config /用户/ <默认> /客户端/ <客户端ID>
/ config /用户/ <默认> /客户端/ <默认>
/ config / users / <默认>
/ config / clients / <客户端ID>
/ config / clients / <默认>
```
代理属性（quota.producer.default，quota.consumer.default）也可以用于为客户端ID组设置网络带宽配额的默认值。这些属性已被弃用，并将在以后的版本中删除。可以在Zookeeper中设置client-id的默认配额，类似于其他配额替代和默认配额。
网络带宽配额
网络带宽配额定义为共享配额的每组客户端的字节速率阈值。默认情况下，每个唯一的客户端组都会收到由群集配置的固定配额（以字节/秒为单位）。此配额是根据每个经纪人定义的。每个客户端组在限制客户端之前，每个代理最多可以发布/获取X个字节/秒。

## 请求率配额
请求速率配额定义为客户端可以在配额窗口内利用每个代理的请求处理程序I / O线程和网络线程的时间百分比。n％的配额表示 一个线程的n％，因此配额超出（（num.io.threads + num.network.threads）* 100）％的总容量。每组客户可使用的总百分比最高为n％在配额窗口中跨所有I / O和网络线程访问。由于分配给I / O和网络线程的线程数通常基于代理主机上可用的内核数，因此请求率配额表示共享配额的每组客户端可以使用的CPU的总百分比。




./kafka-configs.sh --zookeeper localhost:2181 --alter --add-config 'producer_byte_rate=20971520' --entity-type clients --entity-default
Completed Updating config for entity: default client-id.


./kafka-configs.sh --zookeeper localhost:2181 --alter --add-config 'producer_byte_rate=1048576,consumer_byte_rate=1048576'  \
--entity-type   clients  --entity-name dc



摘自 https://www.jianshu.com/p/2dda6d98cdfa