
```shell
$ sudo nano /etc/netplan/00-installer-config.yaml   
```

```yaml
# This is the network config written by 'subiquity'  
network:
  ethernets:
    ens33:
      dhcp4: false
      addresses: [192.168.0.211/24]
      gateway4: 192.168.0.1
      nameservers:
        addresses:
        - 192.168.0.1
        search:
        - 192.168.0.1
        - 8.8.8.8
        - 114.114.114.114
  version: 2
```

```shell
$ sudo netplan apply
```


refer   
1.https://bbs.huaweicloud.com/blogs/406313  
