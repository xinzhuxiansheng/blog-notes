**正文**

## 启动命令
```shell
docker run -d --name graphite --restart=always -p 80:80 -p 2003-2004:2003-2004 -p 2023-2024:2023-2024 -p 8125:8125/udp -p 8126:8126 graphiteapp/graphite-statsd
```


## Visualize the Data
Open Graphite in a browser.

    http://localhost/dashboard
    http://localhost/render?from=-10mins&until=now&target=stats.example


参考地址：
https://github.com/graphite-project/docker-graphite-statsd