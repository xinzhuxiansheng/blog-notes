**`正文`**
[TOC]

## 安装Elasticsearch
请参考官网文档 https://www.elastic.co/guide/en/elasticsearch/reference/6.3/_installation.html

### elasticsearch.yml配置

### 用户及群组设置
```shell
#添加群组
groupadd elsearchgroup

#添加用户
useradd elsearchuser -g elsearchgroup

#将文件授权给用户
chown -R elsearchuser:elsearchgroup  文件夹名称 或者文件
```

### Q&A
* max file descriptors [4096] for elasticsearch process is too low, increase to at least [65536]
    >问题描述：elasticsearch 用户拥有的可创建文件描述的权限太低，至少需要65536；
    **`解决方法：`**
    ```shell
    #切换root用户修改`limits.conf` 文件
    vim /etc/security/limits.conf

    #在最后追加下面内容
    *** hard nofile 65536
    *** soft nofile 65536
    # ***是启动ES的用户，eg： elsearchuser hard nofile 65536
    ```


* max virtual memory areas vm.max_map_count [65530] is too low, increase to at least [262144]
    >问题描述：进程中内存映射区域的最大数量太低，至少需要262144
    **`解决方法：`**
    ```shell
    #切换到root用户修改配置sysctl.conf
    vim /etc/sysctl.conf 

    #在最后追加下面内容
    vm.max_map_count=655360
    #执行
    sysctl -p
    #重启elasticsearch 服务
    ``` 



## Plugins Install

>配置跨域
编译elasticsearch配置文件：
action.auto_create_index: true
http.cors.enabled: true
http.cors.allow-origin: "*"

### sql
* github地址：https://github.com/NLPchina/elasticsearch-sql
* 安装node
* 安装sql 插件
* On elasticsearch 5.x/6.x, download and extract site.(https://github.com/NLPchina/elasticsearch-sql/releases/download/5.4.1.0/es-sql-site-standalone.zip)
    Then start the web front-end like this:
    ```shell
    cd site-server
    npm install express --save
    node node-server.js
    ```

### cerebro
* github地址：https://github.com/lmenezes/cerebro
    ```shell
    Download from https://github.com/lmenezes/cerebro/releases
    Extract files
    Run bin/cerebro(or bin/cerebro.bat if on Windows)
    Access on http://localhost:9000
    ```
### kibana
略
