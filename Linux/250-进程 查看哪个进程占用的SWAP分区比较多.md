**`正文`**
[TOC]

## 查看哪个进程占用的SWAP分区比较多
```shell
#查看swap分区信息
for i in $(ls /proc | grep "^[0-9]" | awk '$0>100'); do awk '/Swap:/{a=a+$2}END{print '"$i"',a/1024"M"}' /proc/$i/smaps;done| sort -k2nr | head
#根据pid查看某个进程相关信息
ps aux | grep xxxxx
```