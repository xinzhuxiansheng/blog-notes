## Flink State 使用 LocalStreamEnvironment（IDEA 开发环境） 验证 State & 从 Checkpoint 恢复 State 

>Flink version: 1.15.4   

### 引言    
在 IDEA环境中开发 Flink Job 使用 State时，经常会遇到这样的问题，需要验证状态是否生效？以及重启应用之后，状态里的数据能否从 checkpoint/savepoint 的恢复？ 下面内容主要讲解 `IDEA环境中验证 Checkpoint 恢复 State 功能`;      

### 带有 State 的 Flink Job 案例分析    
编写一个简单的 Flink 任务，实现功能如下:  
1.从 SocketTextStream 中实时接收文本内容                
2.将接收到文本转换为事件样例类，该事件样例类包含三个字段 id、value、time            
3.事件按照 id 进行 KeyBy 之后，使用 Process function 统计每种事件的个数和value值的总和              
4.控制台输出统计结果  

>注意此时的示例代码不包含 checkpoint 的配置 
```scala
package com.yzhou.scala.state

import org.apache.flink.api.common.state.{ValueState, ValueStateDescriptor}
import org.apache.flink.api.scala._
import org.apache.flink.configuration.Configuration
import org.apache.flink.runtime.state.hashmap.HashMapStateBackend
import org.apache.flink.streaming.api.environment.CheckpointConfig.ExternalizedCheckpointCleanup
import org.apache.flink.streaming.api.functions.KeyedProcessFunction
import org.apache.flink.streaming.api.scala.{DataStream, StreamExecutionEnvironment}
import org.apache.flink.util.Collector

/**
 * 实时计算事件总个数，以及value总和
 */
object EventCounterJob {

  def main(args: Array[String]): Unit = {
    val env: StreamExecutionEnvironment = StreamExecutionEnvironment.createLocalEnvironmentWithWebUI(new Configuration)
    // 1. 从socket中接收文本数据
    val streamText: DataStream[String] = env.socketTextStream("127.0.0.1", 9000)
    // 2. 将文本内容按照空格分割转换为事件样例类
    val events = streamText.map(s => {
      val tokens = s.split(" ")
      Event(tokens(0), tokens(1).toDouble, tokens(2).toLong)
    })
    // 3. 按照时间id分区，然后进行聚合统计
    val counterResult = events.keyBy(_.id).process(new EventCounterProcessFunction)
    // 4. 结果输出到控制台
    counterResult.print()
    env.execute("EventCounterJob")
  }
}

/**
 * 定义事件样例类
 *
 * @param id    事件类型id
 * @param value 事件值
 * @param time  事件时间
 */
case class Event(id: String, value: Double, time: Long)

/**
 * 定义事件统计器样例类
 *
 * @param id    事件类型id
 * @param sum   事件值总和
 * @param count 事件个数
 */
case class EventCounter(id: String, var sum: Double, var count: Int)

/**
 * 继承KeyedProcessFunction实现事件统计
 */
class EventCounterProcessFunction extends KeyedProcessFunction[String, Event, EventCounter] {
  private var counterState: ValueState[EventCounter] = _
  override def open(parameters: Configuration): Unit = {
    super.open(parameters)
    // 从flink上下文中获取状态
    counterState = getRuntimeContext.getState(new ValueStateDescriptor[EventCounter]("event-counter", classOf[EventCounter]))
  }
  override def processElement(i: Event,
                              context: KeyedProcessFunction[String, Event, EventCounter]#Context,
                              collector: Collector[EventCounter]): Unit = {
    // 从状态中获取统计器，如果统计器不存在给定一个初始值
    val counter = Option(counterState.value()).getOrElse(EventCounter(i.id, 0.0, 0))
    // 统计聚合
    counter.count += 1
    counter.sum += i.value
    // 发送结果到下游
    collector.collect(counter)
    // 保存状态
    counterState.update(counter)
  }
}
```

### 不带 checkpoint 测试    

基于上面示例 Code，先启动 nc 命令监听 9000 端口  
```
nc -lk 9000 
```

再启动 main()方法，并在 命令终端模拟发送如下数据：  
```
event-1 1 1591695864473
event-1 12 1591695864474    
```

此时观察 IDEA控制台会输出`最新`的内容： 
```shell
# event-1 value 求和 = 13，出现2次 
EventCounter(event-1,13.0,2)        
```

上面的输出内容是符合上面代码处理的预期的， 但当 `main() 重启后`，我们在模拟一条数据发送：  
```
event-1 10 1591695864476
```

此时控制台输出的`最新`的如下内容，这很明显并非 `示例需求的介绍的那样 “统计每种事件的个数和value值的总和”`，之前的统计数据丢失了。       
```
EventCounter(event-1,10.0,1)  
```

### 带 checkpoint 测试  
基于上面的代码的基础上，增加了 checkpoint 配置：    
```scala
... 省略部分代码

val env: StreamExecutionEnvironment = StreamExecutionEnvironment.createLocalEnvironmentWithWebUI(new Configuration)
// 配置checkpoint
// 设置状态后端为HashMapStateBackend
env.setStateBackend(new HashMapStateBackend());
// 做两个checkpoint的间隔为1秒
env.enableCheckpointing(1000)
// 表示下 Cancel 时是否需要保留当前的 Checkpoint，默认 Checkpoint 会在整个作业 Cancel 时被删除。Checkpoint 是作业级别的保存点。
env.getCheckpointConfig.setExternalizedCheckpointCleanup(ExternalizedCheckpointCleanup.RETAIN_ON_CANCELLATION)
env.getCheckpointConfig.setCheckpointStorage("file:///Users/a/TMP/flink_checkpoint")

... 省略部分代码

```

>注意：在很多Blog 中使用 `FsStateBackend` 来配置 checkpoint 存储状态后端，该类已在高版本 Flink 废弃了，而示例代码中的配置，使用 `HashMapStateBackend` 作为状态存储后端，并异步将内存中的 state 持久化在 本地文件系统中。    

接下来，我们像上一章节“不带 checkpoint 测试” 步骤，重新操作一遍， `得到的想象与之前的想象如出一辙`， 重启后，之前统计的结果也没有恢复过来，但我们指定的 checkpoint 路径(file:///Users/a/TMP/flink_checkpoint)下多了一些文件，内容如下：   
```
➜  8781988ef03014886759192ebd228383 
.
├── chk-69
│   └── _metadata
├── shared
└── taskowned
``` 

>注意，Flink 重启时不会自动加载状态，需要我们手动指定 checkpoint 路径，接下来 我们在上面的示例代码中添加从指定 checkpoint 路径恢复。    

### 指定 execution.savepoint.path 参数 测试 
基于 “带 checkpoint 测试”的示例代码基础上， 修改 StreamExecutionEnvironment 对象，创建时指定 `execution.savepoint.path`   
```
var configuration =  new Configuration()
configuration.setString("execution.savepoint.path","file:///Users/a/TMP/flink_checkpoint/8781988ef03014886759192ebd228383/chk-69");
// 获取执行环境
val env: StreamExecutionEnvironment = StreamExecutionEnvironment.createLocalEnvironmentWithWebUI(configuration)
```

此时，启动 main() 方法后，只需要发送如下内容即可, 我想你应该了解到，从状态恢复后的数据是 `EventCounter(event-1,23.0,3) `, 所以测试的前面3条测试数据不需要发送，仅发送第4条数据(消息内容如下)，验证统计结果是否是基于前面3条数据统计结果计算的即可。     
```
event-1 1 1591695864476    
```

### 总结    
“指定 execution.savepoint.path 参数 测试” 章节的测试结果，应该是符合示例需求的预期的。 这篇blog 验证了从checkpoint 恢复state功能，也从此打开了`调试 checkpoint 恢复 state 数据源码分析`的新篇章。           

