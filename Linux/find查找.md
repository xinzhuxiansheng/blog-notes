## find查找

>在指定目录下查找文件
-name: 指定文件名
-iname: 指定文件名，忽略大小写
.: 表示当前目录，可省略,换成绝对路径或者相对路径就可以根据指定路径查找文件。例如：/root
*.tar.gz: 模糊匹配 

### 使用

```shell

# 在当前目录下查找具体文件名的文件
find . -name xxx.zip

# 在当前目录下查找指定后缀的文件
find . -name "*.zip"

# 在当前目录下查找具体文件名的文件，忽略文件名的大小写
find . -iname xxx

# 在指定的目录下查找文件
find /root -name xxx.zip
```