## 使用 Makefile 构建 Docker 镜像 

### 引言 
当服务使用 Docker 部署时，经常会涉及到手动敲打 docker命令来构建镜像、上传镜像等相关操作，例如 build、push，每次修改代码后，都要经常需要重复输入跟镜像相关的命令，这有些烦躁，后面仅仅通过 shell history命名 提高开发效率。  

在后面系统性学习 Docker 时，了解到 Makefile能更好的提高开发和部署效率。         

### 介绍    
在 Docker 和软件开发领域，"Docker" 和 "Makefile" 是两个不同的概念，但它们可以一起使用以提高开发和部署流程的效率。下面分别解释这两个概念，以及它们如何结合使用。 

Docker 是一个开放源代码的软件平台，用于构建、运行、管理容器化的应用程序。容器让开发者可以打包应用及其所有依赖项到一个封闭的包中，这样应用就可以在任何支持 Docker 的环境中运行，确保了从开发到生产环境的一致性。Docker 使用 Dockerfile 来定义构建容器镜像的步骤。一个 Dockerfile 包含了创建和运行 Docker 容器所需的所有指令。        

Makefile 是一个包含了一系列定义了如何编译和构建程序的指令集的文件。它被 make 工具使用，make 工具读取 Makefile 文件中的指令来自动化编译和构建过程，从而减少手动编译和构建的重复劳动。Makefile 可以定义多个目标（target），每个目标关联一组命令，这些命令当目标被 make 工具调用时执行。           

>在 Docker 的上下文中，Makefile 可以用来简化构建和管理 Docker 镜像及容器的过程。你可以在 Makefile 中定义一些自定义命令（目标），这些命令执行相关的 Docker 命令，如构建镜像、启动容器、停止容器等。          

### 示例说明    

#### 目录结构   
➜  chapter2 git:(main) ✗ tree .    
.
├── Dockerfile
├── HttpClient.go
├── HttpServer.go
├── Makefile
└── bin
    └── amd64
        └── httpserver

#### Dockerfile 
```
FROM ubuntu
ENV MY_SERVICE_PORT=80
ENV MY_SERVICE_PORT1=80
ENV MY_SERVICE_PORT2=80
ENV MY_SERVICE_PORT3=80
LABEL multi.label1="value1" multi.label2="value2" other="value3"
ADD bin/amd64/httpserver /httpserver
EXPOSE 80
ENTRYPOINT /httpserver
``` 

#### Makefile   
```
export tag=v1.0
root:
	export ROOT=github.com/xinzhuxiansheng/golang

build:
	echo "building httpserver binary"
	mkdir -p bin/amd64
	CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -o bin/amd64/httpserver .

release: build
	echo "building httpserver container"
	docker build -t xinzhuxiansheng/httpserver:${tag} .

push: release
	echo "pushing xinzhuxiansheng/httpserver"
	docker push xinzhuxiansheng/httpserver:v1.0
``` 

后面在执行 make 命令，如下：    
```shell
# 编译 go 代码
make build 

# build 镜像  
make release 

# push 镜像
make push
```  

>注意：示例中的 Makefile ，有些像"执行链"。 当执行`make push`,会将 `make build、make release、make push`按照顺序执行，当执行`make release` 同理。  

此时 docker push 是推到 dockerhub仓库中。   

#### 启动示例中的 httpserver 
```shell
docker run --name httserver -d -p 80:80 xinzhuxiansheng/httpserver:v1.0 
```

