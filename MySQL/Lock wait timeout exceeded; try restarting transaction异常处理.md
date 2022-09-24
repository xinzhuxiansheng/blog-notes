
## MySQL发生Lock wait timeout exceeded; try restarting transaction

解决方法:

1.数据库中执行如下sql，查看当前数据库的线程情况:
```shell
show full PROCESSLIST;
```

2.在INNODB_TRX事务表中查看,看trx_mysql_thread_id是否在show full processlist里面的sleep线程中(INNODB_TRX表的`trx_mysql-thread_id`字段对应show full processlist中的id)；如果在，就说明这个sleep的线程事务一直没有commit或者rollback，而是卡住了，需要我们手动删除.

```shell
select * from information_schema.innodb_trx\G;
```

**Output**
```
mysql> select * from information_schema.innodb_trx\G;
*************************** 1. row ***************************
                    trx_id: 859127357
                 trx_state: RUNNING
               trx_started: 2022-09-24 06:10:04
     trx_requested_lock_id: NULL
          trx_wait_started: NULL
                trx_weight: 3
       trx_mysql_thread_id: 7563920
                 trx_query: NULL
       trx_operation_state: NULL
         trx_tables_in_use: 0
         trx_tables_locked: 1
          trx_lock_structs: 2
     trx_lock_memory_bytes: 1136
           trx_rows_locked: 1
         trx_rows_modified: 1
   trx_concurrency_tickets: 0
       trx_isolation_level: REPEATABLE READ
         trx_unique_checks: 1
    trx_foreign_key_checks: 1
trx_last_foreign_key_error: NULL
 trx_adaptive_hash_latched: 0
 trx_adaptive_hash_timeout: 0
          trx_is_read_only: 0
trx_autocommit_non_locking: 0
1 row in set (0.01 sec)
```

>查找trx_mysql_thread_id  

3.找到trx_mysql_thread_id手动删除
```shell
kill trx_mysql_thread_id;
```