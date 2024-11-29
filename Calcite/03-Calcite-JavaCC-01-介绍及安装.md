# Calcite - JavaCC - 介绍及安装  

>javacc version: 7.0.13        

## 介绍    
我是通过 `Calcite` 才过来了解 JavaCC 的，Calcite 使用了 JavaCC 作为其 SQL 解析器的生成工具。而 JavaCC（Java Compiler Compiler）是一个开源的语法分析器生成器和词法分析器生成器。JavaCC 可以根据定义好的语法规则（通常在 .jj 文件中）生成 Java 代码，和 YACC 类似，JavaCC 根据由EBNF格式撰写的形式文法生成语法分析器。不同的是，JavaCC 生成的是`自顶向下`语法分析器，由于可以向前搜寻k个字符，所以可以用来分析LL(k)文法。同时，JavaCC 生成词法分析器的方式和 Lex 也很像。 另外，JavaCC还提供JJTree等辅助工具帮助使用者构建语法树，以上这些设计 Calcite 提供了灵活性和扩展性。             

以下是具体关系的说明：          
**1. JavaCC 在 Calcite 中的作用**               
Calcite 使用 JavaCC 来解析 SQL 查询。它定义了 SQL 语法规则，并通过 JavaCC 生成解析代码。该解析器是 Calcite 处理 SQL 的第一步。              
主要职责            
```bash
•	SQL 语法解析:          
•	将用户输入的 SQL 文本解析为抽象语法树（AST）。          
•	处理 SQL 语法和语义检查。       
•	识别和扩展 Calcite 支持的 SQL 方言。               
•	支持扩展性:              
•	Calcite 支持自定义 SQL 方言，JavaCC 的定义可以很容易扩展以适配新的语法需求。    
```                  

**2. Calcite 的 Parser.jj 文件**                
在 Calcite 的源码中，有一个核心的 JavaCC 定义文件 Parser.jj，它包含了 SQL 的语法规则。这个文件是 JavaCC 的输入，通过编译生成 SQL 解析器的 Java 类。                 
示例（`下面是 Calcite 源码中定义的 Parser.jj 文件`）：      
```bash
https://github.com/apache/calcite/blob/main/core/src/main/codegen/templates/Parser.jj   
```

**3. Calcite 的 SQL 解析流程**    
```bash
•	定义语法规则：
•	在 Parser.jj 中，定义标准 SQL 的语法规则，以及对 Calcite 特殊扩展的支持。
•	JavaCC 生成代码：
•	使用 JavaCC 生成相应的解析器 Java 类（通常是 SqlParser 等）。
•	解析输入：
•	用户输入的 SQL 查询会通过生成的解析器转化为 Calcite 内部的抽象语法树（AST）。
•	这棵树会被进一步优化为逻辑计划和物理计划。
```

**4. JavaCC 的优势对 Calcite 的贡献**  
```bash
•	灵活性：
•	JavaCC 的语法定义易于修改，Calcite 能够快速适配不同 SQL 方言。
•	高效生成解析器：
•	JavaCC 自动生成解析代码，避免了手工编写解析器的复杂性。
•	社区支持与成熟性：
•	JavaCC 是一个成熟的工具，功能稳定，能够很好地满足 Calcite 对 SQL 解析的需求。
```

**5. JavaCC 与 Calcite 的互补关系**   
```bash
•	Calcite 的需求：作为一个查询优化框架，Calcite 需要支持复杂的 SQL 语法，并且允许用户自定义扩展语法。
•	JavaCC 的适配：JavaCC 提供了语法灵活定义和解析器自动生成的能力，为 Calcite 提供了稳定的 SQL 解析工具。
```

**6. 实际开发中的使用场景**    
```bash
如果你需要扩展 Calcite 的 SQL 支持，可以通过以下步骤实现：      
1.	修改 Parser.jj 文件，定义新的 SQL 语法规则。        
2.	使用 JavaCC 编译生成新的解析器代码。        
3.	将解析器集成到 Calcite 的查询处理流程中。           
```
总结
Apache Calcite 与 JavaCC 的关系是框架与工具的关系：         
•	Calcite：一个强大的查询优化框架，负责整个 SQL 查询的解析、优化和执行。          
•	JavaCC：为 Calcite 提供了解析器生成能力，负责将 SQL 转换为 Calcite 能处理的内部结构。               

通过这种分工，Calcite 能够专注于优化和执行，而 JavaCC 提供了解析器生成的高效工具支持。          

>维基百科 - https://zh.wikipedia.org/wiki/JavaCC            
JavaCC官网: https://javacc.github.io/javacc/   


## 本地安装
以下按照官网步骤: https://javacc.github.io/javacc/#download       
```
Download

Download the latest stable release (at least the source and the binaries) in a so called download directory:
    JavaCC 7.0.13 - (Source (zip), Source (tar.gz), Binaries, Javadocs, Release Notes)
All JavaCC releases are available via GitHub and Maven including checksums and cryptographic signatures.

For all previous releases, please see stable releases.
Install

Once you have downloaded the files, navigate to the download directory and unzip the source file, this creating a so called JavaCC installation directory:
$ unzip javacc-7.0.13.zip
or
$ tar xvf javacc-7.0.13.tar.gz

Then move the binary file javacc-7.0.13.jar under the download directory in a new target/ directory under the installation directory, and rename it to javacc.jar.

Then add the scripts/ directory in the JavaCC installation directory to your PATH. The JavaCC, JJTree, and JJDoc invocation scripts/executables reside in this directory.

On UNIX based systems, the scripts may not be executable immediately. This can be solved by using the command from the javacc-7.0.13/ directory:

chmod +x scripts/javacc

Write your grammar and generate your parser
You can then create and edit a grammar file with your favorite text editor.
Then use the appropriate script for generating your parser from your grammar.   
```

### Linux 环境 
```shell
# 下载安装包，并解压
tar -zxf javacc-7.0.13.tar.gz   

# 进入 javacc-javacc-7.0.10目录，创建target/ 并且将 bootstrap/javacc.jar 拷贝到target/目录下 
cd javacc-javacc-7.0.13/
mkdir target/
cp bootstrap/javacc.jar target/

# 配置环境变量 
sudo vim .bash_profile
# 以下是添加内容
#set javacc environment
JAVACC_HOME=/Users/yc/Software/javacc-javacc-7.0.13
PATH=$PATH:$JAVACC_HOME/scripts
export JAVACC_HOME PATH

# 生效环境变量
source .bash_profile
``` 
>在官网特别介绍：在基于UNIX的系统上，脚本可能无法立即执行，通过 chmod +x scripts/javacc授权     

### Windows 环境 
```bash
1.下载完后，解压并配置 JAVACC_HOME 环境变量   
JAVACC_HOME:D:\Software\Dev\javacc-javacc-7.0.13   

2.在 PATH 中配置  
%JAVACC_HOME%\scripts

3.配置 
在根目录下创建 `target`文件夹，将`bootstrap/javacc.jar`拷贝到 target文件件    
```

以上步骤操作完，即可在终端输入`javacc`       

>注意：需要创建 target的目录，原因是 javacc的命令脚本是指定了 target 目录，例如 `scripts/javacc.bat` 内容：  
```bash

@echo off

java -classpath "%~dp0..\target\javacc.jar" javacc %1 %2 %3 %4 %5 %6 %7 %8 %9
```    

### Idea插件安装    

Idea安装JavaCC插件，请以下参考官网内容:        
>IntelliJ IDEA

The IntelliJ IDE supports Maven out of the box and offers a plugin for JavaCC development. 

* IntelliJ download: https://www.jetbrains.com/idea/
* IntelliJ JavaCC Plugin: https://plugins.jetbrains.com/plugin/11431-javacc/

refer   
1.https://javacc.github.io/javacc/#use-javacc-from-the-command-line               
2.https://github.com/apache/calcite/blob/main/core/src/main/codegen/templates/Parser.jj     