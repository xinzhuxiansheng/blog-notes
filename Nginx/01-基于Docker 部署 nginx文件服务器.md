## 基于 Docker 部署 nginx 文件服务器    

### 拉取 nginx 镜像
```
docker pull nginx 
```

### nginx.conf 配置 
```
user  root;
worker_processes  auto;

error_log  /var/log/nginx/error.log info;
pid        /var/run/nginx.pid;


events {
    worker_connections  1024;
}


http {
    include       /etc/nginx/mime.types;
    default_type  application/octet-stream;

    log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
                      '$status $body_bytes_sent "$http_referer" '
                      '"$http_user_agent" "$http_x_forwarded_for"';

    access_log  /var/log/nginx/access.log  main;

    sendfile        on;
    #tcp_nopush     on;

    keepalive_timeout  65;

    server {
        listen 886 default_server;
        listen [::]:886 default_server;
        server_name _;
        charset utf-8;
        root    /home/all_packages;                  # 文件存放目录
        autoindex on;

        # 下载
        location / {
            autoindex_format html;                # 首页格式为HTML
            autoindex_exact_size off;             # 文件大小自动换算
            autoindex_localtime on;               # 按照服务器时间显示文件时间

            default_type application/octet-stream;# 将当前目录中所有文件的默认MIME类型设置为
                                                # application/octet-stream

            if ($request_filename ~* ^.*?\.(txt|doc|pdf|rar|gz|zip|docx|exe|xlsx|ppt|pptx)$){
                # 当文件格式为上述格式时，将头字段属性Content-Disposition的值设置为"attachment"
                add_header Content-Disposition: 'attachment;';
            }
            sendfile on;                          # 开启零复制文件传输功能
            sendfile_max_chunk 1m;                # 每个sendfile调用的最大传输量为1MB
            tcp_nopush on;                        # 启用最小传输限制功能

    #       aio on;                               # 启用异步传输
            directio 5m;                          # 当文件大于5MB时以直接读取磁盘的方式读取文件
            directio_alignment 4096;              # 与磁盘的文件系统对齐
            output_buffers 4 32k;                 # 文件输出的缓冲区大小为128KB

    #       limit_rate 1m;                        # 限制下载速度为1MB
    #       limit_rate_after 2m;                  # 当客户端下载速度达到2MB时进入限速模式
            max_ranges 4096;                      # 客户端执行范围读取的最大值是4096B
            send_timeout 20s;                     # 客户端引发传输超时时间为20s
            postpone_output 2048;                 # 当缓冲区的数据达到2048B时再向客户端发送
            chunked_transfer_encoding on;         # 启用分块传输标识
        }
    }
}
```     


### 启动镜像    

```
docker run -d --name nginx_files_server -p 886:886 -v /root/TMP:/home/all_packages -v /root/yzhou/nginx/nginx.conf:/etc/nginx/nginx.conf nginx
```

>注意：     
* /root/TMP 存放要下载文件的目录    
* /root/yzhou/nginx/nginx.conf 是自定义的 nginx 配置文件   

