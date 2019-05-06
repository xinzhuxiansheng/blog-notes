**`正文`**
[TOC]

### server eureka服务多个实例启动(HA)，互相注册
```yml
service: 
  name: springcloud-eureka

#配置端口和path路径
server:
  #port: 8842
  session:
    tracking-modes: cookie
  servlet-path: /
  max-http-header-size: 1048576
  tomcat:
    max-connections: 3000
    max-http-post-size: 1048576
    max-threads: 1000
  
  
#配置当前profiles
spring:
  profiles:
    active: local
  mvc:
    throw-exception-if-no-handler-found: true
  application:
    name: springcloud-eureka

#配置日志
logging:
  config: classpath:log4j2.xml
 

#配置server eureka
eureka:
  instance:
    hostname: localhost
  client:
    register-with-eureka: false
    fetch-registry: false
    serviceUrl:
      defaultZone: http://${eureka.instance.hostname}:${server.port}/eureka/

#配置多个server eureka实例
---

#配置日志
logging:
  config: classpath:log4j2.xml

spring:
  profiles: localserver01
eureka:
  instance:
    hostname: localhost
    preferIpAddress: true #通过ip通信，不使用主机名
    ip-address: 127.0.0.1
  client:
    service-url:
      defaultZone: http://localhost:8844/eureka/
    #让自己也注册到其他的server eureka 服务中
    register-with-eureka: true
    fetch-registry: false
server:
  port: 8843

#配置 eureka web 账号密码登录  若集成了安全校验，则添加上配置
security:
  basic:
    enabled: false
  user:
    name: yzhou
    password: 123456

---

#配置日志
logging:
  config: classpath:log4j2.xml

spring:
  profiles: localserver02
eureka:
  instance:
    hostname: localhost
    preferIpAddress: true #通过ip通信，不使用主机名
    ip-address: 127.0.0.1
  client:
    service-url:
      defaultZone: http://localhost:8843/eureka/
    #让自己也注册到其他的server eureka 服务中
    register-with-eureka: true
    fetch-registry: false
server:
  port: 8844

#配置 eureka web 账号密码登录 若集成了安全校验，则添加上配置
security:
  basic:
    enabled: false
  user:
    name: yzhou
    password: 123456

```


