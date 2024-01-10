```
CREATE TABLE
  `yzhou_test02_tb` (
    `id` INT NOT NULL COMMENT '',
    `name` STRING NOT NULL COMMENT '',
    `address` STRING COMMENT '',
    `ext_field01` STRING COMMENT '',
    PRIMARY KEY (id) NOT ENFORCED
  )
WITH
  (
    'password' = 'xxx',
    'connector' = 'jdbc',
    'table-name' = 'yzhou_test02',
    'sink.parallelism' = '1',
    'url' = 'jdbc:mysql://192.168.0.xxx:3306/xxx',
    'username' = 'xxxx'
  )
```