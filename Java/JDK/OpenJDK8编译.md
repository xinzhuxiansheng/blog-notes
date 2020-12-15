--In Blog
--Tags: OpenJDK,Java

# 编译 OpenJDK8

## 源码下载
OpenJDK的 Source code的下载方式
1) Github OpenJDK: https://github.com/openjdk/jdk
2) Github AdoptOpenJDK: https://github.com/AdoptOpenJDK/openjdk-jdk8u
3) Mercuial OpenJDK: http://hg.openjdk.java.net/jdk8u

任何一种方式都可以，请自行下载

## 编译环境搭建
> 一定要阅读我下面分享得链接文档 (前期别在Baidu，Google上面浪费过多时间,当然出现问题了，你再搜也来的及)
1) README-builds.html (https://github.com/openjdk/jdk/blob/jdk8-b120/README-builds.html ,若查看不方便，`请将文件保存成HTML即可本地浏览`)
README-builds.html 是OpenJDK的Build文档，`没有什么比这更详细的，请一定一定要好好阅读一遍，甚至多遍，对于后期编译，有更深层次理解`；
特别注意 **THIS IS A MAJOR RE-WRITE of this document.**" 特别强调`编译OpenJDK已经不在需要Ant`，`不在支持配置ALT_* 环境变量`；

>The build is now a "configure && make" style build
Any GNU make 3.81 or newer should work
The build should scale, i.e. more processors should cause the build to be done in less wall-clock time
Nested or recursive make invocations have been significantly reduced, as has the total fork/exec or spawning of sub processes during the build
Windows MKS usage is no longer supported
Windows Visual Studio vsvars*.bat and vcvars*.bat files are run automatically
Ant is no longer used when building the OpenJDK
Use of ALT_* environment variables for configuring the build is no longer supported

再重点阅读下 `Building` Linux方面的部分

2) Supported Build Platforms (https://wiki.openjdk.java.net/display/Build/Supported+Build+Platforms )
`我们总是站在巨人的肩膀上,干更伟大的事`，wiki中特么强调了，已经在 哪些OS Type,哪些 Compiler 编译支持是否ok.
迅速拉到页面最下面:  Other JDK 8 build platforms:
Linux x86 and x86_64	(green star)	Ubuntu 14.04	gcc 4.8.2	Works flawlessly

#### Build Environment Require:
* OS Type: Ubuntu 14.04
* Compiler: gcc 4.8.2

3) Docker构建编译环境
我想这个时候，没有比Docker再合适不过了。

请参考 Docker常用命令[Docker](http://xinzhuxiansheng.com/articleDetail?id=2)

4) 其他帮助文档
Adopt OpenJDK的giitbook:
https://adoptopenjdk.gitbooks.io/adoptopenjdk-getting-started-kit/content/en/

## 编译
```shell
#我用的是ubuntu14.04
#安装gcc
sudo apt-get  build-dep  gcc
gcc -version
#安装编译依赖库
sudo apt-get install libX11-dev libxext-dev libxrender-dev libxtst-dev libxt-dev libcups2-dev libfreetype6-dev

#检查依赖库是否缺失
#进入OpenJDK的代码目录，configure会检查你的缺失的依赖包，缺啥，安装啥就行
bash configure
#编译 ，等待安装好就行
make all
```


## QA
1.**Q**: configure: error: Could not find freetype! You might be able to fix this by running 'sudo apt-get install libfreetype6-dev'.

**A**: 我已经安装了 libfreetype6-dev,可执行bash configure 仍然提示我需要安装 `libfreetype6-dev`,请使用下面命令
```shell
./configure --with-freetype-include=/usr/include/freetype2 --with-freetype-lib=/usr/lib/x86_64-linux-gnu
```
