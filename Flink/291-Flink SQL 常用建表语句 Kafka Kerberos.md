```
CREATE TABLE TABLE_NAME (
  DEAL_SERIAL_NO STRING COMMENT '',
    ...
  current_ts STRING COMMENT '',
  d_timestamp AS TO_TIMESTAMP(current_ts, 'yyyy-MM-dd''T''HH:mm:ss.SSSSSS'),
  WATERMARK FOR d_timestamp AS d_timestamp - INTERVAL '5' SECOND
)
WITH ( 
  'properties.bootstrap.servers' = 'xxx.xxx.xx.xx:9092', 
  'connector' = 'kafka',
  'json.ignore-parse-errors' = 'false', 
  'format' = 'json', 
  'topic' = 'xxx', 
  'properties.group.id' = 'testGroup02', 
  'scan.startup.mode' = 'earliest-offset', 
  'json.fail-on-missing-field' = 'false', 
  'properties.security.protocol' = 'SASL_PLAINTEXT', 'properties.sasl.kerberos.service.name' = 'kafka', 
  'properties.sasl.mechanism' = 'GSSAPI', 
  'properties.sasl.kerberos.principal.to.local.rules' = 'kafka/xxx@xxx', 
  'properties.sasl.jaas.config' = 'com.sun.security.auth.module.Krb5LoginModule required useKeyTab=true storeKey=true keyTab="/kafkaconfig/kafka.keytab" principal="kafka/xxx@xxx";' 
);
```