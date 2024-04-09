# SeaTunnel Server 开启远程调试端口     

## 修改 bin/seatunnel-cluster.sh 
修改 `bin/seatunnel-cluster.sh`, 将下面 `JAVA_OPTS`参数注释放开即可。       

```shell
# Server Debug Config
# Usage instructions:
# If you need to debug your code in cluster mode, please enable this configuration option and listen to the specified
# port in your IDE. After that, you can happily debug your code.
# JAVA_OPTS="${JAVA_OPTS} -Xdebug -Xrunjdwp:server=y,transport=dt_socket,address=5001,suspend=y"
```

>再重新启动。   
