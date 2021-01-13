--In Blog
--Tags: Kafka

# Kafka开启远程调试

**1. 在kafka-run-class.sh配置远程链接参数**
`添加:`
```shell
-Xdebug -Xnoagent -Djava.compiler=NONE -Xrunjdwp:transport=dt_socket,address=8080,server=y,suspend=y 
```

`kafka-run-class.sh`
```shell
# Launch mode
if [ "x$DAEMON_MODE" = "xtrue" ]; then
  nohup $JAVA -Xdebug -Xnoagent -Djava.compiler=NONE -Xrunjdwp:transport=dt_socket,address=8080,server=y,suspend=y  $KAFKA_HEAP_OPTS $KAFKA_JVM_PERFORMANCE_OPTS $KAFKA_GC_LOG_OPTS $KAFKA_JMX_OPTS $KAFKA_LOG4J_OPTS -cp $CLASSPATH $KAFKA_OPTS "$@" > "$CONSOLE_OUTPUT_FILE" 2>&1 < /dev/null &
else
  exec $JAVA $KAFKA_HEAP_OPTS $KAFKA_JVM_PERFORMANCE_OPTS $KAFKA_GC_LOG_OPTS $KAFKA_JMX_OPTS $KAFKA_LOG4J_OPTS -cp $CLASSPATH $KAFKA_OPTS "$@"
fi
```

**2. Idea配置远程调试参数**


