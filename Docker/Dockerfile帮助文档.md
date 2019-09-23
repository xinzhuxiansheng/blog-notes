**`正文`**

[TOC]

## 官网参考文档：
https://docs.docker.com/config/containers/logging/


## 启动时配置日志参数
```shell
docker run -d -p xxxx:xxxx --log-driver json-file --log-opt max-size=10m --log-opt max-file=3 nginx
```
简要说明下 参数
1. --log-driver 设置日志驱动
2. --log-opt 设置日志参数  max-size=10m 表示 json文件最大为10MB (超过10MB就会自动生成新文件) ， max-file=3表示json文件最多为3个(超过3个就会自动删除多余的旧文件) 。

