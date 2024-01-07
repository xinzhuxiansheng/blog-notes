## Docker 命令 docker cp 拷贝文件       

### 将 docker 内部文件 拷贝到外部 
```
docker cp nginx:/etc/nginx/conf.d/default.conf default.conf     
```

### 将 外部文件 拷贝到 docker 内部  
```
docker cp default.conf  nginx:/etc/nginx/conf.d/default.conf   
```