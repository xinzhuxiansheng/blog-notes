## Flink DataStream API 数据模拟        

### 引言    
Flink旁侧流（Side output）是指在Flink的流处理过程中，除了主要的数据流之外，还可以产生一些额外的流数据，这些额外的数据流称为旁侧流。旁侧流通常是由一些不满足主要数据流处理逻辑条件的数据所产生的。在Flink中，通过使用侧输出流，可以将这些额外的数据流输出到不同的目标中进行处理，比如输出到Kafka、输出到HDFS等。这个功能在一些场景下非常有用，比如处理订单数据时，可能需要将异常订单和正常订单分别输出到不同的目标，以进行不同的处理。使用Flink旁侧流需要创建一个OutputTag，来表示额外的数据流，然后在处理过程中通过调用processElement()方法将数据发送到旁侧流中。在对数据流进行处理时，可以使用getSideOutput()方法获取旁侧流的数据。            

### 案例 Code   
```java

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.ververica.cdc.connectors.mysql.source.MySqlSource;
import com.ververica.cdc.debezium.JsonDebeziumDeserializationSchema;
import org.apache.doris.flink.cfg.DorisExecutionOptions;
import org.apache.doris.flink.cfg.DorisOptions;
import org.apache.doris.flink.cfg.DorisReadOptions;
import org.apache.doris.flink.sink.DorisSink;
import org.apache.doris.flink.sink.writer.SimpleStringSerializer;
import org.apache.flink.api.common.eventtime.WatermarkStrategy;
import org.apache.flink.streaming.api.datastream.DataStreamSource;
import org.apache.flink.streaming.api.datastream.SingleOutputStreamOperator;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.streaming.api.functions.ProcessFunction;
import org.apache.flink.util.Collector;
import org.apache.flink.util.OutputTag;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Properties;

/***
 *
 * Synchronize the full database through flink cdc
 *
 */
public class DatabaseFullSync {

    private static String TABLE_A = "tableA";
    private static String TABLE_B = "tableB";
    private static OutputTag<String> tableA = new OutputTag<String>(TABLE_A){};
    private static OutputTag<String> tableB = new OutputTag<String>(TABLE_B){};
    private static final Logger log = LoggerFactory.getLogger(DatabaseFullSync.class);

    public static void main(String[] args) throws Exception {

        MySqlSource<String> mySqlSource = MySqlSource.<String>builder()
            .hostname("127.0.0.1")
            .port(3306)
            .databaseList("test") // set captured database
            .tableList("test.*") // set captured table
            .username("root")
            .password("password")
            .deserializer(new JsonDebeziumDeserializationSchema()) // converts SourceRecord to JSON String
            .build();

        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();
        env.setParallelism(1);
        // enable checkpoint
        env.enableCheckpointing(10000);

        DataStreamSource<String> cdcSource = env.fromSource(mySqlSource, WatermarkStrategy.noWatermarks(), "MySQL CDC Source");

        SingleOutputStreamOperator<String> process = cdcSource.process(new ProcessFunction<String, String>() {
            @Override
            public void processElement(String row, ProcessFunction<String, String>.Context context, Collector<String> collector) throws Exception {
                JSONObject rowJson = JSON.parseObject(row);
                String op = rowJson.getString("op");
                JSONObject source = rowJson.getJSONObject("source");
                String table = source.getString("table");

                //only sync insert
                if ("c".equals(op)) {
                    String value = rowJson.getJSONObject("after").toJSONString();
                    if (TABLE_A.equals(table)) {
                        context.output(tableA, value);
                    } else if (TABLE_B.equals(table)) {
                        context.output(tableB, value);
                    }
                } else {
                    log.info("other operation...");
                }
            }
        });

        //
        process.getSideOutput(tableA).sinkTo(buildDorisSink(TABLE_A));
        process.getSideOutput(tableB).sinkTo(buildDorisSink(TABLE_B));

        env.execute("Full Database Sync ");
    }


    public static DorisSink buildDorisSink(String table){
        DorisSink.Builder<String> builder = DorisSink.builder();
        DorisOptions.Builder dorisBuilder = DorisOptions.builder();
        dorisBuilder.setFenodes("127.0.0.1:8030")
            .setTableIdentifier("test." + table)
            .setUsername("root")
            .setPassword("password");

        Properties pro = new Properties();
        //json data format
        pro.setProperty("format", "json");
        pro.setProperty("read_json_by_line", "true");
        DorisExecutionOptions executionOptions = DorisExecutionOptions.builder()
            .setLabelPrefix("label-" + table) //streamload label prefix,
            .setStreamLoadProp(pro).build();

        builder.setDorisReadOptions(DorisReadOptions.builder().build())
            .setDorisExecutionOptions(executionOptions)
            .setSerializer(new SimpleStringSerializer()) //serialize according to string
            .setDorisOptions(dorisBuilder.build());

        return builder.build();
    }
}
```