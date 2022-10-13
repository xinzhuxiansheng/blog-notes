
## 使用Openjdk8和Maven打包出错

refer： https://blog.csdn.net/c654528593/article/details/121974027

### 异常信息：
```shell
java.lang.RuntimeException: Unexpected error: java.security.InvalidAlgorithmParameterException: the trustAnchors parameter must be non-empty
```

### 解决
经google后发现是https安全验证的导致的错误，openjdk缺少安全证书，通过以下方式追加
```
mvn clean package -Dmaven.wagon.http.ssl.insecure=true -Dmaven.wagon.http.ssl.allowall=true
```