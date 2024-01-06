

1.如果返回的是 PojoType，则对象会被 Flink 视为 POJO对象，序列化使用的是 POjoSerializer，并且序列化的时候，就只会序列化字段，方法不会被序列化，反序列化的时候，会根据公有的无参构造方法，去构造对象        

2.如果返回的是 GenericType，则对象不会被 Flink 视为 POJO对象，序列化使用的就是 kryo
