**`正文`**
[TOC]

## 停止 firewall
```shell
systemctl stop firewalld.service #停止firewall
```

## 禁止firewaall开机启动
```shell
systemctl disable firewalld.service #禁止firewall开机启动
```

## 查看防火墙状态(关闭后显示not running，开启后显示running)
```shell
firewall-cmd  --state
```
