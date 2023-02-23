
## SpringBoot整合WebSocket运行测试类报错

将@SpringBootTest()注解，修改成
```java
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT)
```