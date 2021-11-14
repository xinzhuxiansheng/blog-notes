# JavaCC入门篇(1) - 介绍及安装

## 介绍
JavaCC（Java Compiler Compiler）是一个开源的语法分析器生成器和词法分析器生成器。JavaCC根据输入的文法生成由Java语言编写的分析器。    
和YACC类似，JavaCC根据由EBNF格式撰写的形式文法生成语法分析器。不同的是，JavaCC生成的是自顶向下语法分析器，由于可以向前搜寻k个字符，所以可以用来分析LL(k)文法。同时，JavaCC生成词法分析器的方式和Lex也很像。 
另外，JavaCC还提供JJTree等辅助工具帮助使用者构建语法树  

>维基百科 - https://zh.wikipedia.org/wiki/JavaCC  
JavaCC官网: https://javacc.github.io/javacc/


## 本地安装
以下按照官网步骤: https://javacc.github.io/javacc/#download 
```
Download & Installation

JavaCC 7.0.10 is our latest stable release.

    JavaCC 7.0.10 - (Source (zip), Source (tar.gz), Binaries, Javadocs, Release Notes)

All JavaCC releases are available via GitHub and Maven including checksums and cryptographic signatures.

For all previous releases, please see stable releases.

The GitHub 8.0 branch contains the next generation of JavaCC that splits the frontend – the JavaCC parser – from the backends – the code generator targeted for Java, C++ &and C# –. Status of JavaCC is experimental and not production ready.
Installation

To install JavaCC, navigate to the download directory and type:

$ unzip javacc-7.0.10.zip
or
$ tar xvf javacc-7.0.10.tar.gz

Then place the binary javacc-7.0.10.jar in a new target/ folder, and rename to javacc.jar.

Once you have completed installation add the scripts/ directory in the JavaCC installation to your PATH. The JavaCC, JJTree, and JJDoc invocation scripts/executables reside in this directory.

On UNIX based systems, the scripts may not be executable immediately. This can be solved by using the command from the javacc-7.0.10/ directory:

chmod +x scripts/javacc
```

```shell
# 下载安装包，并解压
tar -zxf javacc-7.0.10.tar.gz   

# 进入 javacc-javacc-7.0.10目录，创建target/ 并且将 bootstrap/javacc.jar 拷贝到target/目录下 
cd javacc-javacc-7.0.10/
mkdir target/
cp bootstrap/javacc.jar target/

# 配置环境变量 
sudo vim .bash_profile
# 以下是添加内容
#set javacc environment
JAVACC_HOME=/Users/yiche/Software/javacc-javacc-7.0.10
PATH=$PATH:$JAVACC_HOME/scripts
export JAVACC_HOME PATH

# 生效环境变量
source .bash_profile
``` 

以上步骤操作完，即可在终端输入`javacc` 

>在官网特别介绍：在基于UNIX的系统上，脚本可能无法立即执行，通过 chmod +x scripts/javacc授权 

### Idea插件安装

Idea安装JavaCC插件，请以下参考官网内容:  
```
IntelliJ IDEA

The IntelliJ IDE supports Maven out of the box and offers a plugin for JavaCC development. 

* IntelliJ download: https://www.jetbrains.com/idea/
* IntelliJ JavaCC Plugin: https://plugins.jetbrains.com/plugin/11431-javacc/
``` 

