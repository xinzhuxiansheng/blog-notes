


nohup bin/hive --service metastore -p 9083 2>&1 >/dev/null &


使用 hive metastore 
bin/beeline -u 'jdbc:hive2://bigdata04:10009/;#kyuubi.engine.type=SPARK_SQL;spark.master=yarn;spark.submit.deployMode=cluster;hive.metastore.uris=thrift://bigdata04:9083' -n root