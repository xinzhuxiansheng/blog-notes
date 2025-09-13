# Python - 安装 Python2 

## 引言  
在当下（2025）的确很少再使用 python2 相关的 python脚本，并且很多 Linux 操作系统将 Python 默认为 3 版本，但的确会有特殊例外，例如部署 Atlas 服务时，它的启动命令使用的是 Python 2的语法。所以我们需要在 Linux 安装 Python2。  

>注意，不建议与 Python3 相关目录混用。  
## 安装步骤  
下载 Python 2版本的源码。执行以下命令： 
```bash
yum groupinstall "Development Tools"

./configure --prefix=/opt/python2.7 --enable-unicode=ucs4

# 并行编译
# 使用 nproc 命令，查看 CPU 核心数
make -j$(nproc) 

make install

ln -s /opt/python2.7/bin/python2.7 /usr/local/bin/python2

python2 --version
```
