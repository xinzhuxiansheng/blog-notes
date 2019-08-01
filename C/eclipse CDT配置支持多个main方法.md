**正文**

[TOC]

1. 在项目中，新建src "源文件夹" 和Debug "文件夹"，其中src文件夹用来存放.cpp或.c源文件和makefile文件！Debug文件夹可以用来存放.exe啥的文件，目的是为了更好的管理文件！

2. 在src目录下面，新建一个makefile文件
```c
CC := gcc
RM := 
LIBS =
 
all:	xxxa    xxxb
 
xxxa:	xxxa.c
	$(CC)	xxxa.c	-o	../Debug/xxxa 
xxxb:	xxxb.c	
	$(CC)	xxxb.c	-o	../Debug/xxxb 
```


3. 选择Window-->show view -->Build Target,得到Build Target视图！然后右键点击src文件夹，在其下面分别新建几个Target，注意Target的名字必须与makefile文件红的target名字相同！


4. 双击all，会编译所有的文件，双击xxxa或xxxb只会编译单独的文件！ 注意：每次修改代码后，必须重新编译文件，否则修改的代码不会生效！
