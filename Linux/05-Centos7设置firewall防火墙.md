## CentOS7设置firewall防火墙

### 停止 firewall
```shell
systemctl stop firewalld.service #停止firewall
```

### 禁止firewaall开机启动
```shell
systemctl disable firewalld.service #禁止firewall开机启动
```

### 查看防火墙状态(关闭后显示not running，开启后显示running)
```shell
firewall-cmd  --state
```

### firewall-cmd
```shell
#例如 查询9200端口是否打开
firewall-cmd    --query-port=9200/tcp
#返回yes 表示打开
#打开9300端口
firewall-cmd   --add-port=9300/tcp
#返回success表示打开成功
#移除端口
--remove-port=12/tcp
```