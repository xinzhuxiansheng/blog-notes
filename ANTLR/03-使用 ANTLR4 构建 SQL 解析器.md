## Antlr4 SQL Parse 项目实践       

>Antlr version: 4.13.1      
>当你阅读该篇 Blog，希望你对 Antlr4 有一定的了解，至少明白它可以做什么。    

### 引言  
本篇 Blog 是参考 "Build SQL parser using ANTLR4" (https://medium.com/@sasidharc/build-sql-parser-using-antlr4-part1-2044916a8406), 老外用python 实现 。 博主会使用 java 实现。               

### 目标
我们要构建一个新数据库，它将使用 SQL 语句的方式来创建 CSV 文件、插入数据、删除数据等操作。          
为了开始研究这个想法，让我们分解一下需要做的任务。                

#### 任务
* 使用我们自己的规则为 SQL 语句创建一个 g4 语法文件              
* 从语法文件生成词法分析器和解析器文件               
* 创建一个监听器来读取 SQL 并将其分解为操作                
* 创建一个 java 程序来对文件执行 Create, READ, UPDATE 和 DELETE 等操作              

在这一部分中，我们将从如何构建语法文件并在语法文件中创建规则来解析基本 SQL 语句。           
我们想要为新数据库构建一个 SQL 解析器。我为此创建了一个语法文件，其中包括令牌和规则。数据库规则目前仅支持 CREATE、INSERT、UPDATE、DELETE 以及将数据存储到 CSV 文件中。                                       

#### 假设
* 所有值都是字符串数据类型                  
* 文件名和列名的标识符中仅允许使用字母数字                
* 仅允许使用 AND/OR 运算符的基本过滤条件        

我们想要支持的SQL如下所示:           
```shell   
CREATE FILE <FILE NAME> ( [ColumnNames] );      
# 示例： CREATE FILE product (id, name, desc , price);

INSERT INTO FILE <FILE NAME> ROWS ([value1,value2…valueN]); 
# 示例： insert into file product (id, name, desc, price) rows (1, "pen", "Ballpoint Pen", 1), (2, "pencil", "Drawing Pen", 2), (3, "book", "Ruled Notebook for kids", 4);   

SELECT [ColumnNames] FROM FILE <FILE NAME> WHERE [Expression]  
# 示例： select * from file product;
# 示例： select * from file product where price > 3;    

DELETE FILE <FILE NAME>;
# 示例： delete file product;       
```

这些陈述是不言自明的。现在我们必须在一个名为语法文件（.g4 是 ANTLR 使用的扩展名）的文件中创建所有词汇标记和解析器规则。         

### 项目设置
让我们创建一个文件夹并安装 ANTLR4 的开发环境, 此处可参考博主的另一篇 Blog '安装 ANTLR'

现在请确保按照 ANTLR 安装说明或以下命令添加用于生成 Lexer 和 Parser 的 ANTLR 工具。         

>注意：对于最新的 ANTLR java 11 或更高版本是必需的。如果您愿意，也可以使用最新版本。            
完成所有步骤后。这是执行的 antlr4 别名的样子。      

packages
```

```

```shell
antlr4 grammar/pockets.g4 -o pockets
```     

antlr4 命令的各个部分含义如下：         
* `grammar/pockets.g4`：这是ANTLR将要处理的语法文件的路径。在这个例子中，该文件位于`grammar`目录下，文件名为`pockets.g4`。`.g4` 是ANTLR语法文件的标准扩展名。           

* `-o pockets`：这是另一个选项，用于指定输出目录，即生成的代码应该放在哪个文件夹中。在这个例子中，生成的代码将被放置在名为`pockets`的文件夹中。             

>注意：对于基于访问者的遍历，我们需要使用 -visitor 选项生成代码。           





refer   
1.https://medium.com/@sasidharc/build-sql-parser-using-antlr4-part1-2044916a8406        

