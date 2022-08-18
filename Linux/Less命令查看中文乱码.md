
## Less中文命令查看log 中文乱码

```shell
# 添加环境变量
vim /etc/profile

# 添加如下内容
export LESS=-isMrf

# 生效环境变量
source /etc/profile
```