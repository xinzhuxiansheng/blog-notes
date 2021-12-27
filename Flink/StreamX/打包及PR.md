
## 本地打包
mvn clean install -DskipTests -Pwebapp

mvn clean install -DskipTests -Denv=prod

## 校验 checkstyle
mvn checkstyle:check 

