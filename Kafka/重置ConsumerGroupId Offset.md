## 根据Timestamp重置ConsumerGroupId的Offset

根据时间重置GroupId的Offset，需要将设置GroupId停止消费任何数据  !!!

shell命令:
```shell
./kafka-consumer-groups.sh --bootstrap-server xx.xx.100.16:9093 --command-config consumer.properties --group yzhougid2020101101 --reset-offsets --topic yzhoutpA001 --to-datetime 2020-10-10T00:00:00.000 --execute
```

执行完，观察当前GroupId的"Consumer Offset" 是否有改变。

1. GroupId有几种状态？ 
```java
/**
 * The consumer group state.
 */
public enum ConsumerGroupState {
    UNKNOWN("Unknown"),
    PREPARING_REBALANCE("PreparingRebalance"),
    COMPLETING_REBALANCE("CompletingRebalance"),
    STABLE("Stable"),
    DEAD("Dead"),    //新的GroupId 
    EMPTY("Empty");  //消费过，但停止
}
```

2. 
