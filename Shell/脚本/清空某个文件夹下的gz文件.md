**`正文`**

[TOC]

## 清空某个文件夹下的gz文件
```shell
#!/bin/bash

# 清理白山云，网宿的cdn gz格式文件，清理间隔 是每天执行一次，清理前一天数据
yesterday=`date +"%Y%m%d" -d "-1 days"`

gzpath="/data/cdngz"

echo 当前清理日期时间为:$yesterday,$gzpath

gzlist=`find $gzpath -name "${yesterday}*.gz"`
for itempath in $gzlist;
do
#echo $itempath:
rm -rf $itempath
done
```
