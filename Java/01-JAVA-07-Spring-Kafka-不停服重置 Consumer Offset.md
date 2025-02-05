# JAVA - Spring-kafka - 不停服重置 Consumer Offset     

>Kafka version: 3.6.0 

>该篇 Blog 内容更像是贴在公司 Confluence Kafka 工作空间 doc，毕竟让别人知晓怎么用，也是我们的责任。                    

## 背景     
在 Kafka consumer 消费数据场景中，消费方会提出 `reset consumer offset`需求，他们希望`跳过`或者`回溯` 某部分的数据，当然这部分大多时候都在处理数据异常后逻辑。这的确很难拒绝。     

>对于`冷数据`的回溯，在数据量较大情况必然会造成 `pagecache 污染`，从而影响到 Kafka 读写性能。（该篇 Blog 暂时不讨论性能，当然贴在Confluence Kafka 工作空间 doc 最好标明一个说明 `应尽可能避免在高峰期间执行 reset consumer offset相关操作`）。    

>知识点：重置消费组的 offset，前提是必须先停止消费动作。   

## Kafka 重置 Offset 相关命令  

### 查看消费组列表
```bash 
[root@vm01 bin]# ./kafka-consumer-groups.sh --bootstrap-server 192.168.0.201:9092 --list
gid012401
groupA
```

### 查看某个消费者信息
```bash
[root@vm01 bin]# ./kafka-consumer-groups.sh --describe --group gid012401 --bootstrap-server 192.168.0.201:9092

Consumer group 'gid012401' has no active members.

GROUP           TOPIC           PARTITION  CURRENT-OFFSET  LOG-END-OFFSET  LAG             CONSUMER-ID     HOST            CLIENT-ID
gid012401       yzhoutpjson01   0          11              11              0               -               -               -
```

>此时特别注意：控制台打印出 `Consumer group 'gid012401' has no active members.` 表示当前消费组中并没有消费者正在消费数据，这里强调的是 consumer 与 broker 建立连接，即使 consumer 与 broker 建立连接但没有消费到数据也算是 active members。 

要是有消费者正在消费，控制会打印什么样呢？ 如下所示：           
```bash
[root@vm01 bin]# ./kafka-consumer-groups.sh --describe --group gid012401 --bootstrap-server 192.168.0.201:9092

GROUP           TOPIC           PARTITION  CURRENT-OFFSET  LOG-END-OFFSET  LAG             CONSUMER-ID                                               HOST            CLIENT-ID
gid012401       yzhoutpjson01   0          11              11              0               consumer-gid012401-1-b5398cc2-5609-448e-8330-48ce9c79f06c /192.168.0.2    consumer-gid012401-1
```

### 修改 consumer offset  
刚才通过上面的示例查看 `CURRENT-OFFSET` 为 11，若要是修改成 8，会怎么样？
```bash
[root@vm01 bin]# ./kafka-consumer-groups.sh --bootstrap-server 192.168.0.201:9092 --group gid012401 --reset-offsets --topic yzhoutpjson01 --to-offset 8 --execute

Error: Assignments can only be reset if the group 'gid012401' is inactive, but the current state is Stable.

GROUP                          TOPIC                          PARTITION  NEW-OFFSET
```

若当前消费组存在 active members，重置 consumer offset 会提示`Assignments can only be reset if the group 'gid012401' is inactive`, 所以在重置时，需要将消费组下的consumer 都关闭。  

下面是关于`--reset-offsets`参数介绍：  
```bash
--reset-offsets                         Reset offsets of consumer group.
                                          Supports one consumer group at the
                                          time, and instances should be
                                          inactive
                                        Has 2 execution options: --dry-run
                                          (the default) to plan which offsets
                                          to reset, and --execute to update
                                          the offsets. Additionally, the --
                                          export option is used to export the
                                          results to a CSV format.
                                        You must choose one of the following
                                          reset specifications: --to-datetime,
                                          --by-duration, --to-earliest, --to-
                                          latest, --shift-by, --from-file, --
                                          to-current, --to-offset.
                                        To define the scope use --all-topics
                                          or --topic. One scope must be
                                          specified unless you use '--from-
                                          file'.

```

常见重置参数解释：          
```bash
--to-earliest：设置到最早位移处
--to-latest：设置到最新处，也就是主题分区HW的位置
--to-offset NUM：指定具体的位移位置
--shift-by NUM：根据偏移量数值相对移动（正数向前，负数向后）
--to-datetime：将偏移量重置到指定的日期时间  
--by-duration： 回退到多长时间   
```

>所以，成功 reset consumer offset的前提是停止当前消费组下的 consume 行为。    

那要是不停服务重置 consumer offset 有什么`便捷的`实现呢？下面介绍如何使用 `spring-kafka` 类库实现`不停服务重置 Consumer Offset`。           

## 使用 spring-kafka 类库实现 不停服务重置 Consumer Offset   



refer           
1.https://spring.io/projects/spring-kafka           
2.