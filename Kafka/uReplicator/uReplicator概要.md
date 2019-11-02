**正文**

## 解决问题背景

第一， 不可以控制它什么时候发生，比如增加或减少 Topic 时可能会触发 Rebalance，增加或减少 Worker 时也可能会触发 Rebalance；

第二， Rebalance 的过程中所有复制都会停止，这段时间任何数据都无法流入到下流，这非常不好。另外，在 Rebalance 结束之后，会留下这段时间的 backlog，MirrorMaker 会有一个巨大的 traffic 去 Catch-up 这些 backlog，这样的 spark 对 Destination 的 broker 也有非常大的影响。

第三， 当时 MirrorMaker 使用了一个静态的 Topic List，需要在 MirrorMaker 集群启动的时候读取，想要增加或者减少一个 Topic 时，需要更新这个静态的文件列表，再重启整个 MirrorMaker 集群。每次都要维护静态文件很不方便，而且在重启集群的时候又会导致触发 Rebalance 的问题。

第四， 当时的 MirrorMaker 无法控制 Consumer Commit offset 的时间，经常出现把数据发送到了 Kafka 的 producer 里，但是还没有实际送到 Destination 的情况。这个时候，Consumer 就 Commit 了 offset，在这种情况下，如果这一个 MirrorMaker 的 Worker Crash 了，恢复以后就不会再去重复复制这些信息，那么这些信息就彻底丢失了。

最后，因为 MirrorMaker 使用了一些静态的配置文件，当这些文件没有被同步到所有 Worker 上的机器时，启动时就会发生各种问题。

## uReplicator要做到

    Stable replication

希望这个平台有稳定复制的能力，如果有 Topic 或者 Worker 在复制过程发生变动，复制行为不应该受到影响，还可以继续稳定复制。

    Simple operations

希望可以方便地增加或者删除 Topic，而不需要维护静态文件，不需要在每次改变 Topic 的时候重启整个集群。

    High throughput

希望这个平台有非常好的性能，以满足 Uber 数据量的增长。

    No data loss

希望在复制的过程中可以保证没有数据丢失。

    Auditing

和上一个相关，希望有机制可以检测是否真的发生了数据丢失。


## Non-federation Mode
Controller,Worker,Route

## Federation Mode
Manager,Controller,Worker,Route

Manager










## 思考点
1. 源集群和目的集群的kafka版本是否一致。目前uReplicator可以支持源集群0.8.2.1向高版本的目的集群进行数据同步。代码中0.8.2.1分支是支持源版本0.8.2.1，0.10.2.1分支支持源集群0.10.2.1
2. config目录下可以对consumer，producer，helix进行配置，consumer和producer的配置与在mirror maker对其的配置差不多。注意：目前每个consumer thread 只支持一个KafkaStream。详情见 How to set the number of worker&#x27;s threads, just like mirror maker&#x27;s num_streams,num_producers? · Issue #40 · uber/uReplicator
3. toipic映射问题：一般源集群和目的集群进行数据同步的topic命名都一样，如果不同，则需在config目录下mapping配置中声明源集群的topic名字和目的集群的topic名字。4.可以动态的添加topic和uRelicator worker,不过在这一小段时间内，uReplicator会rebalance，有暂时的不稳定。

4.目前基于中间件uReplicator实现了Kafka集群间的迁移复制，可以实现跨区，跨云的kafka集群间复制同步，也可以实现kafka集群的冷热互备架构。在实现集群间同步以后，需要解决一个很重要的问题:如何从主集群切换到备份集群？ 最需要解决相关的应用在备份集群的开始消费位移问题。







## 参考链接
https://www.infoq.cn/article/9_eHGBHWJ0j36sWCry35?utm_source=related_read&utm_medium=article

