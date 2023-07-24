

### seatunnel-cluster.sh    

 apache-seatunnel-2.3.2 grep "APP_MAIN" bin/seatunnel-cluster.sh
APP_MAIN="org.apache.seatunnel.core.starter.seatunnel.SeaTunnelServer"
 nohup java ${JAVA_OPTS} -cp ${CLASS_PATH} ${APP_MAIN} ${args} > "$OUT" 200<&- 2>&1 < /dev/null &
 java ${JAVA_OPTS} -cp ${CLASS_PATH} ${APP_MAIN} ${args}