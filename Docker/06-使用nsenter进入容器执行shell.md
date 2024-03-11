## 使用 nsenter 进入容器执行 shell  

### 场景介绍	
通过 nsenter 进入容器查看 IP 配置   		

### 案例演示	
查看 正在运行的 Docker 
```shell
yzhou@yzhou:~/gomain-services/Cncamp$ sudo docker ps 
CONTAINER ID   IMAGE                             COMMAND                  CREATED         STATUS         PORTS                               NAMES
9c14ed2ba191   xinzhuxiansheng/httpserver:v1.0   "/bin/sh -c /httpser…"   7 minutes ago   Up 7 minutes   0.0.0.0:80->80/tcp, :::80->80/tcp   httserver
yzhou@yzhou:~/gomain-services/Cncamp$  
```

### 使用 nsenter 
`nsenter` 是一个 Linux 命令行工具，用于进入到一个或多个 Linux 名字空间。它常被用于调试和管理容器，尤其是在没有提供交云接口或者你想要直接与容器的底层系统交互时。使用 `nsenter` 进入容器并查看 IP 配置，你需要执行以下步骤：				

#### 1. 获取容器的 PID
首先，你需要找到运行容器的进程 ID（PID）。如果你正在使用 Docker，可以使用 `docker inspect` 命令获取容器的 PID：			
```bash
docker inspect --format '{{ .State.Pid }}' <容器ID或名称>
```
替换 `<容器ID或名称>` 为你的容器的实际 ID 或名称。				

#### 2. 使用 `nsenter` 进入容器		
获得 PID 后，你可以使用 `nsenter` 命令并指定需要加入的名字空间。对于查看 IP 配置，通常需要进入网络名字空间：			
```bash
sudo nsenter --target <PID> --net ip addr
```
替换 `<PID>` 为步骤 1 中找到的容器的 PID。这个命令会显示容器内部的网络接口配置信息，类似于在宿主机上运行 `ip addr` 命令的输出。				

示例输出结果：	
```shell
yzhou@yzhou:~/gomain-services/Cncamp$ sudo nsenter --target 20149 --net ip addr 
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
8: eth0@if9: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:11:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.2/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever
yzhou@yzhou:~/gomain-services/Cncamp$
```

### 注意事项

- `nsenter` 通常需要 `sudo` 或 root 权限来执行。				
- 确保你的系统支持 `nsenter` 命令。大多数现代 Linux 发行版都自带此工具，但在某些环境中可能需要单独安装。				
- 使用 `nsenter` 可能会对容器内部造成影响。请确保你了解正在执行的操作，并在生产环境中谨慎使用。				
