# Calcite - JavaCC - 官网 Example 入门实践         

>javacc version: 7.0.13          

## 前置条件     
估计你应该也看到来自官网的介绍`Java Compiler Compiler (JavaCC) is the most popular parser generator for use with Java applications.` 但我不知道你是否跟我一样，一脸愁，Calcite 还没弄明白，怎么还多了一个 JavaCC，这又是什么鬼，双重的打击以至于我仍然游走在入门阶段。          

>建议一定要了解下面文档目录的大概内容。(我的实践案例可帮助大家进一步理解)             

```bash
Home > Documentation > Index  
1.JavaCC Command Line (https://javacc.github.io/javacc/documentation/cli.html)  
2.JavaCC Grammar (https://javacc.github.io/javacc/documentation/grammar.html)   
3.JavaCC BNF (https://javacc.github.io/javacc/documentation/bnf.html)  
4.JavaCC API (https://javacc.github.io/javacc/documentation/api.html)   
5.JJTree (https://javacc.github.io/javacc/documentation/jjtree.html)      
6.JJDoc (https://javacc.github.io/javacc/documentation/jjdoc.html)   
```     

## 了解 .jj    
在学习实践过程中，我大多数时间都在写 jj 文件，再利用 javacc 工具编译 jj 文件，生成 java 代码，最后在项目代码中调用生成的 java 代码，以此达到解析功能的实现。     

接下来，先了解下 jj 语法规则，可访问 `JavaCC Grammar` (https://javacc.github.io/javacc/documentation/grammar.html) , 关于 JavaCC 语法理解部分，官网内容就像是一颗多叉树一样，入下图 (注意它不展示全部内容):             
![javaccpractice01](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice01.png)

首先在`JavaCC Grammar`文档内容中看到 `File structure`（文件结构），它告诉我们包含 `javacc_options`, `PARSER_BEGIN/ PARSER_END`、production 一直到 <EOF>终止符。紧接着下面会再给你介绍 `javacc_options`、`production` 等等， `自上而下就构成一个多叉树结构的文档目录`。 有了结构关系，实践 jj就变得简单多了。     

那先从 `File structure`来看，一个 jj文件整体内容包含以下：        
```bash
javacc_input ::= javacc_options
                 "PARSER_BEGIN" "(" <IDENTIFIER> ")"
                 java_compilation_unit
                 "PARSER_END" "(" <IDENTIFIER> ")"
                 ( production )+
                 <EOF>
```

>注意，目前花太多时间探讨 JavaCC Grammar，可能对于入门来说不是一个好的方法，就像学一门语言来说，上手敲起来，跑起来再去理解它可能会更好一些，那下面我们开始使用它来实现一些简单功能。      

在`JavaCC grammar files`中的保留字段：     
```bash
EOF
IGNORE_CASE
JAVACODE
LOOKAHEAD
MORE
PARSER_BEGIN
PARSER_END
SKIP
SPECIAL_TOKEN
TOKEN
TOKEN_MGR_DECLS
```

**JavaCC 中定义语法的模板**   
```bash
options {
    JavaCC的配置项
}
PARSER_BEGIN(解析器类名)
package包名;
import库名;
public class解析器类名 {    
  任意Java代码
}
PARSER_END(解析器类名)

解析逻辑

关键字定义
```
## 工程化     
为了更好的方便测试生成后的 java 代码，创建 javacc项目，如下图所示：      
![javaccpractice02](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice02.png)   

生成 java 代码，可直接在终端执行 javacc xxx.jj 即可，（注意，前提是配置了相应的环境变量，这部分可参考 `介绍及安装`）

## 示例01 - 校验输入内容        
>参考：https://javacc.github.io/javacc/#an-example （注意 jj 文件有部分修改）                   
该示例是判断输入的内容是否合法，若不合法会抛出异常，此时文档中也标注合法与不合法的示例。              
```bash
Examples of legal strings in this grammar are:
{}, {{{{{}}}}} // … etc

Examples of illegal strings are:
{}{}, }{}}, { }, {x} // … etc
```

**修改后的 jj 文件**        
```bash
options{
        STATIC = true;
        OUTPUT_DIRECTORY = "../../src/main/java/official/examples";
}

PARSER_BEGIN(Example)
/** Simple brace matcher. */
package official.examples;
public class Example {

  /** Main entry point. */
  public static void main(String args[]) throws ParseException {
    Example parser = new Example(System.in);
    parser.Input();
  }
}
PARSER_END(Example)

/** Root production. */
void Input() :
{}
{
  MatchedBraces() ("\n"|"\r")* <EOF>
}

/** Brace matching production. */
void MatchedBraces() :
{}
{
  "{" [ MatchedBraces() ] "}"
}
```
**修改部分**    
添加 options 参数:         
1.STATIC = true; (默认值是 true) 如果是 true，则表示生成 java 方法和类变量被指定为 静态。       
2.OUTPUT_DIRECTORY = "../../src/main/java/official/examples"; 定义生成 java代码的目录，在 Idea 项目中,我希望生成的代码在 `official.examples`包下，这样就便于我们直接执行 mian()方法。      

3.package official.examples;  添加 class 的包名，因为生成的 java 代码会在`official.examples`包下，所以 class 的包名是必不可少的。    

>参数含义可参考 https://javacc.github.io/javacc/documentation/grammar.html 文档中的 javacc_options 部分。    

### 生成 java代码    
打开终端，进入`examples\official-examples 目录`， 执行以下命令：              
```bash
javacc example.jj   
```   

Output log:   
```bash
E:\Code\Java\javamain-services\javamain-javacc\examples\official-examples git:[main]       
javacc .\example.jj
Java Compiler Compiler Version 4.1d1 (Parser Generator)
(type "javacc" with no arguments for help)
Reading from file .\example.jj . . .
File "TokenMgrError.java" does not exist.  Will create one.
File "ParseException.java" does not exist.  Will create one.
File "Token.java" does not exist.  Will create one.
File "SimpleCharStream.java" does not exist.  Will create one.
Parser generated successfully.  
```

此时 `official.examples`包下会生成以下 java 文件：        
![javaccpractice03](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice03.png)     

### 执行 & 验证   
idea 中运行`Example`类的 main() 方法：   

1.在控制台输入合法括号，无任何反馈信息   
```bash
# Ctrl+D （windows 平台表示 EOF）
{}
```

2.在控制台输入非法括号，收到 ParseException 异常，表示括号是非法的。（`windows 输入 Ctrl + D 表示终止符`）                
```bash
}{^D
Exception in thread "main" official.examples.ParseException: Encountered "<EOF>" at line 0, column 0.
Was expecting:
    "{" ...
    
	at official.examples.Example.generateParseException(Example.java:235)
	at official.examples.Example.jj_consume_token(Example.java:173)
	at official.examples.Example.MatchedBraces(Example.java:44)
	at official.examples.Example.Input(Example.java:14)
	at official.examples.Example.main(Example.java:9)
```

### 示例总结  
该示例的作用是为了校验括号的合法性，在整个示例过程中，只涉及`example.jj`文件修改，那现在我们来看看它的组成部分。       

#### PARSER_BEGIN/PARSER_END     
```bash
PARSER_BEGIN(Example)
/** Simple brace matcher. */
package official.examples;
public class Example {

  /** Main entry point. */
  public static void main(String args[]) throws ParseException {
    Example parser = new Example(System.in);
    parser.Input();
  }
}
PARSER_END(Example)
```

根据`File structure`里看，`IDENTIFIER`对应的是 `Example`的类名称，`java_compilation_unit` 部分更像是 java 代码，包名，类名以及 main() 方法。         
```bash
"PARSER_BEGIN" "(" <IDENTIFIER> ")"
java_compilation_unit
"PARSER_END" "(" <IDENTIFIER> ")"
```

`Example#main()`方法：创建了一个`Example`类，并且它还调用了 Input() 方法，下面来看 Input() 方法的定义；    

#### Input()方法、MatchedBraces()方法     
```bash
void Input() :
{}
{
  MatchedBraces() ("\n"|"\r")* <EOF>
}

/** Brace matching production. */
void MatchedBraces() :
{}
{
  "{" [ MatchedBraces() ] "}"
}
```

`void Input`、`void MatchedBraces`是方法名的定义，冒号下面第一个大括号暂时没有使用先不做介绍，第二个大括号是`用于定义解析规则`，这里的含义是 `Input()`方法调用 `MatchedBraces()`方法，同时也匹配了换行符，`*`表示匹配零个或者多个。   

特别注意： MatchedBraces()方法，它使用递归下降解析， MatchedBraces()的第二个大括号也调用了 MatchedBraces(), 并且它使用`[]`包裹，表示`可选项`，即规则中的某部分可以有也可以没有，那`"{" [ MatchedBraces() ] "}"`匹配的一些示例 （windows 以 Ctrl + D 表示 EOF 终止符）：   
```bash
# 不会递归调用 MatchedBraces()   
{}

# 递归调用一次    
{{}}

# 递归调用两次
{{{}}}

... 以此类推       
```  

更多信息，可参考 https://javacc.github.io/javacc/tutorials/lookahead.html    

#### 异常捕获  
该示例，当输入校验不匹配时，生成的 java 代码会报`ParseException`异常，在 main()中显示抛出 ParseException，并未在 `void Input()`、`void MatchedBraces()`中看到，这是因为`ParseException`是 JavaCC 自动生成的解析器异常，针对不符合定义的规则（也就是第二个大括号的部分），则会抛出 ParseException，所以 main()需要显示声明它。               

可以观察生成后的 Example.java 它的 Input()、MatchedBraces() 两个方法都抛出 ParseException 异常。      

>参考https://javacc.github.io/javacc/tutorials/error-handling.html      


## 示例02 - SKIP 跳过某些字符校验   
>参考：https://javacc.github.io/javacc/tutorials/examples.html#javacc-example-2， 有了`示例01`的基础，后面的了解估计你也会得心应手。    
项目结构如下图：      
![javaccpractice04](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice04.png)    

```bash
options{
        STATIC = true;
        OUTPUT_DIRECTORY = "../../src/main/java/official/examples2";
}

PARSER_BEGIN(Example2)

/**
 * Simple brace matcher.
 */
package official.examples2;
public class Example2 {

  /** Main entry point. */
  public static void main(String args[]) throws ParseException {
    Example2 parser = new Example2(System.in);
    parser.Input();
  }

}
PARSER_END(Example2)

SKIP :
{
  " "
| "\t"
| "\n"
| "\r"
}

/** Root production. */
void Input() :
{}
{
  MatchedBraces() <EOF>
}

/** Brace matching production. */
void MatchedBraces() :
{}
{
  "{" [ MatchedBraces() ] "}"
}
```

同样添加了 `options`参数，以及 package 参数，为了理解`Example2.jj`的 SKIP， 我们可以先基于`示例01`中的 main()方法，输入以下内容：         
* {空格}        
```bash
{ }
Exception in thread "main" official.examples.TokenMgrError: Lexical error at line 1, column 2.  Encountered: " " (32), after : ""
	at official.examples.ExampleTokenManager.getNextToken(ExampleTokenManager.java:177)
	at official.examples.Example.jj_ntk(Example.java:198)
	at official.examples.Example.MatchedBraces(Example.java:45)
	at official.examples.Example.Input(Example.java:14)
	at official.examples.Example.main(Example.java:9)
```

* {回车       
```bash
{
Exception in thread "main" official.examples.ParseException: Encountered " "\n" "\n "" at line 1, column 2.
Was expecting one of:
    "{" ...
    "}" ...
    
	at official.examples.Example.generateParseException(Example.java:235)
	at official.examples.Example.jj_consume_token(Example.java:173)
	at official.examples.Example.MatchedBraces(Example.java:53)
	at official.examples.Example.Input(Example.java:14)
	at official.examples.Example.main(Example.java:9)
```
 
* {制表符     
```bash
{	}
Exception in thread "main" official.examples.TokenMgrError: Lexical error at line 1, column 8.  Encountered: "\t" (9), after : ""
	at official.examples.ExampleTokenManager.getNextToken(ExampleTokenManager.java:177)
	at official.examples.Example.jj_ntk(Example.java:198)
	at official.examples.Example.MatchedBraces(Example.java:45)
	at official.examples.Example.Input(Example.java:14)
	at official.examples.Example.main(Example.java:9)
Disconnected from the target VM, address: '127.0.0.1:50772', transport: 'socket'
```

以上内容都以 EOF 结束，以 windows为例，输入 Ctrl + D。 你会发现都会报异常，出现`ParseException`或者`TokenMgrError`。      


那么 Example2.jj的 SKIP 就是为了扩展以上特殊的符号， 关于 SKIP的定义可参考`regexpr-kind` https://javacc.github.io/javacc/documentation/grammar.html#regexpr-kind     

![javaccpractice05](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice05.png)      

### 生成 Java代码 & 演示   
通过 javacc Example2.jj，得到 java 代码。    
* {空格}     
```bash
{	}
^D
```

* {回车  
```bash
{
}
^D
```  

* {制表符      
```bash
{	}
^D
```

### 案例总结  
解析规则会跳过 `SKIP`定义的字符, 关于 SKIP 可访问： https://javacc.github.io/javacc/faq.html#what-are-token-skip-and-special_token           

## 示例03 - TOKEN 变量的定义    
>参考：https://javacc.github.io/javacc/tutorials/examples.html#javacc-example-3       

![javaccpractice07](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice07.png)    

**修改后的 jj 文件**     
```bash
options{
        STATIC = true;
        OUTPUT_DIRECTORY = "../../src/main/java/official/examples3";
}
PARSER_BEGIN(Example3)

/**
 * Simple brace matcher.
 */
package official.examples3;
public class Example3 {

  /** Main entry point. */
  public static void main(String args[]) throws ParseException {
    Example3 parser = new Example3(System.in);
    parser.Input();
  }

}

PARSER_END(Example3)

SKIP :
{
  " "
| "\t"
| "\n"
| "\r"
}

TOKEN :
{
  <LBRACE: "{">
| <RBRACE: "}">
}

/** Root production. */
void Input() :
{ int count; }
{
  count=MatchedBraces() <EOF>
  { System.out.println("The levels of nesting is " + count); }
}

/** Brace counting production. */
int MatchedBraces() :
{ int nested_count=0; }
{
  <LBRACE> [ nested_count=MatchedBraces() ] <RBRACE>
  { return ++nested_count; }
}
```

添加通过 options参数和 package参数，我们重点讨论 TOKEN以下部分：      
* TOKEN: {}，请参考：https://javacc.github.io/javacc/documentation/bnf.html#separators      

![javaccpractice06](http://img.xinzhuxiansheng.com/blogimgs/calcite/javaccpractice06.png)  
定义 `左花括号`、`右花括号`两个 TOKEN。       

* { int count; }, 上面的`示例01`,`示例02`，并没有在第一个花括号中定义什么, 而第一个花括号{}是为`语义动作块`预留的，它用于声明和初始化解析过程中需要用到的变量或状态。 （我想这并不陌生，定义了一个 int count）     

* count=MatchedBraces() <EOF>， 赋值操作      

* { System.out.println("The levels of nesting is " + count); }, 它是一段 java 代码块。 注意： java code 是一定需要添加`花括号`        

* <LBRACE> [ nested_count=MatchedBraces() ] <RBRACE>， 使用 TOKEN     

* { return ++nested_count; }，返回值      

看到这里，这无疑很像`java code`，该案例理解起来并不麻烦。 下面我们看下执行结果是什么样？      

### 生成 Java代码 & 演示      
通过 javacc Example3.jj，得到 java 代码。      

```bash
{{}}
^D
The levels of nesting is 2
```

看到统计结果为 2。     

refer       
1.https://javacc.github.io/javacc/         
2.https://javacc.github.io/javacc/faq.html    
3.https://javacc.github.io/javacc/tutorials/lookahead.html    
4.https://javacc.github.io/javacc/documentation/       
