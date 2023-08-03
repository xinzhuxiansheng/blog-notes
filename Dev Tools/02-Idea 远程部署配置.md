## Idea 远程部署配置    

### 背景    
最近遇到一个项目无法在本地Debug，所以就需要将程序发布到目标机器才能运行，再启动脚本中添加远程debug端口配置， 每次调试过程中，我都是先在本地提交到仓库，然后在目标机器机器中拉取新代码。 后来发现这个过程有很大的弊端，我还是开发期间，频繁的commit，会导致一个功能有大量的commit记录，这显然对CodeReview是很不友好的。  

后来了解到IDEA 支持将本地代码通过`SFTP`协议 远程部署到目标机器，下面我来介绍下如何在IDEA中配置。    

### 远程部署 (Deployment)   

**1.** 打开 IDEA菜单栏 `Tools` -> `Deployment` -> `Configuration`
![deployment01](http://img.xinzhuxiansheng.com/blogimgs/ide/deployment01.png)    

**2.** 配置参数 
Type: 选择 `SFTP`   
SSH configuration:  用户@ip:port, 

![deployment02](http://img.xinzhuxiansheng.com/blogimgs/ide/deployment02.png)    
我的目标机器登录时是需要配置私有key。
Host: 是 ip 
Port: 是 22 
Username: 是机器用户       
Authentication type: 选择 `Key pair`    
Private key file: 选择 私钥路径地址 

**3.**  配置Mappings    
![deployment03](http://img.xinzhuxiansheng.com/blogimgs/ide/deployment03.png)  
Local path: 是本地项目代码目录  
Deployment path: 是目标机器代码存放的目录   

### 配置自动部署
打开 IDEA菜单栏 `Tools` -> `Deployment`  -> `Automatic Upload(Always)`  

你也可以选择`手动部署` 在IDEA中 选择项目根目录，`点击右键` 选择 `Deployment`即可。  

这样就帮我完成了目标机器代码同步了，不用再使用 git commit， 等开发好就可以一次提交代码仓库。    

