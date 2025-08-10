# 单机部署   

## 配置环境
```shell
wget https://dl.min.io/server/minio/release/linux-amd64/minio
cp minio /usr/local/bin/  
chmod +x /usr/local/bin/minio
mkdir /root/minio/data/
```

## 设置账号密码 
```bash
export MINIO_ACCESS_KEY=minio
export MINIO_SECRET_KEY=minio123
```

## 启动命令
```
nohup /usr/local/bin/minio server --address :9000 --console-address :9001 /root/minio/data > /root/minio/minio.log &
```

地址：http://192.168.0.135:9001

refer       
1.https://blog.csdn.net/qq_44697754/article/details/133303180      
