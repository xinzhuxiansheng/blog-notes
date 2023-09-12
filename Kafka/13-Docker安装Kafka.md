## Docker安装Kafka

>本篇涉及到的Kafka部署在Docker上，使用的是`wurstmeister/kafka-docker`。截止 2023.09.11 目前支持基于Zookeeper部署。  


### 单节点部署  
`docker-compose-single-broker.yml`      
```yaml
version: '3.8'
services:
  zookeeper:
    image: wurstmeister/zookeeper
    ports:
      - "2181:2181"
  kafka:
    image: wurstmeister/kafka
    depends_on: [ zookeeper ]
    ports:
      - "9092:9092"
    environment:
      KAFKA_ADVERTISED_HOST_NAME: 192.168.0.9
      KAFKA_CREATE_TOPICS: "yzhoutp01:1:1"
      KAFKA_ZOOKEEPER_CONNECT: zookeeper:2181
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock
```

kafka 环境变量：        
* KAFKA_ADVERTISED_HOST_NAME：你可以修改主机名来以匹配docker主机IP。注意：如果要运行多个Broker，请不要使用localhost或127.0.0.1作为主机ip。      
* KAFKA_CREATE_TOPICS：kafka-docker在创建期间自动在kafka中创建主题，例如 test:1:1 表示主题test包含1个分区和1个副本。    
* KAFKA_ZOOKEEPER_CONNECT：现在是强制的环境变量，表示kafka的zookeeper connect string。  
* kafka depends-on：指定 zookeeper 在 kafka 前面启动。  
* kafka volumes：指定 docker   

**进入Kafka Docker**
```
docker exec -it [docker-id] /bin/bash
```

**创建Topic**
```
$KAFKA_HOME/bin/kafka-topics.sh --create --topic yzhoutp02 --partitions 2 --zookeeper kafka-zookeeper-1:2181 --replication-factor 1
```
注意 --zookeeper 后面的参数为 [zookeeper的docker名称]

**查看Topic**
```
$KAFKA_HOME/bin/kafka-topics.sh --zookeeper kafka-zookeeper-1:2181 --describe --topic yzhoutp02
```

**发送消息**
```
$KAFKA_HOME/bin/kafka-console-producer.sh --topic=yzhoutp02 --broker-list kafka-kafka-1:9092
```

**消费消息**
```
$KAFKA_HOME/bin/kafka-console-consumer.sh --bootstrap-server kafka-kafka-1:9092 --from-beginning --topic yzhoutp02
```

### 启动管理界面    
```
docker run -itd --name=kafka-manager -p 9000:9000 -e ZK_HOSTS="[宿主机IP]:2181" sheepkiller/kafka-manager
```
关于`sheepkiller/kafka-manager` 请参考`https://hub.docker.com/r/sheepkiller/kafka-manager`  



refer   
1.https://github.com/wurstmeister/kafka-docker  
2.https://www.jianshu.com/p/0edcc3addf3f    
3.https://hub.docker.com/r/sheepkiller/kafka-manager    




