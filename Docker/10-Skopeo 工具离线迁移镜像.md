# Skopeo 工具离线迁移镜像         

>基于 Docker Buildx 打包的双架构镜像   

## 背景  
现在有两个 Docker 私服，分别是`A Docker 私服`，`B Docker 私服`, 他们所在的网络环境导致他们网络互不相通，那么如下图中的 `A CentOS Docker`，`B CentOS Docker` 无法从对方的 Docker 私服中拉取镜像。     
![skopeo01](http://img.xinzhuxiansheng.com/blogimgs/docker/skopeo01.png)   

现在的需求是：假设存在某个镜像(是双架构镜像，该镜像包括 amd64，arm64),它的地址`xxx.com/service/servera:0.0.1`  要在`A CentOS Docker`和`B CentOS Docker`环境都能拉取这个镜像，然后再启动服务。      

>注意: 镜像是双镜像，是通过 docker buildx 打包出来的，具体操作步骤可访问我的blog http://xinzhuxiansheng.com/articleDetail/136 了解 Docker Buildx 的使用步骤。 如果是单镜像直接使用 docker save，docker load即可。     

## 预期
先将`A Docker 私服`中的镜像导出，再通过离线文件再导入到`B Docker 私服`去。 针对双架构的镜像，从`A Docker 私服`直接导出离线文件，再使用离线文件导入到`B Docker 私服`去，这部分我并没有实践成功，我的思路是：由之前 Docker Buildx 打包双架构镜像通过`--push`参数推送到`A Docker 私服`去，调整成 通过`--output type=oci,dest=xxxx.tar`打包成本地离线文件。再使用离线文件导入到`B Docker 私服`。 当然我现在的做法导致我需要执行`两次 docker buildx build`打包双架构镜像， 一次是为了推送到`A Docker 私服`，一次是为了打包成离线文件。     
![skopeo02](http://img.xinzhuxiansheng.com/blogimgs/docker/skopeo02.png)          

基于我成功的案例，下面介绍如何使用 Skopeo 工具将双架构镜像离线文件导入到`B Docker 私服`。           


## docker buildx --output   
--output 可以配置docker buildx 构建结果输出方式。而在本案例中针对双架构的离线文件则使用 oci 格式存储的，所以示例命令如下：  
```bash
docker buildx build --provenance=false --platform=linux/arm64,linux/amd64 -t xxx.com/service/servera:0.0.1 --output type=oci,dest=xxxx.tar .   
```

命令执行完后，当前目录会生成一个`xxxx.tar` 文件。    


## 安装 skopeo，导入双架构镜像   
1.在`B CentOS Docker`所在的机器中，安装 skopeo 工具。  
```bash 
yum install skopeo    
```

2.将离线镜像文件解压到 `oci-image` 目录(文件夹名字随意定)   
```bash
tar -xf xxxx.tar -C oci-image/   
```

3.使用 copy 命令，导入镜像      
使用 copy 命令时，需添加 `--all`参数，它将 amd64,arm64 全部导出，`--dest-tls-verify=false`参数作用是在推送到`B Docker 私服`时，使用 http 协议（在不支持 https 协议时使用）     
```bash
skopeo copy --all --dest-tls-verify=false oci:/xxxxx/oci-image docker://xxx.com/service/servera:0.0.1
```

注意： 在skopeo copy --all 推送时，若`B Docker 私服`包含 auth校验，还需提前执行 docker login。   


以上操作就完成了 离线镜像文件导入到其他镜像仓库的步骤。    