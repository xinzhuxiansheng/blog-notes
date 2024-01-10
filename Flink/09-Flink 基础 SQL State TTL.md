## Flink SQL State TTL  

### 介绍    
State TLL 表示是给状态设置过期时间的,在之前介绍Flink SQL 双流 JOIN 的时候提到了需要给数据流对应的 State 设置合适的TTL 策略，防止 State过大。 因为普通的双流 Join 是无界数据流Join，两个数据流中的数据会一直存储在 State 中，就算后期再也用不到了,它也会一直存在于 State 中，当任务运行几天之后，内存可能就扛不住了, 因为 State 的数据默认是存储在内存中的，当然也可以选择使用 rocksdb,但是 State 中存储的数据量大了之后，后期使用的时候查询效率肯定也会降低。所以我们需要给 State 设置合适的 TTL 过期时间。     

针对时间区间 Join，窗口 Join来说，他们内部会有从 State 中删除数据的逻辑,但是在特殊情况下可能会导致 State 变的很大，所以保险起见,建议还是给所有类型的双流 Join 都设置 State TTL。    

>注意： 针对 FLink SQL 的单流处理也会用到状态。 例如： SQL 中用到了分组聚合那么就会在状态中维护之前收到的历史数据，所以也需要设置合适的 State TTL。         

### Flink SQL 中设置 State TTL 
这块建议参考 Flink 源码中的 `TableConfigTest#testSetAndGetIdleStateRetention` 单元测试
```java
@Test
public void testSetAndGetIdleStateRetention() {
    configuration.setString("table.exec.state.ttl", "1 h");
    configByConfiguration.addConfiguration(configuration);
    configByMethod.setIdleStateRetention(Duration.ofHours(1));

    assertEquals(Duration.ofHours(1), configByMethod.getIdleStateRetention());
    assertEquals(Duration.ofSeconds(1), configByConfiguration.getIdleStateRetention());
}
``` 

根据上面代码可知， table.exec.state.ttl = 1h，可设置 state ttl 过期时间。       


>注意：如果没有设置 State TTL, 不管作业过了多长时间是都能匹配到的，只要 State中的数据一直被频繁的使用，那么它是不会 State TTL 策略删除的, 假设 你设置的 State TTL 策略是 30s，只要这条数据在 30s 之内被使用了，那么它的生存时间会被延长,所以后期这条数据只要被频繁使用，它就不会被删除。    


