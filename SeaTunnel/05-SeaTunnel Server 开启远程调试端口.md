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

>注意，seatunnel 官方给的调试参数 suspend=y，是在服务启动时，就开始等待调试端口，若是调试非启动阶段，可将参数设置为 suspend=n 


## Idea 配置 Remote JVM Debug   

Use module classpath:  `seatunnel-starter`      

>再重新启动。   
