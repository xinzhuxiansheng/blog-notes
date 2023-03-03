## CentOS7升级内核

要升级CentOS 7的内核，可以按照以下步骤操作：

1. 查看系统版本
```shell
uname -sr
```

2. 更新系统
首先，运行以下命令来更新系统：
```shell
# 这这将会更新系统上已安装的所有软件包，包括内核
sudo yum update
```

3. 安装 ELRepo 仓库
要升级内核，需要使用 ELRepo 仓库，该仓库提供了最新的稳定版内核。运行以下命令来安装 ELRepo 仓库：
```shell
sudo rpm --import https://www.elrepo.org/RPM-GPG-KEY-elrepo.org
sudo rpm -Uvh https://www.elrepo.org/elrepo-release-7.el7.elrepo.noarch.rpm
```

4. 安装新内核
运行以下命令来列出可用的内核包：
```shell
sudo yum --disablerepo="*" --enablerepo="elrepo-kernel" list available
```
这将列出所有可用的内核包。选择最新的稳定版内核，并运行以下命令来安装它：    
```shell
sudo yum --enablerepo=elrepo-kernel install kernel-ml
```
这将会安装最新版的稳定版内核

5. 更新GRUB2配置
运行以下命令来更新 GRUB2 配置文件：
```shell
sudo grub2-mkconfig -o /boot/grub2/grub.cfg
```

最后，运行以下命令来重启系统：
```
sudo reboot

uname -sr
```

