# 常用命令

## Mysql远程连接
mysql -h localhost -P 3306 -u root -p

## 创建数据库
mysql>create databse [库名];
mysql>use [库名]

## 使用外部文件执行
mysql>source [sql脚本文件的路径全名] 或 Mysql>\. [sql脚本文件的路径全名]，示例：
mysql>source C:\test.sql 或者 \. C:\test.sql

## 删除数据库
msql>drop database [库名]