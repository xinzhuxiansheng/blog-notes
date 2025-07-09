# MySQL 2 MySQL 示例  

## mysql2mysql.json  
```json
{
  "job": {
    "content": [{
      "reader": {
        "name": "mysqlreader",
        "parameter": {
          "username": "root",
          "password": "123456",
          "connection": [{
            "querySql": ["select * from yzhou_test02;"],
            "jdbcUrl": ["jdbc:mysql://192.168.0.201:3306/yzhou_test?useSSL=false"]
          }]
        }
      },
      "writer": {
        "name": "mysqlwriter",
        "parameter": {
          "username": "root",
          "password": "123456",
          "column": ["`id`", "`name`", "`address`", "`ext_field01`"],
          "connection": [{
            "table": ["yzhou_test03"],
            "jdbcUrl": "jdbc:mysql://192.168.0.201:3306/yzhou_test?useSSL=false"
          }]
        }
      }
    }],
    "setting": {
      "speed": {
        "channel": 1,
        "record": 1000
      },
      "errorLimit": {
        "record": 0,
        "percentage": 0
      }
    }
  },
  "core": {
    "transport": {
      "channel": {
        "speed": {
          "channel": 1,
          "record": 10
        }
      }
    }
  }
}
```  

## 执行命令  
```bash
python /root/yzhou/datax/bin/datax.py /root/yzhou/datax-job/mysql2mysql.json
```