创建 client.properties
```
sasl.mechanism=GSSAPI
security.protocol=SASL_PLAINTEXT
sasl.kerberos.service.name=kafka
kerberos.domain.name=kafka30-all
sasl.jaas.config=com.sun.security.auth.module.Krb5LoginModule required \
    useKeyTab=true \
    storeKey=true \
    keyTab="/root/kafka/kafka.keytab" \
    principal="kafka/kafka30-all@KAFKA.COM" \
    renewTGT=true \
    useTicketCache=true;
```


./kafka-topics.sh --create --bootstrap-server xxxx:9092 --replication-factor 1 --partitions 1 --topic yzhoutp01 --command-config client.properties