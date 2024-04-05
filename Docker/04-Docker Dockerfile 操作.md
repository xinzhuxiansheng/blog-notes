
## 打包镜像     
```shell
docker build -t yzhou/k8s-client-test:0.0.1 .
```

## 推送镜像 
注意，若推送到私服，注意需登录
```shell
docker push yzhou/k8s-client-test:0.0.1
```