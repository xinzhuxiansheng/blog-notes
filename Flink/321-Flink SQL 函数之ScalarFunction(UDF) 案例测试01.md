## Flink SQL 函数之标量函数（ScalarFunction）

>特征：1入，1出， 别名 UDF

### 实现 ScalarFunction


```
package com.yzhou.job.sql;

import org.apache.flink.configuration.Configuration;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.Table;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.table.functions.ScalarFunction;
import org.json.JSONObject;

import static org.apache.flink.table.api.Expressions.$;

/**
 * Flink SQL UDF 之 标量函数  ScalarFunction 数据是 1进 1出
 * nc -lk 8888
 */
public class FlinkSQLBaseScalarFunctionApp {
    public static void main(String[] args) {
        Configuration configuration = new Configuration();
        //1.获取stream的执行环境
        StreamExecutionEnvironment senv = StreamExecutionEnvironment.createLocalEnvironmentWithWebUI(configuration);
        senv.setParallelism(1);

        //2.创建表执行环境
        StreamTableEnvironment tEnv = StreamTableEnvironment.create(senv);

        //3.读取数据
        DataStream<String> lines = senv.socketTextStream("yzhou.com",7088);

        //4.流转换为动态表
        Table table = tEnv.fromDataStream(lines,$("line"));
        tEnv.createTemporaryView("clicklog",table);

        //5.调用方式1
//        tEnv.from("clicklog").select(
//                call(JsonFunction.class,$("line"),"user"),
//                call(JsonFunction.class,$("line"),"url"),
//                call(JsonFunction.class,$("line"),"cTime")
//        ).execute().print();

        //5.调用方式2
        tEnv.createTemporarySystemFunction("JsonFunction",JsonFunction.class);
//                tEnv.from("clicklog").select(
//                call("JsonFunction",$("line"),"user"),
//                call("JsonFunction",$("line"),"url"),
//                call("JsonFunction",$("line"),"cTime")
//        ).execute().print();

        //5.调用方式3
        tEnv.sqlQuery("select JsonFunction(line,'user'),JsonFunction(line,'url'),JsonFunction(line,'cTime')  from clicklog").execute().print();

    }

    /**
     * 最简单的标量函数
     */
    public static class JsonFunction extends ScalarFunction{
        public String eval(String line,String key){
            JSONObject baseJson = new JSONObject(line);
            String value = "";
            if(baseJson.has(key)){
                return baseJson.getString(key);
            }
            return value;
        }
    }
}

```