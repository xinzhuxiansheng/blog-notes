## Flink SQL Connectors Upert Kafka 

### 介绍    
upsert-kafka 是对 Kafka Connector 进行了扩展, 提供了对 `更新/删除`操作的支持，其实主要是因为它引入了 changelog 中的 `RowKind` 类型, 然后借助于 State 实现了更新和删除功能，它主要是利用Kafka 数据中的 `key`作为主键, 来确定一条数据是应该作为插入，删除还是更新来处理。 

upert-kafka connector 允许用户以 upsert 的方式, 从kafka 中读取读数据或将数据写入 kafka 中，也就是说它即可以作为 source 使用也可以作为 sink 使用，并且提供了与现有的 kafka connector 相同的基本功能, 因为他们两者之间复用了大部分代码;       
* 当作为 source 使用时： upsert-kafka 会产生 changelog 数据流，每条数据都被表示一个更新或删除事件, 准确一点来说，如果数据中的key 是第一次出现，则视为新增（INSERT）操作。   

* 当作为 sink 使用时：upsert-kafka 会消费一个 changelog 数据流, 它会将 INSERT（+I）和 UPDATE_AFTER (+U) 类型的数据作为新增数据写入, 并且将 Delete（-D）类型的数据以 value 为 null的形式写入kafka 中，这是 upsert-kafka 的基本情况。      

### 注意事项    

那我们在工作中使用 upsert-kafka的时候，有一些注意事项:  
* 1.我们需要在建表语句中指定主键(可以是联合主键)   
    在指定主键的时候，可以根据需求指定一个或者多个字段, 那么多个字段的话就是联合主键            

* 2.针对主键字段, 建议在建表语句中添加 NOT NULL 标识，因为主键是非空的。      

* 3.需要同时指定 `key.format` 和 `value.format`，因为数据中包含 key，之前使用 format 属于 value.format的 简写形式。 

* 4.作为 sink使用时，不能指定 `sink.parttioner`， 因为它会根据key的值对数据进行分区,从而保证相同的key 的数据是有序的，因此相同 key 的新增、更新和删除操作都会进入同一个分区。     

* 5.作为 source使用时，不能指定 `scan.startup.mode`, 因为默认会从 earliest 开始消费数据, 就是消费最早的数据，目前不支持自定义消费策略，这是 upsert-kafka的一些注意事项。    


