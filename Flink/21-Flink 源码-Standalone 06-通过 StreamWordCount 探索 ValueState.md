# Flink 源码 - Standalone - 通过 StreamWordCount 探索 ValueState   

## 引言  
在之前 Blog “Flink 源码-Standalone - 通过 StreamWordCount 探索 State & Checkpoint”的`StreamWordCount`案例中介绍了sum 算子其内部会自动使用 State存储统计后的结果，这对于我们来说是有些不透明的，但 Flink 也提供了用于编写有状态的 API。接下来，基于`StreamWordCount`示例代码改造来说明显示调用`Managed State`的处理逻辑。        

> 注意：该篇 Blog 中 Job 运行环境依赖 `之前 Blog “Flink 源码-Standalone - 通过 StreamWordCount 探索 State & Checkpoint”` 的 Standalone 集群配置。

## StreamWordCountUseState 示例代码   
`StreamWordCountUseState.java` 是以之前 Blog “Flink 源码-Standalone - 通过 StreamWordCount 探索 State & Checkpoint”的`StreamWordCount.java`为基础做了修改，继承了`KeyedProcessFunction`，并且使用`ValueState`存储 Word Count 的统计结果。

```java
public class StreamWordCountUseState {
    private static Logger logger = LoggerFactory.getLogger(com.yzhou.blog.wordcount.StreamWordCount.class);

    public static void main(String[] args) throws Exception {
        // 1. 创建流式执行环境
//        StreamExecutionEnvironment env = StreamExecutionEnvironment
//                .createLocalEnvironmentWithWebUI(new Configuration());
        StreamExecutionEnvironment env = StreamExecutionEnvironment
                .getExecutionEnvironment(new Configuration());
        env.setRestartStrategy(RestartStrategies
                .fixedDelayRestart(3, Time.of(10, TimeUnit.SECONDS)));
        // 2. Socket 读取  nc -lk 7777
        DataStreamSource<String> lineDSS = env
                .socketTextStream("localhost", 7777);

        // 3. 转换数据格式
        SingleOutputStreamOperator<Tuple2<String, Long>> wordAndOne = lineDSS
                .flatMap(
                        (String line, Collector<String> words) -> {
                            Arrays.stream(line.split(" ")).forEach(words::collect);
                        }
                )
                .returns(Types.STRING)
                .map(word -> Tuple2.of(word, 1L))
                .returns(Types.TUPLE(Types.STRING, Types.LONG)).setParallelism(2);

        // 4. 分组
        KeyedStream<Tuple2<String, Long>, String> wordAndOneKS = wordAndOne
                .keyBy(t -> t.f0);
        // 5. 求和
        SingleOutputStreamOperator<Tuple2<String, Long>> result = wordAndOneKS
                .process(new WordCountProcessFunction())
                .setParallelism(1).uid("wc-sum");

        // 6. 打印
        result.print();
        logger.info(result.toString());
        // 7. 执行
        env.execute();
    }

    public static class WordCountProcessFunction extends KeyedProcessFunction<String, Tuple2<String, Long>, Tuple2<String, Long>> {
        private ValueState<Long> countState;

        @Override
        public void open(org.apache.flink.configuration.Configuration parameters) throws Exception {
            ValueStateDescriptor<Long> descriptor = new ValueStateDescriptor<>(
                    "wordCountState", // 状态的名称
                    Types.LONG // 状态存储的数据类型
            );
            countState = getRuntimeContext().getState(descriptor);
        }

        @Override
        public void processElement(
                Tuple2<String, Long> value,
                Context ctx,
                Collector<Tuple2<String, Long>> out) throws Exception {

            // 获取当前单词的计数状态
            Long currentCount = countState.value();

            // 初始化状态
            if (currentCount == null) {
                currentCount = 0L;
            }

            // 自增并更新状态
            currentCount += value.f1;
            countState.update(currentCount);

            // 输出当前单词的计数结果
            out.collect(new Tuple2<>(value.f0, currentCount));
        }
    }
}
```

### 测试统计结果  
1）终端执行 `nc -lk 7777`，发送四次`my name is yzhou`内容     
2）通过 Flink WEB UI 提交 StreamWordCount Job     
3）输入测试数据，最后 word count 的统计结果可在 Task Managers Stdout Log 页面查看，如下图所示：    
![keyedstate01](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate01.png)    

### 手动触发 Checkpoint    
手动触发的 REST API URL： `http://<jobmanager>:8081/jobs/<job_id>/checkpoints`      
```shell
curl --location --request POST 'http://192.168.0.201:8081/jobs/7efe25434fac95d405d15fa834ee378a/checkpoints'
```

观察 Checkpoint 的存储目录。     
**Checkpoint Path:**     

```bash
[root@vm01 7efe25434fac95d405d15fa834ee378a]# tree .
.
├── chk-1
│   └── _metadata
├── chk-2
│   └── _metadata
├── shared
└── taskowned
```

### 从 Checkpoint 恢复作业，验证 ValueState 是否恢复存储值   
在 `Submit New Job`添加 Checkpoint Path`/root/yzhou/flink/flink1172/cppath/7efe25434fac95d405d15fa834ee378a/chk-2` 再提交。     
![keyedstate02](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate02.png)     

同样发送`my name is yzhou`,可在 TaskManager 的 `Stdout log`看到以下统计结果：        
```bash
(name,5)
(is,5)
(my,5)
(yzhou,5)
```

**小结**   
`StreamWordCountUseState.java`的测试结果与 Blog "Flink 源码-Standalone - 通过 StreamWordCount 探索 State & Checkpoint"的`StreamWordCount.java`的测试结果是一致的，也符合预期。    

基于上面的案例，不知道你会不会有一些疑问：      
- 第一个疑问是：`KeyedProcessFunction` 是什么？ 它与 `ProcessFunction`有什么区别？      
- 第二个疑问是：Word Count 的统计结果是 K，V 结构，可在`WordCountProcessFunction#processElement()`方法中，使用`Long currentCount = countState.value();`方法获取计数值，调用`countState.update(currentCount);`更新计数值，那它是如何知道 K 是哪个呢？例如，过来的数据是`my`,`name`,`is`,`yzhou`, `countState`是怎么知道的？      

若你存在其他的疑问，可留言给我，一起探索其他疑问点。        

> 其实接下来，才是该篇 Blog 的重点，上面的示例提出一些思考点将读者带入到下面内容中来。        
 
## ProcessFunction  
> 非常建议在没有开始阅读下面介绍之前，先阅读`ProcessFunction`文档（https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/datastream/operators/process_function/）。

```bash
 The ProcessFunction

The ProcessFunction is a low-level stream processing operation, giving access to the basic building blocks of all (acyclic) streaming applications:

    events (stream elements)
    state (fault-tolerant, consistent, only on keyed stream)
    timers (event time and processing time, only on keyed stream)

The ProcessFunction can be thought of as a FlatMapFunction with access to keyed state and timers. It handles events by being invoked for each event received in the input stream(s).

For fault-tolerant state, the ProcessFunction gives access to Flink’s keyed state, accessible via the RuntimeContext, similar to the way other stateful functions can access keyed state.

The timers allow applications to react to changes in processing time and in event time. Every call to the function processElement(...) gets a Context object which gives access to the element’s event time timestamp, and to the TimerService. The TimerService can be used to register callbacks for future event-/processing-time instants. With event-time timers, the onTimer(...) method is called when the current watermark is advanced up to or beyond the timestamp of the timer, while with processing-time timers, onTimer(...) is called when wall clock time reaches the specified time. During that call, all states are again scoped to the key with which the timer was created, allowing timers to manipulate keyed state.

    If you want to access keyed state and timers you have to apply the ProcessFunction on a keyed stream:

stream.keyBy(...).process(new MyProcessFunction());
```   

根据官网解释：ProcessFunction 是一种低级流处理操作（这里的`低级`体现于它是通用函数，在实际使用时，你需要扩展 ProcessFunction类并重写它的方法），但同时也给出了它的使用边界，例如它的 state、timers only on keyed stream(仅在Keyed Stream 生效)。 要是没有理解，大家可看`StreamWordCountUseState`示例中的代码片段：                  
```java
// 4. 分组
KeyedStream<Tuple2<String, Long>, String> wordAndOneKS = wordAndOne
        .keyBy(t -> t.f0);
// 5. 求和
SingleOutputStreamOperator<Tuple2<String, Long>> result = wordAndOneKS
        .process(new WordCountProcessFunction())
        .setParallelism(1).uid("wc-sum");
```

`.process()`调用链路是在类型为 KeyedStram 的`wordAndOneKS`变量后面，注意该示例中的`WordCountProcessFunction`并没有重写`onTimer()`,所以暂未涉及到 timers特性。下面是 ProcessFunction的类图：          
![keyedstate03](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate03.png)     

`ProcessFunction` 提供了以下两个方法:    
### processElement():    
1. void processElement(I value, Context ctx,Collector<O>out)   
该方法用于处理输入数据，每输入一条数据就调用一次该方法，然后不输出数据或者输出多条数据。从ProcessFunction提供的processElement()方法可以看出，ProcessFunction就是一个增强版的FlatMapFunction，两者处理输入数据的逻辑是相同的。ProcessFunction的入参value和out分别代表输入数据以及输出数据的收集器，入参ctx代表运行时上下文Context。  

内部 Context抽象类提供了以下3个方法用于访问数据的时间戳、注册和删除定时器以及进行旁路数据处理：         
![keyedstate04](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate04.png)       
* timestamp()：用于访问数据的时间戳，单位为ms。数据的时间戳是指StreamRecord中timestamp字段保存的时间戳。当使用Watermark生成策略获取数据时间戳时，Flink会将数据时间戳保存在StreamRecord的timestamp字段中。注意，如果没有使用Watermark生成策略来获取数据时间戳，那么该方法的返回值为null，而在处理时间语义下，通常不会用到Watermark生成策略，因此方法返回值也为null。         
* timerService()：该方法会返回TimerService（定时器服务）,TimerService接口的定义和窗口触发器Trigger抽象类中的TriggerContext接口提供的方法一样，两者都提供了获取SubTask时钟、注册定时器和删除定时器的功能。其中longcurrentProcessingTime()方法用于获取当前SubTask的处理时间时钟的时间戳，long currentWatermark()方法用于获取当前SubTask的事件时间时钟的时间戳，两个方法返回值的单位都为ms。          
* output(): 用于将数据输出到旁路中。入参outputTag是用于标记旁路的标签，value是需要输出到旁路的数据。ProcessOperator在执行旁路输出时，相当于给每一条输出到旁路的数据打了一个标签，当下游使用到某个标签的旁路数据时，ProcessOperator会直接将这个标签下的所有数据发给下游算子，而不是将所有的数据发送到下游算子，这样可以有效减少算子间传输的数据量。     

### onTimer():  
该方法会在定时器触发时调用。入参 timestamp 代表当前触发的定时器的时间戳，out代表输出数据的收集器，ctx代表定时器的上下文 OnTimerContext，OnTimerContext继承自Context，因此通过 OnTimerContext 可以访问数据的时间戳、注册和删除定时器以及进行旁路数据处理。       

此外，OnTimerContext 相比于Context多了一个`TimeDomain timeDomain()`方法，该方法的返回值 TimeDomain 代表当前触发的定时器的时间语义。TimeDomain 是一个枚举类型，包含EVENT_TIME 和 PROCESSING_TIME两个枚举值，如果值为EVENT_TIME，则代表当前触发的定时器是事件时间语义，如果值为PROCESSING_TIME，则代表当前触发的定时器是处理时间语义。
通过ProcessFunction提供的方法的定义，我们知道ProcessFunction可以实现自由访问一个流处理作业的事件数据、状态数据和定时器（事件时间定时器或处理时间定时器）的原因了。

## KeyedProcessFunction    
Flink 提供了多种不同场景下的处理函数，例如`键值数据流处理场景 KeyedProcessFunction`、`数据连接处理场景 CoProcessFunction`、`窗口数据处理场景 ProcessWindowFunction和ProcessAllWindowFunction`、`时间区间Join场景 ProcessJoinFunction`、`广播状态数据处理场景 BroadcastProcessFunction和KeyedBroadcastProcessFunction`。              

```bash
KeyedProcessFunction, as an extension of ProcessFunction, gives access to the key of timers in its onTimer(...) method.   
```

根据官网的定义，KeyedProcessFunction 是`ProcessFunction`处理函数的一种扩展, 在`StreamWordCountUseState`示例代码中可了解到`KeyedProcessFunction`包含3个参数，K、 I、O，其中的 K是指 KeyedStream的分区键(key), 这部分也体现了与 ProcessFunction 区别：`KeyedProcessFunction 中的Context和OnTimerContext相比于ProcessFunction的Context和OnTimerContext仅多了一个K getCurrentKey()的方法`, 下面是 KeyedProcessFunction的 Context 类型：    
![keyedstate05](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate05.png)     

在 KeyedProcessFunction 的 processElement()方法中，使用K getCurrentKey()方法可以获取当前处理的数据的key，在 KeyedProcessFunction 的onTimer() 方法中，使用KgetCurrentKey()方法可以获取当前触发的定时器的key。其他方法的执行逻辑和ProcessFunction完全相同。    

>基于上述，第一个疑问的答案已阐述,也同时涉及到第二个疑问，因为 KeyedProcessFunction 的context包含 getCurrentKey()，因为它的存在，我们不需要关心 Key的定义。           

## WordCountProcessFunction 的 countState 创建
countState 是用来存储统计结果的变量，它的创建是在 open()方法中的`getRuntimeContext().getState(descriptor)`发生的，这的确让人很难想象，首先 `getRuntimeContext()`是一个类型为`StreamingRuntimeContext`的变量，当 Flink Job 完成执行计划转换到创建 StreamTask 过程中（后面的 Blog 会介绍 ExecutionGraph 中涉及到），会为每个Task 创建一个 StreamingRuntimeContext (),若存在自定义算子，例如 WordCountProcessFunction，执行它的 open()方法, 此时的 State 若不存在，需创建新的 State。  

你可以将断点打在`AbstractStreamOperator#setup()`的 216行，代码如下：            
```java
this.runtimeContext =
        new StreamingRuntimeContext(
                environment,
                environment.getAccumulatorRegistry().getUserMap(),
                getMetricGroup(),
                getOperatorID(),
                getProcessingTimeService(),
                null,
                environment.getExternalResourceInfoProvider());
```

下面是`WordCountProcessFunction#open()`为入口，State 的创建入口：   
![keyedstate11](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate11.png)   

**AbstractKeyedStateBackend#getOrCreateKeyedState()**  
```java
public <N, S extends State, V> S getOrCreateKeyedState(
            final TypeSerializer<N> namespaceSerializer, StateDescriptor<S, V> stateDescriptor)
            throws Exception {
        // 检查序列化器
        checkNotNull(namespaceSerializer, "Namespace serializer");
        checkNotNull(
                keySerializer,
                "State key serializer has not been configured in the config. "
                        + "This operation cannot use partitioned state.");
        // 从缓存中获取与状态名称关联的内部键值状态  
        InternalKvState<K, ?, ?> kvState = keyValueStatesByName.get(stateDescriptor.getName());
        if (kvState == null) {
            if (!stateDescriptor.isSerializerInitialized()) {
                stateDescriptor.initializeSerializerUnlessSet(executionConfig);
            }
            // 创建一个具有TTL（如果启用）和延迟跟踪（如果启用）的内部键值状态
            kvState =
                    LatencyTrackingStateFactory.createStateAndWrapWithLatencyTrackingIfEnabled(
                            TtlStateFactory.createStateAndWrapWithTtlIfEnabled(
                                    namespaceSerializer, stateDescriptor, this, ttlTimeProvider),
                            stateDescriptor,
                            latencyTrackingStateConfig);
            // 将新创建的状态添加到缓存中                
            keyValueStatesByName.put(stateDescriptor.getName(), kvState);
            // 如果状态可以被查询，则发布它
            publishQueryableStateIfEnabled(stateDescriptor, kvState);
        }
        // 将内部键值状态强制转换为指定的状态类型并返回
        return (S) kvState;
}
```  

`latencyTracking`是 Flink 中的一种功能，它可以使用 State时跟踪和记录延迟信息，这个功能有助于监控和优化作业性能，特别是当你希望了解 State 操作的延迟情况时。Flink 提供了以下参数进行设置：                         
* state.backend.latency-track.keyed-state-enabled        
* state.backend.latency-track.sample-interval        
* state.backend.latency-track.history-size       
* state.backend.latency-track.state-name-as-variable         

`TtlStateFactory#createStateAndWrapWithTtlIfEnabled()` 方法主要用于创建带有过期时间配置的 State，判断 `stateDesc.getTtlConfig()`是否开启，如果开启则调用 `createState()`方法创建状态,否则调用 `KeyedStateFactory#createOrUpdateInternalState()`方法创建状态。     

>注意：stateDesc.getTtlConfig() 默认是不开启的。         
```java
@Nonnull private StateTtlConfig ttlConfig = StateTtlConfig.DISABLED;       
```        

在`StreamWordCountUseState`示例代码中，State 存储的是统计结果，显然数据是不能过期，那接下来，我们来看`KeyedStateFactory#createOrUpdateInternalState()`创建 State的流程。`KeyedStateFactory`是一个接口，在流处理场景中对应的`HeapKeyedStateBackend`实现类，所以 State 创建的入口如下图所示：     
![keyedstate06](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate06.png)        

下面是创建 State 的代码:
**HeapKeyedStateBackend#createOrUpdateInternalState()**            
```java
@Override
@Nonnull
public <N, SV, SEV, S extends State, IS extends S> IS createOrUpdateInternalState(
        @Nonnull TypeSerializer<N> namespaceSerializer,
        @Nonnull StateDescriptor<S, SV> stateDesc,
        @Nonnull StateSnapshotTransformFactory<SEV> snapshotTransformFactory,
        boolean allowFutureMetadataUpdates)
        throws Exception {
        StateTable<K, N, SV> stateTable =
                tryRegisterStateTable(
                        namespaceSerializer,
                        stateDesc,
                        getStateSnapshotTransformFactory(stateDesc, snapshotTransformFactory),
                        allowFutureMetadataUpdates);

        @SuppressWarnings("unchecked")
        IS createdState = (IS) createdKVStates.get(stateDesc.getName());
        if (createdState == null) {
                StateCreateFactory stateCreateFactory = STATE_CREATE_FACTORIES.get(stateDesc.getType());
                if (stateCreateFactory == null) {
                throw new FlinkRuntimeException(stateNotSupportedMessage(stateDesc));
                }
                createdState =
                        stateCreateFactory.createState(stateDesc, stateTable, getKeySerializer());
        } else {
                StateUpdateFactory stateUpdateFactory = STATE_UPDATE_FACTORIES.get(stateDesc.getType());
                if (stateUpdateFactory == null) {
                throw new FlinkRuntimeException(stateNotSupportedMessage(stateDesc));
                }
                createdState = stateUpdateFactory.updateState(stateDesc, stateTable, createdState);
        }

        createdKVStates.put(stateDesc.getName(), createdState);
        return createdState;
}
```   

先从已创建的状态映射中获取指定名称的状态对象，`createdKVStates`是一个 Map<String, State> 类型变量，它的 Key 对应的是 State 的名称 (例如 `WordCountProcessFunction.countState`,它的 name 是`wordCountState`)，如下图所示：    
![keyedstate07](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate07.png)     

从 Map中获取 State 对象为空，表示尚未创建，则利用 State创建工厂来创建对应的  State，`这部分涉及到泛型映射`，根据状态类型从`STATE_CREATE_FACTORIES`获取对应的状态创建工厂，而`STATE_CREATE_FACTORIES`是一个静态变量，它会类加载的时候初始化它的集合项, 注意：它的每个子项中的第二个元素是方法引用传递,`::create`是缩写方式。     
![keyedstate08](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate08.png)        

泛型映射：   
![keyedstate09](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate09.png)    

当时看到`(StateCreateFactory) HeapValueState::create` 这行代码本人有些懵，在方法引用前面添加强制转换，并且`StateCreateFactory`接口没有找到其实现类。      
![keyedstate10](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate10.png)      
```java
private interface StateCreateFactory {
<K, N, SV, S extends State, IS extends S> IS createState(
        StateDescriptor<S, SV> stateDesc,
        StateTable<K, N, SV> stateTable,
        TypeSerializer<K> keySerializer)
        throws Exception;
}
```

后来了解其目的是将 HeapValueState 、HeapListState 等等，它们的create()作为 `StateCreateFactory` 接口的实现，这样，当你调用 StateCreateFactory接口的 createState() 方法时，实际调用的是 HeapValueState类的 create()方法。`注意前提是 create()方法签名与 StateCreateFactory接口的 createState() 方法匹配`, 下面是`HeapValueState#create()`代码：     
```java
static <K, N, SV, S extends State, IS extends S> IS create(
        StateDescriptor<S, SV> stateDesc,
        StateTable<K, N, SV> stateTable,
        TypeSerializer<K> keySerializer) {
return (IS)
        new HeapValueState<>(
                stateTable,
                keySerializer,
                stateTable.getStateSerializer(),
                stateTable.getNamespaceSerializer(),
                stateDesc.getDefaultValue());
}
```

经过上述得到的结论是，调用`stateCreateFactory.createState`,其内部调用的是`HeapValueState#create()`,而 create()方法的形参就是createState()方法的形参。      
```java
createdState =
        stateCreateFactory.createState(stateDesc, stateTable, getKeySerializer());    
```

根据对象引用传递，createdState返回给了`WordCountProcessFunction.countState`,此时 countState 的类型是`HeapValueState`。      
![keyedstate12](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate12.png)     

## WordCountProcessFunction 的 countState 更新  
`StreamWordCountUseState.WordCountProcessFunction#processElement()`方法, 若当前 key 对应的 value 不为 null，则进行加1，下面是逻辑代码：  
```java
@Override
public void processElement(
        Tuple2<String, Long> value,
        Context ctx,
        Collector<Tuple2<String, Long>> out) throws Exception {

        // 获取当前单词的计数状态
        Long currentCount = countState.value();

        // 初始化状态
        if (currentCount == null) {
        currentCount = 0L;
        }

        // 自增并更新状态
        currentCount += value.f1;
        countState.update(currentCount);

        // 输出当前单词的计数结果
        out.collect(new Tuple2<>(value.f0, currentCount));
}
```

`countState.update(currentCount);`负责对 state的值进行更新，此时 update()方法对应的是`HeapValueState#update()`方法。    
```java
@Override
public void update(V value) {

if (value == null) {
        clear();
        return;
}

stateTable.put(currentNamespace, value);   
}
```

`stateTable 是什么？` 在上面的内容中多次出现，但我并没有介绍过，我们再回到 State创建方法中`HeapKeyedStateBackend#createOrUpdateInternalState()`,首先通过`tryRegisterStateTable()`方法创建 `stateTable`, 创建 State时，将 stateTable 当作形参传递过去。 接下来，我们还是已 stateTable.put 为入口，了解它是如何使用的？   

![keyedstate13](http://img.xinzhuxiansheng.com/blogimgs/flink/keyedstate13.png)  
通过上图可了解到，stateTable 是`CopyOnWriteStateTable`类型，其次 currentNamespace是`VoidNamespace`, 它的主要作用是用于对状态（state）进行逻辑隔离，而VoidNamespace 是 State的默认的命名空间类型。     

**StateTable#put()**  
```java
public void put(N namespace, S state) {
put(keyContext.getCurrentKey(), keyContext.getCurrentKeyGroupIndex(), namespace, state);
}
```

put()方法中除了`keyContext.getCurrentKeyGroupIndex()`非常陌生，其他形参都已介绍过, `keyContext.getCurrentKeyGroupIndex()`方法用于获取当前处理的键所属的键组（Key Group）索引。键组（Key Group）是 Flink 中状态分区的基本单位，用于将键空间划分为多个部分，以便更好地管理和分配状态。KeyGroup 是 Flink 的状态后端用于在并行子任务（parallel subtasks）之间分配状态的一种机制。每个键组对应一个唯一的索引值，范围从 0 到 maxParallelism - 1。maxParallelism 是一个配置参数，表示状态操作的最大并行度。Flink 将所有键映射到这些键组中，然后将键组分配给不同的并行子任务。       

关于`keyContext.getCurrentKeyGroupIndex()` 的生成逻辑，可参考`AbstractKeyedStateBackend#setCurrentKey()`和`KeyGroupRangeAssignment#computeKeyGroupForKeyHash()` 两个方法，KeyGroupIndex 是根据 `key.hashCode()`经过 MurmurHash运算后，将结果对`maxParallelism`取模，所以它的范围是从 0 到 maxParallelism - 1。  
**KeyGroupRangeAssignment#computeKeyGroupForKeyHash()**                 
```java
public static int computeKeyGroupForKeyHash(int keyHash, int maxParallelism) {
    return MathUtils.murmurHash(keyHash) % maxParallelism;
}
```

获取到 keyGroupIndex, 紧接着校验 key,namespace 的合法性，根据 keyGroupIndex 获取`StateTable对象下的 StateMap<K, N, S>[] keyGroupedStateMaps`值, 此时返回的是 `CopyOnWriteStateMap`类型对象。  
**StateTable#put()**  
```java
public void put(K key, int keyGroup, N namespace, S state) {
checkKeyNamespacePreconditions(key, namespace);
getMapForKeyGroup(keyGroup).put(key, namespace, state);
}
```

通过 putEntry()方法，找到 key，namespace 对应的 Entry，然后将 value 赋值给 Entry 的 state。 注意，这里其实可以有很大篇幅介绍`CopyOnWriteStateMap`的增删改查逻辑，其实就像 Java 中的 HashMap分析逻辑一样，所以该篇不过多介绍，会在后续的 Blog中介绍`CopyOnWriteStateMap`数据结构。      
**CopyOnWriteStateMap#put()**   
```java
@Override
public void put(K key, N namespace, S value) {
final StateMapEntry<K, N, S> e = putEntry(key, namespace);

e.state = value;
e.stateVersion = stateMapVersion;
}
```

## 总结   
关于 State 注册过程，并没有过多介绍，等后面深入探究后，再进行介绍。该篇介绍了`ProcessFunction`,`StreamingRuntimeContext` 的使用、StreamWordCountUseState示例中 ValueState的创建流程，如果大家有时间可以思考一个点，KeyGroup 和 KeyGroupIndex 对于 State的操作来说有什么帮助？      

refer  
1.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/datastream/fault-tolerance/state/      
2.https://nightlies.apache.org/flink/flink-docs-release-1.17/docs/dev/datastream/operators/process_function/       
3.'Flink SQL与DataStream：入门、进阶与实战'     