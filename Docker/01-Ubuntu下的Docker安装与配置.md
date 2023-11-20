## Ubuntu 下的 Docker 安装与配置 

### 安装 Docker 
```shell
apt install docker.io -y 
```

### 修改配置 

* 修改镜像源地址    

```shell
# 进入目录
cd /etc/docker/

# 编辑配置
vim daemon.json
```

配置内容如下：      
```json
{
    "registry-mirrors":
    [
        "https://docker.mirrors.ustc.edu.cn/"
    ]
}
```

>注意，针对 Docker 配置修改也可通过 修改 `/lib/systemd/system/docker.service` 配置文件。   

