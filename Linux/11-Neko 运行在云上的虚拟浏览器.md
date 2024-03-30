## Neko 运行在云上的虚拟浏览器  

### 背景    
在实际开发过程中，经常会出现网络隔离等情况, 如图中所示，`腾讯云K8s集群`对于`本地Mac 开发机` 来说只能通过公网访问，那么想访问K8s集群 Pod服务，最好的方法是 将服务绑定的Service使用NodePort通信，这样我可以在`本地 Mac 开发机`直接访问，可这种与实际生产过程中还是会有些偏差，在实际生产过程中，非特殊情况是不允许使用NodePort通信，而是使用`ClusterIP`,所以这需要我们使用`ingress-nginx`做域名绑定到Service，从而让服务无需关心IP是什么。
![neko01](http://img.xinzhuxiansheng.com/blogimgs/linux/neko01.png)    


而我的`腾讯云 K8s集群`是腾讯云机器，我只好用公网IP访问，如果我在开发机中配置 hosts映射后访问会遭到腾讯云拦截，内容如下：        

![neko02](http://img.xinzhuxiansheng.com/blogimgs/linux/neko02.png)        

这导致我在本地开发机无法使用域名映射方式访问，当然与`腾讯云K8s集群`同在内网的`腾讯云机器`肯定不受影响。    

如何像生产服务那样使用ingress来访问？   

### neko 介绍   
首先，你需要做的是先访问`https://github.com/m1k1o/neko`网址，查看一个动态图：   
`https://raw.githubusercontent.com/m1k1o/neko/master/docs/_media/intro.gif` 

至少你能看出，在浏览器里面还嵌套了一个`浏览器`, 所以我在`本地 Mac开发机`通过公网IP访问 `腾讯云机器`的某个端口，此时我的浏览器会帮我嵌套一个浏览器，而此时嵌套浏览器环境是`腾讯云机器`，所以访问嵌套浏览器其实就等同于`腾讯云内网`访问。 我想介绍到这，大家就应该理解差不多了，其实就类似于远程桌面一样，只不过这次通过浏览器连接，并且只能控制远程机器的浏览器。    

### neko 安装   
neko的安装过程算是简单的了，它是基于Docker-Compose部署，详细的安装步骤请参考`https://neko.m1k1o.net/#/getting-started/quick-start`。    

**下面我们重点介绍下 neko的 docker-compose.yaml**
```yaml
version: "3.4"
services:
  neko:
    image: "m1k1o/neko:firefox"
    extra_hosts:
      - "hello.k8s.com:192.168.xxx.xxx"
      - "flink.k8s.io:192.168.xxx.xxx"
    restart: "unless-stopped"
    shm_size: "2gb"
    ports:
      - "8082:8080"
      - "52000-52100:52000-52100/udp"
    environment:
      NEKO_SCREEN: 1920x1080@30
      NEKO_PASSWORD: neko
      NEKO_PASSWORD_ADMIN: admin
      NEKO_EPR: 52000-52100
      NEKO_ICELITE: 1
      NEKO_LOCKS: control file_transfer
      NEKO_VIDEO_BITRATE: 3000
      NEKO_FILE_TRANSFER_ENABLED: true
      NEKO_FILE_TRANSFER_PATH: /home/neko/Downloads
      NEKO_NAT1TO1: 192.168.0.140
```    

```bash
docker-compose -f xx.yaml up -d
```

**image**: 可参考官网选择不同浏览器镜像 
![neko03](http://img.xinzhuxiansheng.com/blogimgs/linux/neko03.png)        

**extra_hosts**：配置 hosts域名映射 

**ports**: `- "8082:8080"`, 8082表示宿主机映射端口，8080是pod端口，所以这里可能需要调整宿主机映射端口，可能会出现端口占用。 

其他请参考以下配置即可：
NEKO_LOCKS: control file_transfer
NEKO_VIDEO_BITRATE: 3000
NEKO_FILE_TRANSFER_ENABLED: true
NEKO_FILE_TRANSFER_PATH: /home/neko/Downloads


### neko 访问   
访问公网IP+8082, 直接访问即可,默认账号密码：admin/admin
![neko04](http://img.xinzhuxiansheng.com/blogimgs/linux/neko04.png)    

`点击键盘`即可控制浏览器啦  
![neko05](http://img.xinzhuxiansheng.com/blogimgs/linux/neko05.png)

这里特别说明一个字符串复制操作，当希望把`本地 Mac开发机`的字符串 copy到虚拟浏览器中，请使用 neko。 
![neko06](http://img.xinzhuxiansheng.com/blogimgs/linux/neko06.png)    

先将copy的字符串paste到 剪贴板内，在进行 copy，paste。 这里其实特别像`系统剪贴板与vim剪贴板`。 不是同一个缓冲区层，无法互相拷贝。   


> neko 真香！！！


refer   
1.https://github.com/m1k1o/neko         
2.https://www.zyglq.cn/posts/neko-remote-browser.html   


