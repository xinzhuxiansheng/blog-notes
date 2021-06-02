## 本地编译打包
```shell
gradlew clean releaseTarGz -x test
gradle clean releaseTarGz -x test
```


## 打包kafka-clients jar 并上传到私服

```shell
gradle clients:install

mvn deploy:deploy-file -DgroupId=org.apache.kafka -DartifactId=kafka-clients -Dversion=2.2.1-auto-1.0-SNAPSHOT -Dpackaging=jar -Dfile=D:/code/xxxxx_code/bigdata/kafka-2.2.1/clients/build/libs/kafka-clients-2.2.1-auto-1.0-SNAPSHOT.jar -Durl=http://xxxxxx.com/nexus/content/repositories/snapshots/ -DrepositoryId=axxxhome -DpomFile=D:/code/xxxxx_code/bigdata/kafka-2.2.1/clients/build/poms/pom-default.xml
```