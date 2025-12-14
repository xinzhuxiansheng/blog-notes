# Calcite - 探索 Relation Algebra (关系代数)

>Calcite version: 1.35.0        

## 引言  
 (https://mp.weixin.qq.com/s/SgOAByvcS2W6p6UcQCoSjQ) 

在上一篇 `Calcite - 实践 动态 DDL，Calcite SqlParser，SqlValidator，自定义 UDF` 公众号文章内容中的 `YzhouCsvTest_withoutjson.java` 示例代码，我们直接执行 SQL 语句，对数据集进行查询操作。部分代码如下所示，但是在 Apache Calcite 中也提供了另一种方式来构建查询，但需注意：它们本质是一样的, 都是`转换成` Relation Algebra，但 `Algebra builder`更容易接近`Relational Algebra` 语义 。但该篇更像是实践前的理论知识补充。                   
```java
String sql = "select deptno,name from depts";
try (Statement statement = connection.createStatement();
      ResultSet resultSet = statement.executeQuery(sql)) {
  print(resultSet);
}
```  

接下来，我们开始探索 `Relation Algebra`， 它很重要！！！      

## “一头雾水” （Calcite 的 Algebra 文档）    

**图01**     
![algebra01](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra01.png)           

```bash
Algebra
Relational algebra is at the heart of Calcite. Every query is represented as a tree of relational operators. You can translate from SQL to relational algebra, or you can build the tree directly.

Planner rules transform expression trees using mathematical identities that preserve semantics. For example, it is valid to push a filter into an input of an inner join if the filter does not reference columns from the other input.

Calcite optimizes queries by repeatedly applying planner rules to a relational expression. A cost model guides the process, and the planner engine generates an alternative expression that has the same semantics as the original but a lower cost.

The planning process is extensible. You can add your own relational operators, planner rules, cost model, and statistics.
```

### 开头1
博主在刚开始学习 Apache Calcite 时，对这篇 `Algebra` 文档的理解有些“犯难”，正文如 `图01` 所示，首先文档介绍 `Relational algebra`是 Calcite 的核心概念，任何查询都可以表示为 `a tree of relational operators`, 它告诉我们可以通过 SQL 语句转成 `relational algebra`,也可以通过 `RelBuilder` 构建它。      

![algebra03](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra03.png)   

>PS: 为啥是 `RelBuilder`，在该篇文档中的 `Algebra builder`章节告诉我们了，查看 `图02`。    

**引入 RelBuilder 图02**        
![algebra02](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra02.png)    

### 段落2  
段落2首句话翻译过来是：`Planner rules` 使用保持语义的 "数学恒等式" 来转换 "表达式树"; 一句非常简单的描述，它的确让我提出了很多问题，下面我用不同颜色标记出来名词和动词，如图04所示：  

1.`Planner rules` 是什么？       
2.告诉我们用保持语义的 `mathematical identities`，那数学恒等式是什么？ 它用了复数形式 `ties` 那有哪些恒等式，我们又如何理解它？       
3.转换成 `expression tree`, 那之前的形式是什么？ 转换后 `expression tree` 又是什么？ 转换的目的是什么？         

>Tip：  
提出问题，是为了更好思考，搁置问题，仅代表暂时它没有答案，但体系知识的形成也不是一头走到黑。 `学会搁置问题是很重要的学习方法！`                      

**图4**           
![algebra04](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra04.png)     

段落2第二句话翻译过来是：例如，如果过滤条件不引用另一个输入中的列，则可以将过滤条件推入内连接的输入中；     

>这句话你完全可以用 AI 帮你解释，并且它可以给出示例，告诉你怎么回事，但我们又如何理解，这的确是另外一件事，在这个交互过程，博主仍然有些焦虑感，往往用 AI 给出的定义或者名词解释，我很难在不懂的领域来佐证它是否正确，同时我也无法将它作为结论转述给其他人。这是因为它给出答案的 `不确定性`，这里需要与 Code 做一些区分，AI 给出 Code 好坏，大多时候可以通过运行结果来判定它是否正确；     

第二句话我也用不同的颜色标记处动词和名词，如图05 所示，这里多了一个 `懵逼的 input`,它指代什么?                 

**图05**          
![algebra05](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra05.png)  

### 段落3 
段落3原文翻译：Calcite 通过反复对关系表达式应用规划器规则来优化查询。成本模型指导这一过程，`The planner engine` 生成一个与原始表达式具有`相同语义但成本更低`的替代表达式。              

>这句子看着特别别扭。如果表述成：`The planner engine` 根据 `a cost model` 算法反复对 `a relational expression` 使用 `planner rules` 来优化查询。其目标是生成一个与原始表达式具有相同语义但成本更低的 `alternative expression`;     

我又同样对内容做了颜色的区分，如图06 所示，这里又收获了超多的问题：                     
1.`a cost model` 是什么？       
2.反复对 `a relational expression` 使用 `planner rules`，它告诉我们是 `optimizes queries` 行为，这种行为依据是什么？              
3.`the planner engine` 生成的 `alternative expression` 在上面的段落介绍中，也提到过 `relational expression`, 什么是 expression ？        
4.`but a lower cost`，说成本更低，我得先知道成本是什么？       

**图06**        
![algebra06](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra06.png)        

### 段落4 
段落4原文翻译：规划过程是可扩展的。你可以添加自己的关系运算符、规划规则、成本模型和统计信息。 

这句话告诉我，很多它的学习方向：自定义 operators，自定义 planner rules，自定义 cost model，自定义 statistics。 博主会在后续的实践中紧扣这些内容。    

**图07**          
![algebra07](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra07.png)             

### 小结     
Calicte Algebra 文档的这几段介绍，虽然句子不长，但提供的信息量还是比较大。对于博主来说，提出了很多需要弄明白的问题。                        
![algebra08](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra08.png)              

接下来，开始探索吧！让我们去击破它们。     

>Tip:    
理论知识得硬啃，没啥好捷径！            

## The Relational Algebra（关系代数）    
`Relational Algebra` 相关知识大家可以从两本书中获取，下面将这部分的内容从书中 `摘要`出来。分别是： `    
1.Database System Concepts Seventh Edition - The Relational Algebra 2.6 章节                
2.Calcite 数据管理实战 - 关系代数 4.3.1 章节                

### Database System Concepts Seventh Edition - Relational Algebra (2.6 章节)     
![algebra09](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra09.png)   

#### The Relational Algebra   
The relational algebra consists of a set of operations that take one or two relations as
input and produce a new relation as their result.     
（PS:告诉我们，关系代数是由一组 `operations`组成，这些 `operations` 以一个或两个关系为输入，这里第一次出现 `input`, 在上面的 Calicte 文章中也出现 `input`）            

Some of these operations, such as the select, project, and rename operations, are
called unary operations because they operate on one relation. The other operations,
such as union, Cartesian product, and set difference, operate on pairs of relations and
are, therefore, called binary operations.               
（PS：了解到，这些操作可以根据对 `relation`的个数区分成 一元操作 / 二元操作。 

你可以访问 https://dbis-uibk.github.io/relax/help，它对 operate 做了详细归类： 
![algebra57](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra57.png)    
）  

Although the relational algebra operations form the basis for the widely used SQL
query language, database systems do not allow users to write queries in relational algebra. However, there are implementations of relational algebra that have been built for
students to practice relational algebra queries. The website of our book, db-book.com,
under the link titled Laboratory Material, provides pointers to a few such implementations.       
（PS：这里面包含一些隐含的说明是 SQL 不单单是一种可以用作数据查询的语言，更重要的是可通过关系代数（Relation Algebra）进行形式化表示，使得计算机理解和处理查询的语义更加方便。             

关系代数是一种简洁的，形式化的语言，同时也是一种过程化语言。它过程
化地表示了 SQL 的执行逻辑。SQL 查询优化的本质是优化其对应关系代数

尽管数据库不允许直接使用 `relational algebra` 进行查询，但可以访问 `https://db-book.com` 获取该书提供了一些学习案例，然后在 `https://dbis-uibk.github.io/relax/landing` 网站提供的 `RelaX - relational algebra calculator` 功能上进行测试 `Relational Algebra`      

这部分我会在后面实践中，告诉大家 calculator 如何使用。        

**db-book.com**         
![algebra11](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra11.png)   

**关系代数计算器**   
![algebra10](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra10.png)    
）

It is worth recalling at this point that since a relation is a set of tuples, relations
cannot contain duplicate tuples. In practice, however, tables in database systems are
permitted to contain duplicates unless a specific constraint prohibits it. But, in discussing the formal relational algebra, we require that duplicates be eliminated, as is
required by the mathematical definition of a set. In Chapter 3 we discuss how relational algebra can be extended to work on multisets, which are sets that can contain
duplicates.               

#### 为实操 `Relational Algebra` 运算做准备  
**1.** 首先访问 `https://db-book.com/` 页面，点击 `Sample tables`，获取示例的 DDL `DDL.sql` 和 DML `largeRelationsInsertFile.sql` 语句，下载好它们。     

**Sample tables**             
![algebra12](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra12.png)         

**下载 DDL，DML**           
![algebra13](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra13.png)       

**2.** 导入 DataSet  
在 `RelaX - relational algebra calculator` 网站中并没有 `Database System Concepts Seventh Edition` 示例数据集，所以需要将下载 DDL & DML 整理成一个符合 `Relax Dataset` 语法规范的数据文件。可以先通过访问 `https://dbis-uibk.github.io/relax/help#tutorial-maintainer-create-dataset` 了解一下。 (PS: 看 doc 介绍那么多，但格式很简单，AI 帮忙搞定)       

语法的格式：          
```bash
group: [数据集名称]

表名 = {
    字段01:string, 字段02:string, 字段03:number ....
    [示例数据]
}

# 多个表名，大括号之间无需逗号隔开   
```

**示例**          
```bash
group: university

time_slot = {
    time_slot_id:string, day:string, start_hr:number, start_min:number, end_hr:number, end_min:number
    "A", "M", 8, 0, 8, 50
    "A", "W", 8, 0, 8, 50
    ...
}

classroom = {
    building:string, room_number:string, capacity:number
    "Lamberton", 134, 10
    "Chandler", 375, 10
    ...
}

department = {
    dept_name:string, building:string, budget:number
    "Civil Eng.", "Chandler", 255041.46
    "Biology", "Candlestick", 647610.55
    ...
}
```

**3.** 使用 AI工具将 DDL SQL 和 DML SQL 拼接处 `Relax Dataset` 格式数据文件，再将该文件上传到 gist（`https://gist.github.com`）,示例如下，创建了名为 `sql-relax.txt` 内容。     
![algebra14](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra14.png)    

**“Create public gist” 与 “Create secret gist” 的区别**       
| 项目 | Public Gist | Secret Gist |
|------|--------------|--------------|
| **可访问性** | 任何人都可以访问 | 只有拿到链接的人能访问 |
| **是否登录可见** | 否，一般用户都能浏览 | 是，默认只有你和知道链接的人 |
| **是否出现在 GitHub 搜索结果中** | ✅ 会被 GitHub 索引、也能被 Google 搜到 | ❌ 不会出现在任何搜索或列表 |
| **是否出现在你的用户主页上** | ✅ 在个人 Gists 列表中公开显示 | ❌ 仅你登录后能看到 |
| **用途** | 分享代码示例、公开资源 | 保存笔记、临时代码、私密任务日志等 |
| **安全性级别** | 公共，可被任何人 fork | 相对保密，但注意不是完全私有仓库（有链接就能访问） |

如果你已经准备好,像 `sql-relax.txt`的数据文件，那接下来就可以打开 Relax Calculator 网站，加载 gist 中的数据文件，并开始使用。         
![algebra15](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra15.png)   

将你的 gist 文件对应的 URL中的 gist id 拷贝表单中，点击 `load`即可。这里还需特别注意： gist 的文件内容，尽量少一些，避免出现一些稀奇古怪的语法异常。    
![algebra16](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra16.png)   

导入成功后，可以在 Calculator 左侧栏看到导入的 DDL结构。如下图所示：  
![algebra17](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra17.png)    

到这里，实操 `Relational Algebra` 运算的环境就已经准备好了。  

>需要特别，书中的 `Relational Algebra` 示例，`"` 在 `Relax Calculator`会报错，改用 `'`就行；另外查询返回的数据与你上传的 dataset 中数据息息相关，与书中不符的话，没必要纠结!                 
![algebra19](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra19.png)          

#### The Select Operation
The select operation selects tuples that satisfy a given predicate. We use the lowercase Greek letter sigma (σ) to denote selection. The predicate appears as a subscript to σ.The argument relation is in parentheses after the σ. Thus, to select those tuples of the instructor relation where the instructor is in the “Physics” department, we write:    
![algebra18](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra18.png)     

**输出结果如下：图20**               
![algebra20](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra20.png)            

If the instructor relation is as shown in Figure 20, then the relation that results from the preceding query is as shown in Figure 2.10. We can find all instructors with salary greater than $90,000 by writing:         
![algebra21](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra21.png)     

**输出结果如下：图22**     
![algebra22](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra22.png)            

In general, we allow comparisons using =, ≠, <, ≤, >, and ≥ in the selection predicate. Furthermore, we can combine several predicates into a larger predicate by using the connectives and (∧), or (∨), and not (¬). Thus, to find the instructors in Physics with a salary greater than $90,000, we write:        
![algebra23](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra23.png)

**输出结果如下：图24**      
![algebra24](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra24.png)        

The selection predicate may include comparisons between two attributes. To illustrate, consider the relation department. To find all departments whose name is the same
as their building name, we can write: 
（PS: 选择`谓词`也可以包含两个属性的比较，在 Table 将字段称为属性有点怪怪的， 在 `department` 表中，dept_name 与 building 字段相等的数据， 这里需要注意的是 Relax 公式中的 building 后面需要加上`空格`，不然 Calculator 会提示报错）             
![algebra25](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra25.png)    

#### The Project Operation
Suppose we want to list all instructors' ID, name, and salary, but we do not care about the dept name. The project operation allws us to produce this relation. The project operation is a unary operation that returns its argument relation, with certain attributes left out. Since a relation is a set, any duplicate rows are eliminated. Projection is denoted by the uppercase Greek letter pi (Π). We list those attributes that we wish to appear in the result as a subscript to Π. The argument relation follows in parentheses. We write the query to produce such a list as:      
![algebra26](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra26.png)            

**输出结果如下：图27**    
![algebra27](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra27.png)               

The basic version of the project operator ΠL(E) allows only attribute names to be present in the list L. A generalized version of the operator allows expressions involving attributes to appear in the list L. For example, we could use:           
(PS: 文中提到 `基础版的 project operator`，它仅允许属性/字段 出现在 L,在 `广义版本中 project operator` 是允许允许属性/字段包含计算表达式的， 下面的示例是计算每个老师的月薪。     

在 `Relax Calculator` 中,提示 salary/12 是不合法，可以得到 Relax Calculator 支持基础版本。    
)             
![algebra28](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra28.png)        
to get the monthly salary of each instructor.         

#### Composition of Relational Operations           
The fact that the result of a relational operation is itself a relation is important. Consider the more complicated query “Find the names of all instructors in the Physics department.” We write:      
![algebra29](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra29.png)         

**输出结果如下：图30**        
![algebra30](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra30.png)          

Notice that, instead of giving the name of a relation as the argument of the projection operation, we give an expression that evaluates to a relation. In general, since the result of a relational-algebra operation is of the same type (relation) as its inputs, relational-algebra operations can be composed together into a relational-algebra expression. Composing relational-algebra operations into relationalalgebra expressions is just like composing arithmetic operations (such as +, −, ∗, and ÷) into arithmetic expressions.     
（PS：这里特别重要，首次提到 `relation`,`relational-algebra operation`， 再次提到 `input`。            

提到 `projection operation π` 的参数并不是 某个 `表`的名称，它可以是关系代数运算的结果，此时这种结果集称为 `relation`。它给定的原因是 前面产生的`relation`与 project operation 需要的 `input` 是相同的。此时，我们回到 Calcite Algebra 文档段落02 提到的示例：`For example, it is valid to push a filter into an input of an inner join if the filter does not reference columns from the other input.`,我们就用上传的 gist dataset 表和数据来拼接符合示例的查询：`查询 秋季（Fall）开设的所有课程`。          
```bash
SELECT s.course_id, c.title, c.dept_name
FROM section AS s
INNER JOIN course AS c ON s.course_id = c.course_id
WHERE s.semester = 'Fall'      
```   
![algebra33](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra33.png)      

文档中也提到的 an input / the other input，现在我们再去理解，它就像子查询，查询出来的结果集 `relation`,作为下次运算的输入。       

将示例 SQL 放入 Relax Calculator 查看它的 `expression tree`，哈哈，在上面关系代数表达式示例中，在返回结果集的图片，我们看到像 tree 结构，那代表的是 `expression tree`,在上面 Calcite Algebra 文档也有`expression tree` 名词出现。             
**输出结果如下：图31**          
![algebra31](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra31.png)      

我们看到 WHERE 的过滤条件，它只针对 section 一个 input 作用，对 course input 不起任何作用, 我们将过滤逻辑下推到 section input 中， 这个过程其实就是 `Planner rules`优化的过程。 如下图所示：         
![algebra32](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra32.png)          

关于 `relational-algebra operation`的定义：将关系代数运算组合成关系代数表达式，就像将算术运算（如+、−、∗和÷）组合成算术表达式一样。             
）  

#### The Cartesian-Product Operation
The Cartesian-product operation, denoted by a cross (×), allows us to combine information from any two relations. We write the Cartesian product of relations r1 and r2 as r1 × r2.       
![algebra34](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra34.png)   

**输出结果如下：图35**      
![algebra35](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra35.png)      

A Cartesian product of database relations differs in its definition slightly from the mathematical definition of a Cartesian product of sets. Instead of r1 × r2 producing  pairs (t1, t2) of tuples from r1 and r2, the  relational algebra concatenates t1 and t2 into  a single tuple, as shown in Figure 35.    

Since the same attribute name may appear in the schemas of both r1 and r2, we need to devise a naming schema to distinguish between these attributes. We do so here by attaching to an attribute the name of the relation from which the attribute originally came. For example, the relation schema for r = instructor × teaches is:       
(PS: 笛卡尔积之后，新的关系 schema 如下所示)          
```bash
(instructor.ID, instructor.name, instructor.dept_name, instructor.salary,
teaches.ID, teaches.course_id, teaches.sec_id, teaches.semester, teaches.year)
```   

With this schema, we can distinguish instructor.ID from teaches.ID. For those attributes that appear in only one of the two schemas, we shall usually drop the relation-name prefix. This simplification does not lead to any ambiguity. We can then write the relation schema for r as:        
(PS: 可以在首个字段添加 schama，后续的 fields 就可以代表查询的是哪个 schama)            
![algebra36](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra36.png)      

This naming convention requires that the relations that are the arguments of the  Cartesian-product operation have distinct names. This requirement causes problems in some cases, such as when the Cartesian product of a relation with itself is desired. A similar problem arises if we use the result of a relational-algebra expression in a Cartesian product, since we shall need a name for the relation so that we can refer to the  relation’s attributes. In Section 2.6.8, we see how to avoid these problems by using the rename operation.      
(PS: 但上面的方式也会存在问题，当 某个 relation 对自己进行笛卡尔积，那就会出现像下图这样的报错信息，无法识别唯一列， 但这种问题，可以通过 `rename operation` 解决)                       
![algebra37](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra37.png)        

Now that we know the relation schema for r = instructor × teaches, what tuples appear in r? As you may suspect, we construct a tuple of r out of each possible pair of tuples: one from the instructor relation (Figure 2.1) and one from the teaches relation . Thus, r is a large relation, as you can see from Figure 35, which includes only a portion of the tuples that make up r.       

Assume that we have n1 tuples in instructor and n2 tuples in teaches. Then, there are n1 ∗ n2 ways of choosing a pair of tuples—one tuple from each relation; so there are n1 ∗ n2 tuples in r. In particular for our example, for some tuples t in r, it may be that the two ID values, instructor.ID and teaches.ID, are different.     

In general, if we have relations r1(R1) and r2(R2), then r1 × r2 is a relation r(R) whose schema R is the concatenation of the schemas R1 and R2. Relation r contains all tuples t for which there is a tuple t1 in r1 and a tuple t2 in r2 for which t and t1 have the same value on the attributes in R1 and t and t2 have the same value on the attributes in
R2.              

#### The Join Operation
Suppose we want to find the information about all instructors together with the course id of all courses they have taught. We need the information in both the instructor relation and the teaches relation to compute the required result. The Cartesian product of instructor and teaches does bring together information from both these relations, but unfortunately the Cartesian product associates every instructor with every course that was taught, regardless of whether that instructor taught that course.         

Since the Cartesian-product operation associates every tuple of instructor with every tuple of teaches, we know that if an instructor has taught a course (as recorded in the teaches relation), then there is some tuple in instructor × teaches that contains her name and satisfies instructor.ID = teaches.ID. So, if we write:               
![algebra38](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra38.png)     

**输出结果如下：图39**                
![algebra39](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra39.png)              

we get only those tuples of instructor × teaches that pertain to instructors and the courses that they taught.      

The result of this expression is shown in Figure 2.13. Observe that instructors Gold, Califieri, and Singh do not teach any course (as recorded in the teaches relation), and therefore do not appear in the result.        

Note that this expression results in the duplication of the instructor’s ID. This can be easily handled by adding a projection to eliminate the column teaches.ID.      

The join operation allows us to combine a selection and a Cartesian product into a single operation.      

Consider relations r(R) and s(S), and let θ be a predicate on attributes in the schema R ∪ S. The join operation r ⋈θ s is defined as follows:            
![algebra40](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra40.png)        

Thus, σinstructor.ID=teaches.ID(instructor × teaches) can equivalently be written as instructor ⋈instructor.ID=teaches.ID teaches.       
（PS： 这两者是等价的）        
![algebra41](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra41.png)        
![algebra42](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra42.png)            

#### Set Operations
Consider a query to find the set of all courses taught in the Fall 2017 semester, the Spring 2018 semester, or both. The information is contained in the section relation. To find the set of all courses taught in the Fall 2017 semester, we write:         
![algebra43](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra43.png)       

To find the set of all courses taught in the Spring 2018 semester, we write:        
![algebra44](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra44.png)          

To answer the query, we need the union of these two sets; that is, we need all course ids that appear in either or both of the two relations. We find these data by the binary operation union, denoted, as in set theory, by ∪. So the expression needed is:       
![algebra45](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra45.png)    

（PS： 我的 gist dataset 中的 `section relation` 没有 2017,2018 的数据，所以结果为空， 此处以实际测试情况为准!）     
**输出结果如下：图46**          
![algebra46](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra46.png)      

The result relation for this query appears in Figure 47. Notice that there are eight tuples in the result, even though there are three distinct courses offered in the Fall 2017 semester and six distinct courses offered in the Spring 2018 semester. Since relations are sets, duplicate values such as CS-101, which is offered in both semesters, are replaced by a single occurrence.                      

Observe that, in our example, we took the union of two sets, both of which consisted of course id values. In general, for a union operation to make sense:                  
1. We must ensure that the input relations to the union operation have the same number of attributes; the number of attributes of a relation is referred to as its arity.                 
2. When the attributes have associated types, the types of the ith attributes of both input relations must be the same, for each i.             

Such relations are referred to as compatible relations.                 
For example, it would not make sense to take the union of the instructor and section relations, since they have different numbers of attributes. And even though the instructor and the student relations both have arity 4, their 4th attributes, namely, salary and tot cred, are of two different types. The union of these two attributes would not make sense in most situations.                

The intersection operation, denoted by ∩, allows us to find tuples that are in both the input relations. The expression r ∩ s produces a relation containing those tuples in            

![algebra47](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra47.png)     
![algebra48](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra48.png)      

r as well as in s. As with the union operation, we must ensure that intersection is done between compatible relations.            

Suppose that we wish to find the set of all courses taught in both the Fall 2017 and the Spring 2018 semesters. Using set intersection, we can write    
![algebra49](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra49.png)      

The result relation for this query appears in Figure 2.15.        
The set-difference operation, denoted by −, allows us to find tuples that are in one relation but are not in another. The expression r − s produces a relation containing those tuples in r but not in s.           

We can find all the courses taught in the Fall 2017 semester but not in Spring 2018 semester by writing:     
![algebra50](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra50.png)      

The result relation for this query appears in Figure 2.16.        
As with the union operation, we must ensure that set differences are taken between compatible relations.        

#### The Assignment Operation
It is convenient at times to write a relational-algebra expression by assigning parts of it to temporary relation variables. The assignment operation, denoted by ←, works like assignment in a programming language. To illustrate this operation, consider the query to find courses that run in Fall 2017 as well as Spring 2018, which we saw earlier. We could write it as:              
![algebra51](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra51.png)          

（PS:书中 Assignment Operation 使用 ←， 但我使用的是 `=`）  
![algebra52](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra52.png)        

**输出结果如下：图53**      
![algebra53](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra53.png)      

The final line above displays the query result. The preceding two lines assign the query result to a temporary relation. The evaluation of an assignment does not result in any relation being displayed to the user. Rather, the result of the expression to the right of the ← is assigned to the relation variable on the left of the ←. This relation variable may be used in subsequent expressions.         

With the assignment operation, a query can be written as a sequential program consisting of a series of assignments followed by an expression whose value is displayed as the result of the query. For relational-algebra queries, assignment must always be made to a temporary relation variable. Assignments to permanent relations constitute a database modification. Note that the assignment operation does not provide any additional power to the algebra. It is, however, a convenient way to express complex queries.                       

#### The Rename Operation
Unlike relations in the database, the results of relational-algebra expressions do not have a name that we can use to refer to them. It is useful in some cases to give them names; the rename operator, denoted by the lowercase Greek letter rho (ρ), lets us do this. Given a relational-algebra expression E, the expression        
ρx (E)        

returns the result of expression E under the name x.        

A relation r by itself is considered a (trivial) relational-algebra expression. Thus, we can also apply the rename operation to a relation r to get the same relation under a new name. Some queries require the same relation to be used more than once in the query; in such cases, the rename operation can be used to give unique names to the different occurrences of the same relation.            

A second form of the rename operation is as follows: Assume that a relationalalgebra expression E has arity n. Then, the expression       

ρx(A1,A2,…,An) (E)

returns the result of expression E under the name x, and with the         attributes renamed to A1, A2, …, An. This form of the rename operation can be used to give names to attributes in the results of relational algebra operations that involve expressions on attributes.            

To illustrate renaming a relation, we consider the query “Find the ID and name of those instructors who earn more than the instructor whose ID is 12121.” (That’s the instructor Wu in the example table in Figure 2.1.)        

There are several strategies for writing this query, but to illustrate the rename operation, our strategy is to compare the salary of each instructor with the salary of the      

instructor with ID 12121. The difficulty here is that we need to reference the instructor relation once to get the salary of each instructor and then a second time to get the salary of instructor 12121; and we want to do all this in one expression. The rename operator allows us to do this using different names for each referencing of the instructor relation. In this example, we shall use the name i to refer to our scan of the instructor relation in which we are seeking those that will be part of the answer, and w to refer to the scan of the instructor relation to obtain the salary of instructor 12121:          
![algebra54](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra54.png)    

The rename operation is not strictly required, since it is possible to use a positional notation for attributes. We can name attributes of a relation implicitly by using a positional notation, where $1, $2, … refer to the first attribute, the second attribute, and so on. The positional notation can also be used to refer to attributes of the results of relational-algebra operations. However, the positional notation is inconvenient for humans, since the position of the attribute is a number, rather than an easy-to-remember attribute name. Hence, we do not use the positional notation in this textbook.             

#### Equivalent Queries
Note that there is often more than one way to write a query in relational algebra. Consider the following query, which finds information about courses taught by instructors in the Physics department:           

![algebra55](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra55.png)   

Now consider an alternative query:        

![algebra56](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra56.png)        

Note the subtle difference between the two queries: in the first query, the selection that restricts dept name to Physics is applied after the join of instructor and teaches has been computed, whereas in the second query, the selection that restricts dept name to Physics is applied to instructor, and the join operation is applied subsequently.           

Although the two queries are not identical, they are in fact equivalent; that is, they give the same result on any database.            

Query optimizers in database systems typically look at what result an expression computes and find an efficient way of computing that result, rather than following the exact sequence of steps specified in the query. The algebraic structure of relational algebra makes it easy to find efficient but equivalent alternative expressions, as we will
see in Chapter 16.            

（PS： 关系代数的代数结构使得找到高效且等价的替代表达式变得容易）

到这里，Database System Concepts 书中的 第2.6章节就介绍完成了。如果想了解更多，可以访问 `https://dbis-uibk.github.io/relax/help#relalg-reference` 了解关系代数的一般语法。   

### Calcite 数据管理实战 - 关系代数 (4.3.1 章节)           
关系代数是一种关于数据库查询和数据管理方法的理论模型，以其为核心的关系模型在1970年埃德加·弗兰克·考德（Edgar Frank Codd）发表的论文“A Relational Model of Datafor Shared Data Banks”中出现，并一举奠定了其在之后几十年内在数据库领域的“江湖地位”。现有的很多被广泛使用的关系数据库（例如MySQL、Oracle等）都是在关系模型的基础上发展而来的。          

关系模型主要分为3个部分：关系数据结构、关系运算集合和关系完整性约束。   

关系数据结构就是我们在日常生活当中常见的表格的形式，它是一个横纵结合的表。在关系模型中，`每一行的数据被称为一个元组，也被称为一条记录`，大量的元组共同汇聚成一个集合，即整张表的数据。每一列则表示不同的属性，也被称为字段，它通过表的元数据进行管理，记录了不同属性的名称、数据类型以及其他的描述信息。          

（PS：       
1.每一行的数据被称为一个元组，也被称为一条记录， 所以在 `Database System Concepts Seventh Edition - Relational Algebra (2.6 章节)` 书中就是使用 `tuples` 来表示 数据）            
2.提到每一列则表示不同的属性，也被称为字段， 所以在 `Database System Concepts Seventh Edition - Relational Algebra (2.6 章节)` 书中就是使用 `attributes` 来表示 fields）         

关系运算集合指的是对关系模型中的数据进行运算的操作方式。关系运算符主要分为四大类：集合运算符、专门的关系运算符、比较运算符以及逻辑运算符。集合运算符指的是集合的并集、交集、差集以及笛卡儿积等针对集合关系的运算符。专门的关系运算符指的是对于数据集的选择、投影、连接等操作的运算符。比较运算符的原始内涵是指大于、小于、等于这样的对于数值比较结果的真假进行判断的运算符。现在由于数据库函数的介入，比较运算符的外延有了极大的扩展，用户可以通过函数来输出真假的结果，很大程度上使得关系代数的适用范围更加广阔。逻辑运算符则是指与、或、非这样的对条件进行逻辑组织的运算符。下面图表展示了这4种关系运算符的基本内容及其在SQL中的示例。   

| 关系运算符类型 | 关系运算 | 符号表示 | 关系运算的含义 | SQL示例（R集合与S集合，R集合中有x、y两个字段，S集合中有y、z两个字段） |
|--------------|---------|----------|----------------|---------------------------------------------------------------------|
| 集合运算符    | 并       | ∪        | 多个关系合并元组 | `SELECT * FROM R UNION SELECT * FROM S` |
| 集合运算符    | 交       | ∩        | 多个关系中根据条件筛选元组 | `SELECT y FROM R INNER JOIN S ON 条件` |
| 集合运算符    | 差       | -        | 多个关系中根据条件去除元组 | `SELECT y FROM R WHERE y NOT IN (SELECT y FROM S)` |
| 集合运算符    | 笛卡尔积  | ×        | 无连接条件       | `SELECT R.*, S.* FROM R, S` |
| 专门的关系运算符 | 选择     | σ        | 单个关系中筛选元组 | `SELECT * FROM R WHERE x > 10` |
| 专门的关系运算符 | 投影     | π        | 单个关系中筛选列   | `SELECT x, y FROM R` |
| 专门的关系运算符 | 连接     | ⋈        | 多个关系中根据列间的逻辑运算筛选元组 | `SELECT R.x, S.z FROM R, S [WHERE condition]` |
| 专门的关系运算符 | 除       | ÷        | 多个关系中根据条件筛选元组 | `SELECT DISTINCT r1.x FROM R r1 WHERE NOT EXISTS (SELECT S.y FROM S WHERE NOT EXISTS (SELECT * FROM R r2 WHERE r2.x = r1.x AND r2.y = S.y))` |


| 关系运算符类型 | 关系运算 | 符号表示 | 关系运算的含义 | SQL示例（R集合与S集合，R集合中有x、y两个字段，S集合中有y、z两个字段） |
| --- | --- | --- | --- | --- |
| 比较运算符 | 大于 | > | 根据大于某个值的条件筛选元组 | `SELECT * FROM R WHERE x > 10` |
| 比较运算符 | 大于等于 | >= | 根据大于等于某个值的条件筛选元组 | `SELECT * FROM R WHERE x >= 10` |
| 比较运算符 | 小于 | < | 根据小于某个值的条件筛选元组 | `SELECT * FROM R WHERE x < 10` |
| 比较运算符 | 小于等于 | <= | 根据小于等于某个值的条件筛选元组 | `SELECT * FROM R WHERE x <= 10` |
| 比较运算符 | 不等于 | ≠ | 根据不等于某个值的条件筛选元组 | `SELECT * FROM R WHERE x≠10` |
| 比较运算符 | 等于 | = | 根据等于某个值的条件筛选元组 | `SELECT * FROM R WHERE x = 10` |
| 逻辑运算符 | 与 | ∧ | 根据两个条件的交集筛选元组 | `SELECT * FROM R WHERE x = 10 AND y = 20` |
| 逻辑运算符 | 或 | ∨ | 根据两个条件的并集筛选元组 | `SELECT * FROM R WHERE x = 10 OR x=20` |
| 逻辑运算符 | 非 | ¬ | 筛选出不符合某个条件的元组 | `SELECT * FROM R WHERE x != 10` |


## 小结 
通过阅读关系代数，我们对 Calcite Algebra 文档内容有了新的认识。它给我们继续探索 Calcite optimizes queries，提供了宝贵的理论知识。上面的内容，我们提到 `规划器引擎生成一个与原始表达式具有相同语义但成本更低的替代表达式`,建议大家继续阅读 `Database System Concepts Seventh Edition` 第16章节，它告诉我们什么是等价或相同语义？                 

![algebra58](http://img.xinzhuxiansheng.com/blogimgs/calcite/algebra58.png)     

refer:        
1.Algebra https://calcite.apache.org/docs/algebra.html         
2.SqlNode,RelNode,RexNode https://lists.apache.org/thread/z3pvzy1fnl6t5m04gd3wv4tntwpf3g52    
3.关系代数计算器 https://dbis-uibk.github.io/relax/landing      
4.https://dbis-uibk.github.io/relax/help#relalg-reference     