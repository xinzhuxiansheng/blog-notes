## CentOS7修改Hosts映射

```shell    
# 示例
cat >> /etc/hosts << EOF
192.168.56.101 master
192.168.56.102 node1
192.168.56.103 node2
EOF
```