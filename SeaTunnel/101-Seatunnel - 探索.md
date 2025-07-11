# Seatunnel - 探索 Seatunnel Runs On Flink 的

## 引文  

执行 seatunnel 脚本启动任务  
```bash 
/root/seatunnel/apache-seatunnel-2.3.12-SNAPSHOT/bin/start-seatunnel-flink-15-connector-v2.sh --config /root/seatunnel/jobs/mysql2mysql.config
```

执行 Flink Run 命令  
```bash
${FLINK_HOME}/bin/flink run -c org.apache.seatunnel.core.starter.flink.SeaTunnelFlink /root/seatunnel/apache-seatunnel-2.3.12-SNAPSHOT/starter/seatunnel-flink-15-starter.jar --config /root/seatunnel/jobs/mysql2mysql.config --name SeaTunnel --deploy-mode run
```

