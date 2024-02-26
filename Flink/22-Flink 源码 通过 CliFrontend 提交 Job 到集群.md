## Flink 源码 通过 CliFrontend 提交 Job 到集群

### 1.配置 CliFrontend 启动项 
配置启动类：`org.apache.flink.client.cli.CliFrontend`       

#### 1.1 配置以下启动参数:   
* **JDK**: 1.8    
* **Module**: `flink-clients`   
* **VM options**:   
```shell 
-Dlog.file=./log/flink-hunter-client-hunter.log 
-Dlog4j.configuration=./conf/log4j-cli.properties 
-Dlog4j.configurationFile=./conf/log4j-cli.properties 
-Dlogback.configurationFile=./conf/logback.xml
```
* **Main class**: org.apache.flink.client.cli.CliFrontend     
* **Program arguments**:  run -c com.yzhou.flink.example.wordcount.WordCount ./flink-jar-1.0-SNAPSHOT.jar
* **Environment variables**: FLINK_CONF_DIR=./conf  


### 2.开发 demo.jar  
在上一篇 Blog中，我们已经 Flink 源码导入到 Idea中，所以我们可以基于 Flink 源码项目来开发一些 `examples` 来验证。 可以在项目根目录下的`flink-examples/flink-examples-streaming` 模块开发示例 WordCount（为了不破坏源代码中的 example，可自己创建包和java）； 

>好处：在flink 源码上开发 example，这帮我们省去 example 项目的搭建和 jar 依赖，针对调整后的代码，仅需打包单独 module即可。   

**示例代码**        
```java
import org.apache.flink.api.common.functions.ReduceFunction;
import org.apache.flink.api.common.typeinfo.TypeHint;
import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.streaming.api.datastream.DataStreamSource;
import org.apache.flink.streaming.api.datastream.SingleOutputStreamOperator;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;

/**
 * yzhou 测试 word count
 */
public class WordCount {
    public static void main(String[] args) throws Exception {
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        env.setParallelism(1);

        DataStreamSource<String> s1 =  env.fromElements("a","b","c","d");

        SingleOutputStreamOperator<Tuple2<String, Integer>> res = s1.map(s -> Tuple2.of(s, 1))
                .returns(new TypeHint<Tuple2<String, Integer>>() {
                })
                .keyBy(t -> t.f0)
                .reduce(new ReduceFunction<Tuple2<String, Integer>>() {
                    @Override
                    public Tuple2<String, Integer> reduce(
                            Tuple2<String, Integer> value1,
                            Tuple2<String, Integer> value2) throws Exception {
                        return Tuple2.of(value1.f0, value1.f1 + value2.f1);
                    }
                });
        res.print();
        env.execute();
    }
}
```

### 3.通过 CliFrontend 测试示例jar   
Idea 启动 CliFrontend#main()，会在控制台打印出如下内容：    
```
Job has been submitted with JobID 47e551ebdd4460bdc34811249cfa3b70
Program execution finished
Job with JobID 47e551ebdd4460bdc34811249cfa3b70 has finished.
Job Runtime: 1696 ms
```

因为示例jar 是有限数据源，则会自动停止；为了验证 job是否执行成功，可查看 Idea 中的 TaskManger的控制台查看预期所打印的结果，内容如下：   
```
(a,1)
(b,1)
(c,1)
(d,1)
```  

