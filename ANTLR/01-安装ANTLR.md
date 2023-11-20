## 安装 ANTLR 

### 背景  

### 安装 ANTLR 
>env: Mac, antlr version: 4.13.1, jdk version: 11

`官网原文` 
>All users should download the ANTLR tool itself and then choose a runtime target below, unless you are using Java which is built into the tool jar.
See Release Notes, README.md, Getting started, ANTLR 4 grammar repository, and How to build ANTLR itself.    

安装步骤可完全参考 `https://github.com/antlr/antlr4/blob/master/doc/getting-started.md`的 `Installation` 章节，因本人是 Mac 环境，所以仅摘要 Unix下的安装步骤       

>注意 JDK版本，antlr-4.13.1-xxx.jar 要求 JDK 版本 >= 11。   

* 根据上面官网介绍，访问 `https://www.antlr.org/download.html`, 先下载`ANTLR tool itself`。         
```
wget https://www.antlr.org/download/antlr-4.13.1-complete.jar 
```

* 配置 ANTLR 的环境变量  
编辑 .bash_profile 文件，配置 antlr 的环境变量  
```
export CLASSPATH=".:/Users/a/Software/antlr/antlr-4.13.1-complete.jar:$CLASSPATH"  #设置antlr的jar包到环境变量
alias antlr4='java -Xmx500M -cp "/Users/a/Software/antlr/antlr-4.13.1-complete.jar:$CLASSPATH" org.antlr.v4.Tool'  #快速运行ANTLR的解释器
alias grun='java -Xmx500M -cp "/Users/a/Software/antlr/antlr-4.13.1-complete.jar:$CLASSPATH" org.antlr.v4.gui.TestRig' #快速运行ANTLR测试工具
```

在终端上输入 `antlr4`, 可返回以下信息： 
```
ANTLR Parser Generator  Version 4.13.1
 -o ___              specify output directory where all output is generated
 -lib ___            specify location of grammars, tokens files
 -atn                generate rule augmented transition network diagrams
 -encoding ___       specify grammar file encoding; e.g., euc-jp
 -message-format ___ specify output style for messages in antlr, gnu, vs2005
 -long-messages      show exception details when available for errors and warnings
 -listener           generate parse tree listener (default)
 -no-listener        don't generate parse tree listener
 -visitor            generate parse tree visitor
 -no-visitor         don't generate parse tree visitor (default)
 -package ___        specify a package/namespace for the generated code
 -depend             generate file dependencies
 -D<option>=value    set/override a grammar-level option
 -Werror             treat warnings as errors
 -XdbgST             launch StringTemplate visualizer on generated code
 -XdbgSTWait         wait for STViz to close before continuing
 -Xforce-atn         use the ATN simulator for all predictions
 -Xlog               dump lots of logging info to antlr-timestamp.log
 -Xexact-output-dir  all output goes into -o dir regardless of paths/package
```

### 运行 Github 文档中的简单示例    
在临时目录下创建 `Expr.g4` 文件，内容如下： 
```

```


refer   
1.https://www.antlr.org/download.html   
2.https://github.com/antlr/antlr4/blob/master/doc/getting-started.md   
3."The Definitive ANTLR 4 Reference" 
4.《ANTLR 4 权威指南》  