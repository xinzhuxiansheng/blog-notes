#!/bin/bash

/usr/bin/wget http://containerfiles.corpautohome.com/ci-config/kafka/kafka_2.12-2.2.1.tar.gz -N -P /data/sysdir/servers/

/usr/bin/tar zxvf /data/sysdir/servers/kafka_2.12-2.2.1.tar.gz -C /data/sysdir/servers/

chmod 777 -R /data/sysdir/servers/kafka_2.12-2.2.1/bin/*.sh

/usr/bin/wget http://containerfiles.corpautohome.com/ci-config/kafka/server_nginx_test.properties -N -P /data/sysdir/servers/kafka_2.12-2.2.1/config/

CustomTagIFconfig=`/usr/sbin/ifconfig eth0 | grep inet | grep netmask | grep broadcast | grep 10 | awk -F " " '{print $2}'`
sed -i "s/custom-tag-ifconfig/$CustomTagIFconfig/g" /data/sysdir/servers/kafka_2.12-2.2.1/config/server_nginx_test.properties
sed -i "s/custom-tag-hostname/${HOSTNAME##*-}/g" /data/sysdir/servers/kafka_2.12-2.2.1/config/server_nginx_test.properties


JMX_PORT=9999 /data/sysdir/servers/kafka_2.12-2.2.1/bin/kafka-server-start.sh -daemon /data/sysdir/servers/kafka_2.12-2.2.1/config/server_nginx_test.properties ;/usr/sbin/sshd ; sleep 36500d
