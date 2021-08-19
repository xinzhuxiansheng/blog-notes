# 常用命令

## Mysql远程连接
mysql -h localhost -P 3306 -u root -p

>指定编码格式  --default-character-set=utf8
```shell
mysql -h localhost -P 3306 -u root -p --default-character-set=utf8
```

## 创建数据库
mysql>create databse [库名];
mysql>use [库名]

## 使用外部文件执行
mysql>source [sql脚本文件的路径全名] 或 Mysql>\. [sql脚本文件的路径全名]，示例：
mysql>source C:\test.sql 或者 \. C:\test.sql

>注意.sql文件 windows的路径将 \ 换成 /

## 删除数据库
msql>drop database [库名]