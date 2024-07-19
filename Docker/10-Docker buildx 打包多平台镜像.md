# Docker buildx 打包多平台镜像   

## 引言   
在近几年来，ARM 架构的服务随着热潮起来，大多数服务的 Docker Image 需要开发者提供支持多个架构，例如`linux/amd64`,`linux/arm64/v8`。估计有不少人像我一样，没太在意这件事，是因为安装服务的机器可以访问外网，使用`docker pull`就解决了，而对于镜像架构选择是由 Docker 帮我们做了选择，若你的机器是 x86 则会自动下载`linux/amd64`。当然也有特殊的案例，例如一个 Docker Image只提供了一种架构（linux/amd64）,那要是在 arm架构的机器上执行 docker pull，很显然会拉取到 linux/amd64的，这的确会让人头疼。 

接下来，接着介绍 Docker Image 多平台的事。                      

## 了解 Docker Image 的 OS/ARCH  
我以 DockerHub中的 Flink 镜像为例，访问`https://hub.docker.com/_/flink/tags`,查看 Flink镜像的 TAG 信息，可看到它的`OS/ARCH`：   
![buildx01](images/buildx01.png)   

从图片可以看到 TAG:scala_2.12-java8 的 Flink 镜像，它的`OS/ARCH`分别包含 `linux/amd64`,`linux/arm64`。针对`docker pull`可以跟随系统配置自动拉取对应的架构的 Docker Image来说，也可以通过`--platform`参数来指定 OS 信息，如下：   
```shell
docker pull --platform=linux/amd64 flink:1.15.1-java8   
```

拉取镜像后，可通过如下命令来验证 Docker Image 架构信息：    
```shell
docker inspect flink:1.15.1-java8 | grep -i 'architecture'   
```

Output log:   
```bash
[root@VM-64-111-centos ~]# docker inspect flink:1.15.1-java8 | grep -i 'architecture'
        "Architecture": "amd64",  
```

## Docker buildx 安装  
`buildx` 是 Docker 的功能之一，若使用不到可以不用安装，验证环境是否已安装 buildx，可执行`docker buildx`, 查看它提示的是`Usage:  docker buildx [OPTIONS] COMMAND` 还是`Usage:	docker [OPTIONS] COMMAND`, 我想你已经看出它的差异了。  

**已安装**  
```bash
[root@VM-64-111-centos ~]# docker buildx   
Extended build capabilities with BuildKit   

Usage:  docker buildx [OPTIONS] COMMAND   
```

**未安装**   
```bash
[root@VM-64-112-centos ~]# docker buildx version

Usage:	docker [OPTIONS] COMMAND
```  

正对 CentOS 同学，可访问 Docker的安装官网 `https://docs.docker.com/engine/install/centos/`，若你的Docker 参考官网步骤，那基本上已安装 buildx，参考以下命令：  
```shell
sudo yum install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin   
```

若你在安装 Docker 时没有安装 `docker-buildx-plugin`，你可以卸载 Docker 重新安装，也可访问`https://github.com/docker/buildx?tab=readme-ov-file#installing` 单独安装 docker-buildx。 （而我选择的是重新安装 Docker）  

## 基于 buildx 打包多平台的 Docker Image  
>注意该章节内容主要是实操，并不涉及到 buildx的架构原理。   

首先你要对 `docker buildx`有个初步的认识是：它的命令是独立的，可访问`https://docs.docker.com/reference/cli/docker/buildx/`了解，并且它的打包不是不是基于宿主机的，而是先启动一个 Docker 服务，基于指定的 buildx Docker 服务进行打包。那么接下来，我们开始进行实操：   

### 1.docker buildx ls 查看环境的设定     
```shell
[root@VM-64-111-centos ~]# docker buildx ls
NAME/NODE                 DRIVER/ENDPOINT                   STATUS    BUILDKIT   PLATFORMS
build-node-example*       docker-container
 \_ build-node-example0    \_ unix:///var/run/docker.sock   running   v0.15.0    linux/arm64*, linux/amd64, linux/amd64/v2, linux/amd64/v3, linux/386
default                   docker
 \_ default                \_ default                       running   v0.13.2    linux/amd64, linux/amd64/v2, linux/amd64/v3, linux/386
```

安装完 buildx 后，会默认存在名为`default` buildx，注意它的`PLATFORMS`参数，它可能并不能满足你要的 OS架构要求，例如 `linux/arm64`,所以，需要重新创建 buildx 设置它的 `PLATFORMS`, 可能你会设置 docker hub 的代理。因为国内访问 docker hub确实挺难的。    

### 2.docker buildx create 创建新的 buildx     

* 创建 `config.toml` 设置代理地址 
vim config.toml，注意`mirrors`参数值是需要根据你的代理地址修改的。示例内容如下：  
```bash
# registry configures a new Docker register used for cache import or output.
[registry."docker.io"]
  mirrors = ["https://docker.1panel.live"]
  # http = true
  # insecure = true
  # ca=["/etc/config/myca.pem"]
  # [[registry."docker.io".keypair]]
  #   key="/etc/config/key.pem"
  #   cert="/etc/config/cert.pem"
```


[root@VM-64-145-centos dp-realtime-computation]# cat config.toml
# registry configures a new Docker register used for cache import or output.
[registry."docker.io"]
  mirrors = ["https://docker.1panel.live"]
  # http = true
  # insecure = true
  # ca=["/etc/config/myca.pem"]
  # [[registry."docker.io".keypair]]
  #   key="/etc/config/key.pem"
  #   cert="/etc/config/cert.pem"

```shell

```




refer       
1.https://docs.docker.com/reference/cli/docker/buildx/      
2.https://docs.docker.com/engine/install/centos/       
3.https://github.com/docker/buildx    
4.https://github.com/docker/buildx/issues/136  

