```
CREATE TABLE hbase_lookup (
  rowkey STRING,
  info ROW<id STRING, name STRING, address STRING>,
  PRIMARY KEY (rowkey) NOT ENFORCED
) WITH (
  'connector' = 'hbase-2.2',
  'table-name' = 'yzhou:person',
  'zookeeper.quorum' = 'cdh5-1:2181',
  'zookeeper.znode.parent' = '/hbase',
  'properties.hbase.client.authentication.type' = 'kerberos',
  'properties.hbase.client.keytab.file' = '/root/hdfsconfig/hbase.keytab',
  'properties.hbase.client.principal' = 'hbase/cdh5-1@CDH5.COM'
);
```