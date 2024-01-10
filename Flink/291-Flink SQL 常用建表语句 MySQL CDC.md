```
CREATE TABLE
  `yzhou_test01_cdc` (
    `id` INT NOT NULL COMMENT '',
    `address` STRING COMMENT '',
    `ext_field01` STRING COMMENT '',
    `name` STRING COMMENT '',
    PRIMARY KEY (id) NOT ENFORCED
  )
WITH
  (
    'scan.incremental.snapshot.enabled' = 'true',
    'debezium.datetime.format.date' = 'yyyy-MM-dd',
    'debezium.datetime.format.time' = 'HH-mm-ss',
    'hostname' = '192.168.0.xxx',
    'password' = 'xxxx',
    'debezium.snapshot.mode' = 'initial',
    'connector' = 'mysql-cdc',
    'port' = '3306',
    'database-name' = 'xxxx',
    'debezium.datetime.format.timestamp.zone' = 'UTC+8',
    'debezium.datetime.format.datetime' = 'yyyy-MM-dd HH-mm-ss',
    'table-name' = 'yzhou_test01',
    'username' = 'xxxx',
	'server-time-zone' = 'Asia/Shanghai'
  )
```