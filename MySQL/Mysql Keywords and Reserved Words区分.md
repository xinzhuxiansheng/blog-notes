
# 介绍
请阅读Mysql doc： https://dev.mysql.com/doc/refman/8.0/en/keywords.html

# Q&A
**1. ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'order' at line 1**      
order是关键字，所以 `select * from order;`要添加 **`** 最后变成：   
```sql
select * from `order`;
```