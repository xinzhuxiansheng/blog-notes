
# Centos删除乱码文件或文件夹

```shell
#1. 打印文件及文件夹索引号 -i, --inode                print the index number of each file
ll -i

#2. 删除文件
find -inum xxxxx -delete

#3. 删除文件夹
find ./ -inum xxxxx -exec rm {} -rf \;
```
