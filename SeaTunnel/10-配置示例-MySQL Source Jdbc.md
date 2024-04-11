# 配置示例 - MySQL Source Jdbc

```json
Jdbc {
    url = "jdbc:mysql://192.168.0.202:3306/yzhou_test?serverTimezone=GMT%2b8"
    driver = "com.mysql.cj.jdbc.Driver"
    connection_check_timeout_sec = 100
    user = "root"
    password = "123456"
    query = "select * from yzhou_test01"
  }
```