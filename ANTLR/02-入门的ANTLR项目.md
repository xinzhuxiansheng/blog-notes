## 入门的ANTLR项目  

> "EOF" 表示的是 终止符（mac = control + d） 

### 引言  
在 ANTLR 项目实战，可了解到  ANTLR Tool, Runtime, and Generated Code 三个方面的使用，以及如何与 Java 项目集成。 

### 示例项目  

```
/** Grammars always start with a grammar header. This grammar is called
* ArrayInit and must match the filename: ArrayInit.g4
*/
grammar ArrayInit;
/** A rule called init that matches comma-separated values between {...}. */
init : '{' value (',' value)* '}' ; // must match at least one value
/** A value can be either a nested array/struct or a simple integer (INT) */
value : init
| INT
;
// parser rules start with lowercase letters, lexer rules with uppercase
INT : [0-9]+ ; // Define token INT as one or more digits
WS : [ \t\r\n]+ -> skip ; // Define whitespace rule, toss it out
```

使用 antlr4 这个别名命令来生成语法分析器和词法分析器      
```shell 
$ antlr4 ArrayInit.g4  
```

根据语法`ArrayInit.g4`，ANTLR 自动生成了很多文件,生成文件内容如下：         
```
➜  starter git:(main) ✗ ll 
total 88
-rw-r--r--  1 a  staff   710B Nov 19 21:55 ArrayInit.g4
-rw-r--r--  1 a  staff   675B Nov 19 22:04 ArrayInit.interp
-rw-r--r--  1 a  staff    50B Nov 19 22:04 ArrayInit.tokens
-rw-r--r--  1 a  staff   1.7K Nov 19 22:04 ArrayInitBaseListener.java
-rw-r--r--  1 a  staff   1.1K Nov 19 22:04 ArrayInitLexer.interp
-rw-r--r--  1 a  staff   4.7K Nov 19 22:04 ArrayInitLexer.java
-rw-r--r--  1 a  staff    50B Nov 19 22:04 ArrayInitLexer.tokens
-rw-r--r--  1 a  staff   897B Nov 19 22:04 ArrayInitListener.java
-rw-r--r--  1 a  staff   6.6K Nov 19 22:04 ArrayInitParser.java
```

目前，我们仅仅需要大致了解这个过程，下面简单介绍一下生成的文件：            
* ArrayInitParser.java：该文件包含一个语法分析器类的定义，这个语法分析器专门用来识别我们的“数组语言”的语法 ArrayInit。          
```
public class ArrayInitParser extends Parser {...}
``` 
* ArrayInitLexer.java：ANTLR 能够自动识别出我们的语法中的文法规则和词法规则。这个文件包含的是词法分析器的类定义，它是由ANTLR通过分析词法规则 INT和 WS，以及语法中的字面值'{'、'，'，和'}'生成的。回想一下上一章的内容，词法分析器的作用是将输入字符序列分解成词汇符号。它形如：                         
```
public class ArrayInitLexer extends Lexer {...}
```

* ArrayInit.tokens：ANTLR会给每个我们定义的词法符号指定一个数字形式的类型，然后将它们的对应关系存储于该文件中。有时，我们需要将一个大型语法切分为多个更小的语法，在这种情况下，这个文件就非常有用了。通过它，ANTLR 可以在多个小型语法间同步全部的词法符号类型。         

* ArrayInitListener.java，ArrayInitBaseListener.java：默认情况下，ANTLR 生成的语法分析器能将输入文本转换为一棵语法分析树。在遍历语法分析树时，遍历器能够触发一系列“事件”（回调），并通知我们提供的监听器对象。ArrayInitListener 接口给出了这些回调方法的定义，我们可以实现它来完成自定义的功能。ArrayInitBaseListener 是该接口的默认实现类，为其中的每个方法提供了一个空实现。ArrayInitBaseListener 类使得我们只需要覆盖那些我们感兴趣的回调方法 。通过指定`-visitor`命令行参数，ANTLR 也可以为我们生成语法分析树的访问器。    

接下来，我们将使用监听器来将short数组初始化语句转换为字符串对象，不过在这之前，我们首先使用一些样例输入来验证我们的语法分析器是否能正常进行匹配工作。       


### 测试生成的语法分析器  


➜  starter git:(main) ✗ antlr4 ArrayInit.g4
➜  starter git:(main) ✗ javac *.java
➜  starter git:(main) ✗ grun ArrayInit init -tokens
{99, 3, 451}
[@0,0:0='{',<'{'>,1:0]
[@1,1:2='99',<INT>,1:1]
[@2,3:3=',',<','>,1:3]
[@3,5:5='3',<INT>,1:5]
[@4,6:6=',',<','>,1:6]
[@5,8:10='451',<INT>,1:8]
[@6,11:11='}',<'}'>,1:11]
[@7,13:12='<EOF>',<EOF>,2:0]



