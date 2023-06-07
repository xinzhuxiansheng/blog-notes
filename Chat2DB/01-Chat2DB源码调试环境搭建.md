## Chat2DB 源码调试环境搭建 

>Chat2DB version: 1.0.9 

### 引言    
首次看到Chat2DB项目，并不是因为集成了AIGC的能力，而是与博主的一段项目开发经历相似，在企业开发中，RDS平台是支撑企业MySQL平台化一个产物，其中就包含一个特定的功能`Web Console`，类似于阿里云，腾讯云的MySQL Web查询终端，这样用户就无需再使用Shell登录MySQL，简化了他们使用MySQL的成本，企业也能做到审计相关的一些业务。  接下来我们就想搭建好它的源码调试环境，为后面的章节做准备。  

>注意 环境要求：    
    java运行 Open JDK 17
    Node运行环境Node16 Node.js.


### 源码搭建    
首先克隆Chat2DB，并且从`tags/v1.0.9`创建分支， 参考`README.md`的本地调试章节，前后端分开启动。

```shell
# 克隆代码
git clone git@github.com:alibaba/Chat2DB.git

# 从tags 创建分支
git checkout tags/v1.0.9 -b newBranch
```

启动前端：  
```shell
$ cd Chat2DB/ali-dbhub-client
$ npm install 
$ npm run start
$ # 打开 http://127.0.0.1:10821 开启前端调试
$ # 注：前端页面完全赖服务，所以前端同学调试也需要把后端项目跑起来  
```


启动后端： 
根据官方给的启动命名，可以得出在IDEA中我们启动`ali-dbhub-server-start`模块的`Application#main`即可  
```shell
$ cd ../ali-dbhub-server
$ mvn clean install # 需要安装maven 3.8以上版本
$ cd ali-dbhub-server/ali-dbhub-server-start/target/
$ java -jar -Dchatgpt.apiKey=xxxxx ali-dbhub-server-start.jar  # 启动应用 chatgpt.apiKey 需要输入ChatGPT的key,如果不输入无法使用AIGC功能
$ # 打开 http://127.0.0.1:10821 开启调试 注：需要进行前端安装
```

目前文档介绍，`ali-dbhub-server-start`模块启动并不依赖DB组件。  


>文档提示，需配置XSwitch浏览器插件，来做URL转发。  

```
但是前端调试需要映射下资源，可以下载XSwitch (https://chrome.google.com/webstore/detail/xswitch/idkjhjggpffolpidfkikidcokdkdaogg),添加以下配置文件

{
  "proxy": [
    [
      "http://127.0.0.1:10821/(.*).js$",
      "http://127.0.0.1:8001/$1.js",
    ],
    [
      "http://127.0.0.1:10821/(.*).css$",
      "http://127.0.0.1:8001/$1.css",
    ],
    [
      "http://127.0.0.1:10821/static/front/(.*)",
      "http://127.0.0.1:8001/$1",
    ],
    [
      "http://127.0.0.1:10821/static/(.*)$",
      "http://127.0.0.1:8001/static/$1",
    ],
  ],
}
```

完成以上操作，启动即可，别忘记在页面配置后台地址，如果所示：    
![envidedebug01](http://img.xinzhuxiansheng.com/blogimgs/chat2db/envidedebug01.png)  


### Q&A

1. Error: error:0308010C:digital envelope routines::unsupported
```
* Webpack █████████████████████████ building (10%) 0/2 entries 1/2 dependencies 0/1 modules 1 active
 node_modules\@umijs\preset-built-in\bundled\@pmmmwh\react-refresh-webpack-plugin\client\ReactRefreshEntry.js

node:internal/crypto/hash:71
  this[kHandle] = new _Hash(algorithm, xofLen);
                  ^

Error: error:0308010C:digital envelope routines::unsupported
    at new Hash (node:internal/crypto/hash:71:19)
    at Object.createHash (node:crypto:133:10)
    at BulkUpdateDecorator.hashFactory (E:\Code\Java\Chat2DB\ali-dbhub-client\node_modules\@umijs\deps\compiled\webpack\5\bundle5.js:184161:18)
    at OriginalSource.updateHash (E:\Code\Java\Chat2DB\ali-dbhub-client\node_modules\@umijs\deps\compiled\webpack-sources2\index.js:1:51038)
    at handleParseResult (E:\Code\Java\Chat2DB\ali-dbhub-client\node_modules\@umijs\deps\compiled\webpack\5\bundle5.js:116027:10)
    at E:\Code\Java\Chat2DB\ali-dbhub-client\node_modules\@umijs\deps\compiled\webpack\5\bundle5.js:116119:4
    at processResult (E:\Code\Java\Chat2DB\ali-dbhub-client\node_modules\@umijs\deps\compiled\webpack\5\bundle5.js:115836:11)

```

**A:**  
参考该issue（https://github.com/ant-design/ant-design-pro/issues/9272）    
```shell
# 注意 博主是windows环境，使用set设置临时环境变量，其他请参考 issue
# 修改 package.json
 "scripts": {
    "start": "SET NODE_OPTIONS=--openssl-legacy-provider && umi dev",  # 添加  SET NODE_OPTIONS=--openssl-legacy-provider &&
    "build": "cross-env UMI_ENV=test umi build",
    "build:electron": "cross-env UMI_ENV=desktop umi build && cp -r ./dist ./electron ",
    "postinstall": "umi generate tmp",
```
在 start参数添加`SET NODE_OPTIONS=--openssl-legacy-provider &&`, 再执行 npm run start。

> 博主用的是Node18，或者将Node降到16+。


refer
1.https://chat2db.opensource.alibaba.com   
