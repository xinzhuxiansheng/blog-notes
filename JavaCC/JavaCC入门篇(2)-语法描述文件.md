# JavaCC入门篇(2) - 语法描述文件

## 简介
JavaCC的语法描述文件是扩展名为.jj的文件，一般情况下，语法描述文件的内容采用如下形式:   
```
options {
    JavaCC的选项
}

PARSER_BEGIN(解析器类名)
package 包名;
import 库名;

public class 解析器类名 {
    任意的Java代码
}
PARSER_END(解析器类名)

扫描器的描述

解析器的描述

```



https://www.cnblogs.com/Gavin_Liu/archive/2009/03/07/1405029.html





### Token Manager(JavaCC的词法分析器)
