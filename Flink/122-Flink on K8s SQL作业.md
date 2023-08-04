## Flink on K8s SQL作业 

一、flink-sql程序开发和本地测试
程序功能示意图


 

1、开发SqlRunner.java，功能是读取外部传入的sql脚本，解析出sql语句，然后以flink sql方式执行

package com.yale;

import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.table.api.TableResult;
import org.apache.flink.table.api.bridge.java.StreamTableEnvironment;
import org.apache.flink.util.FileUtils;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class SqlRunner {

    private static final Logger LOG = LoggerFactory.getLogger(SqlRunner.class);

    private static final String STATEMENT_DELIMITER = ";"; // a statement should end with `;`
    private static final String LINE_DELIMITER = "\n";

    private static final String COMMENT_PATTERN = "(--.*)|(((\\/\\*)+?[\\w\\W]+?(\\*\\/)+))";

    public static void main(String[] args) throws Exception {
        String scriptFilePath = "simple.sql";
        if (args.length == 1) {
            scriptFilePath = args[0];
        }

        String script = FileUtils.readFileUtf8(new File(scriptFilePath));
        List<String> statements = parseStatements(script);

        //建立Stream环境，设置并行度为1
        StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment().setParallelism(1);
        //建立Table环境
        StreamTableEnvironment tableEnvironment = StreamTableEnvironment.create(env);
        TableResult tableResult = null;
        for (String statement : statements) {
            LOG.info("Executing:\n{}", statement);
            System.out.println("Executing: " + statement);
            tableResult = tableEnvironment.executeSql(statement);
        }
//        ** executeSql是一个异步的接口，在idea里面跑的话，直接就结束了，需要手动拿到那个executeSql的返回的TableResult
//        tableResult.getJobClient().get().getJobExecutionResult().get();
    }

    public static List<String> parseStatements(String script) {
        String formatted = formatSqlFile(script).replaceAll(COMMENT_PATTERN, "");
        List<String> statements = new ArrayList<String>();

        StringBuilder current = null;
        boolean statementSet = false;
        for (String line : formatted.split("\n")) {
            String trimmed = line.trim();
            if (trimmed == null || trimmed.length() < 1) {
                continue;
            }
            if (current == null) {
                current = new StringBuilder();
            }
            if (trimmed.startsWith("EXECUTE STATEMENT SET")) {
                statementSet = true;
            }
            current.append(trimmed);
            current.append("\n");
            if (trimmed.endsWith(STATEMENT_DELIMITER)) {
                if (!statementSet || trimmed.equals("END;")) {
                    // SQL语句不能以分号结尾
                    statements.add(current.toString().replace(";", ""));
                    current = null;
                    statementSet = false;
                }
            }
        }
        return statements;
    }

    public static String formatSqlFile(String content) {
        String trimmed = content.trim();
        StringBuilder formatted = new StringBuilder();
        formatted.append(trimmed);
        if (!trimmed.endsWith(STATEMENT_DELIMITER)) {
            formatted.append(STATEMENT_DELIMITER);
        }
        formatted.append(LINE_DELIMITER);
        return formatted.toString();
    }

}



2、编写sql脚本文件
simple.sql

-- source到kafka

CREATE TABLE user_log
(
    user_id     VARCHAR,
    item_id     VARCHAR,
    category_id VARCHAR,
    behavior    VARCHAR,
    ts          TIMESTAMP
) WITH (
      'connector' = 'kafka',
      'topic' = 'user_behavior',
      'properties.bootstrap.servers' = 'k8s01:9092,k8s02:9092,k8s03:9092',
      'properties.group.id' = 'testGroup1',
      'scan.startup.mode' = 'earliest-offset',
      'format' = 'json'
      );

-- sink到mysql
CREATE TABLE pvuv_sink (
    dt VARCHAR,
    pv BIGINT,
    uv BIGINT,
    primary key (dt) not enforced
) WITH (
      'connector' = 'jdbc',
      'url' = 'jdbc:mysql://k8s02:3306/flink_test?characterEncoding=utf8&connectTimeout=1000&socketTimeout=3000&autoReconnect=true&useUnicode=true&useSSL=false&serverTimezone=UTC',
      'table-name' = 'pvuv_sink',
      'username' = 'root',
      'password' = '123456'
      );

-- 打印到屏幕
-- CREATE TABLE pvuv_sink
-- (
--     dt VARCHAR,
--     pv BIGINT,
--     uv BIGINT
-- ) WITH (
--       'connector' = 'print'
--       );


-- 把统计结果数据保存到MySQL

INSERT INTO pvuv_sink
SELECT DATE_FORMAT(ts, 'yyyy-MM-dd HH:00') dt,
       COUNT(*)                AS          pv,
       COUNT(DISTINCT user_id) AS          uv
FROM user_log
GROUP BY DATE_FORMAT(ts, 'yyyy-MM-dd HH:00');


3、创建kafka topic
kafka-topics.sh --bootstrap-server k8s01:9092,k8s02:9092,k8s03:9092 --topic user_behavior --create --partitions 1

4、创建表
CREATE TABLE pvuv_sink (
    dt VARCHAR(30),
    pv BIGINT,
    uv BIGINT,
    primary key (dt)
)

5、本地测试，运行程序，发送kafka消息
kafka-console-producer.sh --broker-list k8s01:9092,k8s02:9092,k8s03:9092 --topic user_behavior

{"user_id": "543462", "item_id":"1715", "category_id": "1464116", "behavior": "pv", "ts": "2023-03-26 01:00:00"}
{"user_id": "543462", "item_id":"1715", "category_id": "1464116", "behavior": "pv", "ts": "2023-03-26 02:00:00"}
{"user_id": "662867", "item_id":"2244074", "category_id": "1575622", "behavior": "pv", "ts": "2023-03-26 03:36:00"}
{"user_id": "662867", "item_id":"2244074", "category_id": "1575622", "behavior": "pv", "ts": "2023-03-26 04:36:00"}


6、打包，先修改maven pom.xml， 把相关的依赖范围改为provided，减少jar包体积。 当然不改为provided也是可以的。

<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-kafka_${scala.binary.version}</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-jdbc_${scala.binary.version}</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>mysql</groupId>
    <artifactId>mysql-connector-java</artifactId>
    <version>5.1.49</version>
    <scope>provided</scope>
</dependency>


<!-- Flink SQL 相关 -->
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-connector-files</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-table-api-java</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-table-api-java-bridge_${scala.binary.version}</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-table-planner-blink_${scala.binary.version}</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>

<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-csv</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>
<dependency>
    <groupId>org.apache.flink</groupId>
    <artifactId>flink-json</artifactId>
    <version>${flink.version}</version>
    <scope>provided</scope>
</dependency>




7、上传jar包和simple.sql到flink-jar-pvc对应的pv目录下

二、构建flink-sql镜像
因为JobManager启动的时候就要去连接kafka和mysql，而flink自有lib目录下没有kafka和mysql的jar包，所以需要新构建flink-sql的镜像，将kafka和mysql的jar包放到/opt/flint/lib目录下
1、下载依赖Jar包
https://nightlies.apache.org/flink/flink-docs-release-1.13/docs/connectors/table/jdbc/

cd /root/my_dockerfile/flink-sql
wget https://repo.maven.apache.org/maven2/org/apache/flink/flink-sql-connector-kafka_2.12/1.13.6/flink-sql-connector-kafka_2.12-1.13.6.jar
wget https://repo.maven.apache.org/maven2/org/apache/flink/flink-connector-jdbc_2.12/1.13.6/flink-connector-jdbc_2.12-1.13.6.jar
wget https://repo.maven.apache.org/maven2/mysql/mysql-connector-java/5.1.49/mysql-connector-java-5.1.49.jar

2、编写Dockerfile

FROM flink:1.13.6
WORKDIR /opt/flink
COPY flink-connector-jdbc_2.12-1.13.6.jar /opt/flink/lib/flink-connector-jdbc_2.12-1.13.6.jar
COPY flink-sql-connector-kafka_2.11-1.13.6.jar /opt/flink/lib/flink-sql-connector-kafka_2.11-1.13.6.jar
COPY mysql-connector-java-5.1.49.jar /opt/flink/lib/mysql-connector-java-5.1.49.jar
RUN chown -R flink:flink /opt/flink/lib
ENTRYPOINT ["/docker-entrypoint.sh"]
EXPOSE 6123 8081
CMD ["help"]



3、分发
xsync flink-sql/

4、构建（k8s03~k8s06执行）
cd /root/my_dockerfile/flink-sql
docker rmi flink-sql:1.13.6
docker build -f Dockerfile -t flink-sql:1.13.6 .

5、查看确认
docker images | grep flink-sql

三、提交flink-sql作业
1、编写flink sql作业yaml文件sql-application.yaml
cd /root/flink-operator/flink-kubernetes-operator-1.3.1/examples
vi sql-application.yaml

apiVersion: flink.apache.org/v1beta1
kind: FlinkDeployment
metadata:
  namespace: flink
  name: sql-application
spec:
  image: flink-sql:1.13.6  # 使用flink-sql的镜像
  flinkVersion: v1_13
  imagePullPolicy: IfNotPresent  # 镜像拉取策略，本地没有则从仓库拉取
  ingress:   # ingress配置，用于访问flink web页面
    template: "flink.k8s.io/{{namespace}}/{{name}}(/|$)(.*)"
    className: "nginx"
    annotations:
      nginx.ingress.kubernetes.io/rewrite-target: "/$2"
  flinkConfiguration:
    taskmanager.numberOfTaskSlots: "2"
    classloader.resolve-order: parent-first  # 默认是child-first，必须改为parent-first，先加载flink自带的Jar，要不然容易出现各种类型不匹配问题
  serviceAccount: flink
  jobManager:
    resource:
      memory: "1024m"
      cpu: 1
  taskManager:
    resource:
      memory: "1024m"
      cpu: 1
  podTemplate:
    spec:
      hostAliases:
        - ip: "192.168.56.80"
          hostnames:
            - "k8s01"
        - ip: "192.168.56.81"
          hostnames:
            - "k8s02"
        - ip: "192.168.56.82"
          hostnames:
            - "k8s03"
      containers:
        - name: flink-main-container
          env:
            - name: TZ
              value: Asia/Shanghai
          volumeMounts:
            - name: flink-jar  # 挂载nfs上的jar
              mountPath: /opt/flink/jar
            - name: flink-log
              mountPath: /opt/flink/log
      volumes:
        - name: flink-jar
          persistentVolumeClaim:
            claimName: flink-jar-pvc
        - name: flink-log
          persistentVolumeClaim:
            claimName: flink-log-pvc
  job:
    jarURI: local:///opt/flink/jar/flink-on-k8s-demo-1.0-SNAPSHOT-jar-with-dependencies.jar # 使用pv方式挂载jar包
    entryClass: com.yale.SqlRunner
    args:   # 传递到作业main方法的参数
      - "/opt/flink/jar/simple.sql"
    parallelism: 1
    upgradeMode: stateless



（2）提交作业
kubectl apply -f sql-application.yaml

（3）查看
kubectl get po -n flink -owide

http://flink.k8s.io:30502/flink/sql-application/#/overview

（4）测试，发送kafka消息
kafka-console-producer.sh --broker-list k8s01:9092,k8s02:9092,k8s03:9092 --topic user_behavior
{"user_id": "543462", "item_id":"1715", "category_id": "1464116", "behavior": "pv", "ts": "2023-03-26 11:00:00"}
{"user_id": "543462", "item_id":"1715", "category_id": "1464116", "behavior": "pv", "ts": "2023-03-26 12:00:00"}
{"user_id": "662867", "item_id":"2244074", "category_id": "1575622", "behavior": "pv", "ts": "2023-03-26 13:36:00"}
{"user_id": "662867", "item_id":"2244074", "category_id": "1575622", "behavior": "pv", "ts": "2023-03-26 14:36:00"}