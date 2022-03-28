
## 我与StreamX的初识及搭建开发环境

### StreamX的初识
2021年 基于Flink的流式计算平台在国内蓬勃发展且Flink的Release迭代速度也是飞快，几乎每月都有小版本的修复和间隔2,3月的大版本发布。我在2021年11月份开始实时计算平台的调研，它解决的是“减少用户使用Flink流式计算的开发成本”，将`flink.sh或sql-client.sh`作业提交等转到Web进行操作。之前我对Flink的开发有些了解，但仅限于Flink相关Jar的API使用及Job的Submit，可针对平台性的开发经验几乎没有，不过这也让我能从用户的角度去思考，“实时计算平台”应该有哪些features，由于缺少较多的知识盲区（专业度）或者从另一个角度去思考是学习别人的经验竟可能少走弯路，所以我从GitHub检索出一些具有相似性的“实时计算平台”开源项目，例如 Dlink，flink-streaming-platform-web 等等,其中不乏很多是`feature项目`，我之所以这么称呼是因为它仅仅实现一小部分功能。而我需要是比较成熟的开源项目，这对于我去梳理`实时计算平台思维导图`特别有用，它会增强我对实时计算平台的系统性认知帮助我查缺补漏。 我也会去找我具有这方面经验的同事来沟通学习，他向我推荐了StreamX开源项目。  于是乎 .... 就有了接下来故事 - 成为`StreamX的开发者`。        

>StreamX 的初衷是为了让流处理更简单. 打造一个一站式大数据平台,流批一体,湖仓一体的解决方案。

StreamX简介中的features正是我想要的，那就`将StreamX跑起来` -- 还好利用自己的开发经验以及阅读官网的DOC，将项目从git clone到import idea，再到local run起来，没有耗费太多时间，这里想对大家提个“tips”，用闲余的时间去阅读它的issue，这会帮助解决你在搭建、阅读代码遇到的疑惑同时也增加了你对这个项目的了解。 我在别人的issue中看到StreamX的作者的微信，于是乎 ....我加入了StreamX的大家庭（微信沟通群）。  

>了解新鲜事物，一定要看官网的DOC，它的目的是帮助你花较少的时间去了解，使用它。

后面在StreamX功能测试过程中，发现了一些问题，陆续参与issue的讨论及相应的PR提交 (开源这事，坚持融入，会收获更多，这里需要感谢我的老领导 :) )


### 搭建开发环境
大家可以访问 https://www.streamxhub.com/docs/user-guide/deployment 阅读用户手册。了解平台部署和开发环境搭建，下面我也会介绍开发环境搭建,这里对DOC的一些解读

**配置本地环境**    
涉及到的Env需要：     
Maven 3.6+              
node v14.18.1，npm 8.1.4 (本人测试OK)            
JDK 1.8+        

`注意`  
需要特别注意：StreamX提供2种打包
* 混合打包
mvn clean install -DskipTests -Pwebapp（在1.2.1之后版本 混合打包，将service和web一起打包，直接执行start.sh即可）      
* 前后端分开打包
```shell
# 后端打包
cd streamx
mvn clean install -DskipTests

# 前端打包
cd streamx/streamx-console/streamx-console-webapp
npm install
npm run build
```

后端安装完成之后就看到最终的工程文件，位于 streamx/streamx-console/streamx-console-service/target/streamx-console-service-x.x.x-bin.tar.gz

**表结构初始化**        
StreamX提供两种sql（streamx/streamx-console/streamx-console-service/src/assembly/script），`final.sql` 提供全新的tables的初始化，`upgrade.sql` 提供版本迭代升级所需的增量table或者field的初始化。

新用户直接执行`final.sql`初始化即可。


**Idea配置后端**
如下图所示, 在`VM options`添加"-Dapp.home=...(编译好的安装包路径)"
![RunConfigurations](http://img.xinzhuxiansheng.com/blogimgs/streamx/RunConfigurations.jpg)  

>特别注意：app.home是指StreamX后端编译的安装包解压后路径，此步骤一定要配置

**VsCode配置前端**
`npm run serve`运行前端代码。


剩下的就是测试StreamX即可

