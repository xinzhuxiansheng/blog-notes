

### #与$区别

1. #{} 会自动加上单引号
2. ${} 不会加


### 使用模糊查询

```xml
select * from student where name like '%#{张三}%'
```

上面的sql语句使用了模糊查询，但是这个sql在mybatis中会报错，因为没法对其进行预编译，但是改为$时就正确了，但是这样会造成sql注入的漏洞，

因此推荐下面的写法：   
```xml
select * from student where name like concat('%', #{张三}, '%"}
```