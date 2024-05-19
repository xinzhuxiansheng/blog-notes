# Akkajob - Worker 构建      

## 引言     


## 启动 
1.从配置文件中读取 重试次数  
2.初始化一个 `ScheduledThreadPoolExecutor` 线程池，基于 initializeExecutor.scheduleWithFixedDelay 进行重试      




 

refer   
1."Akka实战：快速构建高可用分布式应用"      
2.https://github.com/open-job/openjob           
3.https://www.baeldung.com/akka-with-spring         
4.https://doc.akka.io/docs/akka/current/actors.html#dependency-injection       
5.https://github.com/typesafehub/activator-akka-java-spring          
6.https://doc.akka.io/docs/akka/current/persistence-journals.html       
7.https://index.scala-lang.org/search?topics=akka       