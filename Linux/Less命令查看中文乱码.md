
## Less命令查看log 中文乱码

```shell
# 添加环境变量
vim /etc/profile

# 添加如下内容
export LESSCHARSET=utf-8

# 生效环境变量
source /etc/profile
```