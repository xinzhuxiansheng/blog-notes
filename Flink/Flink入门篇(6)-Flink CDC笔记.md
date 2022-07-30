
## Flink CDC笔记

确认MySQL是否开启binlog
```
show variables like ‘log_bin%’;
```





## QA

1. Flink CDC 2.2.0 不支持Flink 1.14.x
https://www.cnblogs.com/Springmoon-venn/p/15951496.html

已完成

2. Flink CDC支持捕获Schema Change
includeSchemaChanges

https://blog.csdn.net/qq_31866793/article/details/121373178


3. DataStream实现Mysql同步


4. Mysql同步timestamp 快了8小时

https://blog.csdn.net/weixin_44762298/article/details/110198809

增加server-time-zone
" 'database-name' = 'db_test'," +
" 'table-name' = 'yzhou_tb_flink_job'," +
" 'server-time-zone' = 'Asia/Shanghai' " +
")");


5. 设置起始点
使用.startupOptions()

databaseList("otc_uat") // set captured database
.tableList("otc_uat.deposits") // set captured table
.username("qa")
.password("twJLNzbA1JNm7PEiQYi9kMdN")
.startupOptions(StartupOptions.latest())
.deserializer(new JsonDebeziumDeserializationSchema()) // converts SourceRecord to JSON String
.build();

也可以参考 https://github.com/fideism/gitee/blob/486a9b01267bba4e9941c7950bf998c0648c3ca7/DataLinkDC/Dinky/dlink-client/dlink-client-1.11/src/main/java/com/dlink/cdc/FlinkCDCMergeBuilder.java


6. 设置chunk.size
private static MySqlSource<JSONObject> getSource(String tableName) {
    //Properties properties = new Properties();
    //properties.setProperty("scan.incremental.snapshot.chunk.size", "777770");
    return MySqlSource.<JSONObject>builder()
            .hostname("114.67.101.133")
            .port(3306)
            .username("root")
            .password("jd@gmh#mysql")
            // 读取哪个库，可以读取多个库，默认监控库下所有表
            .databaseList("fangao")
            // 监控库下的某些表 test_jdbc1.table,test_jdbc1.table1
            .tableList("fangao."+tableName)
            // 反序列化  用的是 Debezium 的 StringDebeziumDeserializationSchema() 格式不方便，所以要自定义
            .deserializer(new JsonDebeziumDeserializationSchema())
            // 启动参数 提供了如下几个静态方法
            // StartupOptions.initial() 第一次启动的时候，会把历史数据读过来（全量）做快照，后续读取binlog加载新的数据，如果不做 chackpoint 会存在重启又全量一遍。
            // StartupOptions.earliest() 只从binlog开始的位置读（源头），这里注意，如果binlog开启的时间比你建库时间晚，可能会读不到建库语句会报错，earliest要求能读到建表语句
            // StartupOptions.latest() 只从binlog最新的位置开始读
            // StartupOptions.specificOffset() 自指定从binlog的什么位置开始读
            // StartupOptions.timestamp() 自指定binlog的开始时间戳
            .startupOptions(StartupOptions.initial())
            //.debeziumProperties(properties)
            .build();
}

7. Mysql CDC DataSteam API 这篇文章，要好好读
https://blog.csdn.net/qq_31866793/article/details/121373178

8. 动态Scheme 
https://blog.csdn.net/cloudbigdata/article/details/122935333


9. 水平扩展

Chunk 切分

分别设置 source ，sink的并行度 

blog： https://www.aboutyun.com/thread-31961-1-1.html
video： https://www.bilibili.com/video/BV1wL4y1Y7Xu?p=15&spm_id_from=pageDriver


10. 扩展

https://www.jianshu.com/p/6e5178dcd70c




4. 完整的CDC DEMO

参考
https://github.com/jizhang/flink-sandbox/tree/da9e3b23c8490d2156072af8692bf5dea4f74444/src/main/java/com/shzhangji/flinksandbox/cdc

参考
https://github.com/kolterr/flink-learn/blob/118196c47d6ef2f2a8d796e4971927bba6acb50b/mysql-cdc/src/main/java/com/akazone/stream/mysql2mysql.java

cdc只是source，datadstream API可以拿到数据和schema信息，拿到schema信息后怎么处理和加工需要自己写代码实现，去目标端建表这种群里不少小伙伴已经实现了，不过这个不是cdc提供的能力



datax-web
https://github.com/WeiYe-Jing/datax-web



包含CDC项目
https://github.com/authorwlh/wlhbdp









增加字段

{"source":{"version":"1.5.4.Final","connector":"mysql","name":"mysql_binlog_source","ts_ms":1648694900467,"snapshot":"false","db":"db_test","sequence":null,"table":"yzhou_tb_flink_job","server_id":57148,"gtid":"47cbfaaf-f416-11eb-b39d-005056907747:665142957","file":"binlog.002274","pos":952652947,"row":0,"thread":null,"query":null},"historyRecord":"{\"source\":{\"file\":\"binlog.002274\",\"pos\":952652947,\"server_id\":57148},\"position\":{\"transaction_id\":null,\"ts_sec\":1648694900,\"file\":\"binlog.002274\",\"pos\":952653161,\"gtids\":\"47cbfaaf-f416-11eb-b39d-005056907747:1-665142956,aaaaaaaa-aaaa-aaaa-2021-080312495999:1-272900538\",\"server_id\":57148},\"databaseName\":\"db_test\",\"ddl\":\"ALTER TABLE `db_test`.`yzhou_tb_flink_job` \\nADD COLUMN `new_field` varchar(255) NULL COMMENT 'yzhou 测试' AFTER `jar_manager_id`\",\"tableChanges\":[{\"type\":\"ALTER\",\"id\":\"\\\"db_test\\\".\\\"yzhou_tb_flink_job\\\"\",\"table\":{\"defaultCharsetName\":\"utf8mb4\",\"primaryKeyColumnNames\":[\"id\"],\"columns\":[{\"name\":\"id\",\"jdbcType\":-5,\"typeName\":\"BIGINT\",\"typeExpression\":\"BIGINT\",\"charsetName\":null,\"length\":20,\"position\":1,\"optional\":false,\"autoIncremented\":true,\"generated\":true},{\"name\":\"job_type\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":2,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"creator\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":100,\"position\":3,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"job_name\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":200,\"position\":4,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"resource_from\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":5,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"belong_team_id\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":6,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"k8s_cluster_id\",\"jdbcType\":-5,\"typeName\":\"BIGINT\",\"typeExpression\":\"BIGINT\",\"charsetName\":null,\"length\":20,\"position\":7,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"execution_mode\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":8,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"main_class\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":300,\"position\":9,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"jm_memory\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":10,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"tm_memory\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":11,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"total_task\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":12,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"total_slot\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":13,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"available_slot\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":14,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"dynamic_options\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":2000,\"position\":15,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"job_cluster_id\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":200,\"position\":16,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"k8s_restexposed_type\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":17,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"flink_image\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":255,\"position\":18,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"k8s_namespace\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":100,\"position\":19,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"flink_resturl\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":255,\"position\":20,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"job_state\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":21,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"create_stime\",\"jdbcType\":93,\"typeName\":\"DATETIME\",\"typeExpression\":\"DATETIME\",\"charsetName\":null,\"position\":22,\"optional\":false,\"autoIncremented\":false,\"generated\":false},{\"name\":\"update_stime\",\"jdbcType\":93,\"typeName\":\"DATETIME\",\"typeExpression\":\"DATETIME\",\"charsetName\":null,\"position\":23,\"optional\":false,\"autoIncremented\":false,\"generated\":false},{\"name\":\"is_delete\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":3,\"position\":24,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"restart_count\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":25,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"program_args\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":1500,\"position\":26,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"flink_job_id\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":100,\"position\":27,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"is_online\",\"jdbcType\":4,\"typeName\":\"INT\",\"typeExpression\":\"INT\",\"charsetName\":null,\"length\":11,\"position\":28,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"duration\",\"jdbcType\":-5,\"typeName\":\"BIGINT\",\"typeExpression\":\"BIGINT\",\"charsetName\":null,\"length\":20,\"position\":29,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"start_time\",\"jdbcType\":93,\"typeName\":\"DATETIME\",\"typeExpression\":\"DATETIME\",\"charsetName\":null,\"position\":30,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"end_time\",\"jdbcType\":93,\"typeName\":\"DATETIME\",\"typeExpression\":\"DATETIME\",\"charsetName\":null,\"position\":31,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"jar_manager_id\",\"jdbcType\":-5,\"typeName\":\"BIGINT\",\"typeExpression\":\"BIGINT\",\"charsetName\":null,\"length\":20,\"position\":32,\"optional\":true,\"autoIncremented\":false,\"generated\":false},{\"name\":\"new_field\",\"jdbcType\":12,\"typeName\":\"VARCHAR\",\"typeExpression\":\"VARCHAR\",\"charsetName\":\"utf8mb4\",\"length\":255,\"position\":33,\"optional\":true,\"autoIncremented\":false,\"generated\":false}]}}]}"}