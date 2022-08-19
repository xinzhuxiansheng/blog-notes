
## Helm

Helm中有四大基本概念：helm客户端、Char、Repository、Release

* helm客户端：主要是命令行，负责管理本地的Charts、repositories，发送Chart，实例安装、查询、卸载等操作。
* Chart：是创建一个应用的信息集合，包括各种Kubernetes对象的配置模板、参数定义、依赖关系、文档说明等，形成一个压缩包.tar。Char是应用部署的自包含逻辑单元。可以将Chart想象成apt、yum中的软件安装包，同时Char模板可以发布到专门的repository。
* Repository: Chart仓库。类似于docker的镜像仓库，可以使用Harbor作为Chart的仓库。
* Release：release是Chart运行的实例，使用helm install命令在Kubernetes集群中部署的Chart就生成了一个Release。一个Chart能够多次安装到同一个集群，每次安装都是一个Release。Release基于名称空间进行隔离。


