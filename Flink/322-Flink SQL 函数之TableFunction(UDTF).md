## Flink SQL 函数之 TableFunction

>特征：1入，多出, 别名 UDTF

### 实现 TableFunction


```
package com.yzhou.job.sql;

import org.apache.flink.configuration.Configuration;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.annotation.DataTypeHint;
import org.apache.flink.table.annotation.FunctionHint;
import org.apache.flink.table.api.Table;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.table.functions.ScalarFunction;
import org.apache.flink.table.functions.TableFunction;
import org.apache.flink.types.Row;
import org.json.JSONArray;
import org.json.JSONObject;

import static org.apache.flink.table.api.Expressions.$;
import static org.apache.flink.table.api.Expressions.call;

/**
 * Flink SQL UDTF 之  TableFunction 数据是 1进 多出
 * 将 data 字段拆分后，再与 其他字段 join 成多条数据
 * 测试数据： {"userId":9527,"day":"2021-05-12","begintime":1620785058833,"endtime":1620785217511,"data":[{"package":"com.browser","activetime":120000},{"package":"com.qq","activetime":80000}]}
 * <p>
 * 这里要用到 侧写表
 */
public class FlinkSQLBaseTableFunctionApp {
    public static void main(String[] args) {
        Configuration configuration = new Configuration();
        //1.获取stream的执行环境
        StreamExecutionEnvironment senv = StreamExecutionEnvironment.createLocalEnvironmentWithWebUI(configuration);
        //设置并行度
        senv.setParallelism(1);

        //2.创建表执行环境
        StreamTableEnvironment tEnv = StreamTableEnvironment.create(senv);

        //3.读取数据
        DataStream<String> lines = senv.socketTextStream("yzhou.com", 7088);

        //4.流转换为动态表
        Table table = tEnv.fromDataStream(lines, $("line"));
        tEnv.createTemporaryView("userbehavior", table);

        //5.1调用方式1：TableAPI内连接实现
        tEnv.from("userbehavior")
                .joinLateral(call(explodeFunction.class, $("line"), "data").as("package", "activetime"))
                .select(
                        call(JsonFunction.class, $("line"), "userId"),
                        call(JsonFunction.class, $("line"), "day"),
                        call(JsonFunction.class, $("line"), "begintime"),
                        call(JsonFunction.class, $("line"), "endtime"),
                        $("package"),
                        $("activetime")
                ).execute().print();
        //5.2调用方式2：TableAPI左外连接实现
//        tEnv.from("userbehavior")
//                .leftOuterJoinLateral(call(explodeFunction.class, $("line"), "data").as("package", "activetime"))
//                .select(
//                        call(JsonFunction.class, $("line"), "userId"),
//                        call(JsonFunction.class, $("line"), "day"),
//                        call(JsonFunction.class, $("line"), "begintime"),
//                        call(JsonFunction.class, $("line"), "endtime"),
//                        $("package"),
//                        $("activetime")
//                ).execute().print();
//
//        tEnv.createTemporarySystemFunction("JsonFunction", JsonFunction.class);
//        tEnv.createTemporarySystemFunction("explodeFunction", explodeFunction.class);
        //5.3 调用方式3：sql内连接实现
//        tEnv.sqlQuery("select " +
//                "JsonFunction(line,'userId')," +
//                "JsonFunction(line,'day')," +
//                "JsonFunction(line,'begintime')," +
//                "JsonFunction(line,'endtime')," +
//                "package," +
//                "activetime " +
//                "  from userbehavior " +
//                ",lateral table(explodeFunction(line,'data')) "
//        ).execute().print();
        //5.4调用方式4：sql左外连接实现
//        tEnv.sqlQuery("select " +
//                "JsonFunction(line,'userId')," +
//                "JsonFunction(line,'day')," +
//                "JsonFunction(line,'begintime')," +
//                "JsonFunction(line,'endtime')," +
//                "package," +
//                "activetime " +
//                "  from userbehavior " +
//                "left join lateral table(explodeFunction(line,'data')) as sc(package,activetime) on true "
//        ).execute().print();
    }

    /**
     * 自定义udf
     */
    public static class JsonFunction extends ScalarFunction {
        public String eval(String line, String key) {
            //转换为JSON
            JSONObject baseJson = new JSONObject(line);
            String value = "";
            if (baseJson.has(key)) {
                //根据key获取value
                return baseJson.optString(key);
            }
            return value;
        }
    }

    /**
     * 自定义udtf
     */
    @FunctionHint(output = @DataTypeHint("ROW<package STRING,activetime INT>"))
    public static class explodeFunction extends TableFunction {
        public void eval(String line, String key) {
            //转换为JSON
            JSONObject baseJson = new JSONObject(line);
            //提取key为data的JSONArray数据
            JSONArray jsonArray = baseJson.getJSONArray(key);
            //循环解析输出
            for (int i = 0; i < jsonArray.length(); i++) {
                String col1 = jsonArray.getJSONObject(i).getString("package");
                Integer col2 = jsonArray.getJSONObject(i).getInt("activetime");
                collect(Row.of(col1, col2)); // Row.of() 输出一行数据
            }
        }
    }
}


```