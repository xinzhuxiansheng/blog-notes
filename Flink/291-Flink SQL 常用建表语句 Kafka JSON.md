```
CREATE TABLE `TABLENAME` (
    `TRANS_DATE` STRING COMMENT '',
    ...
    current_ts STRING COMMENT '',
    d_timestamp AS TO_TIMESTAMP (current_ts, 'yyyy-MM-dd''T''HH:mm:ss.SSSSSS'),
    WATERMARK FOR d_timestamp AS d_timestamp - INTERVAL '5' SECOND
  )
WITH
  (
    'properties.bootstrap.servers' = 'DN-KAFKA3:9092',
    'connector' = 'kafka',
    'json.ignore-parse-errors' = 'false',
    'format' = 'json',
    'topic' = 'CTDS_TBBONDPROPERTY',
    'properties.group.id' = 'yzhougid122101',
    'scan.startup.mode' = 'earliest-offset',
    'json.fail-on-missing-field' = 'false'
  )
```