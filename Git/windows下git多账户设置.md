--In Blog
--Tags: 

# windows下git多账户设置

## 生成ssh密钥
```shell
ssh-keygen -t rsa -C 邮箱地址
#在Enter file in which to save the key  行中，输入： id_rsa_XXXX
#剩下的步骤，直接回车即可
```

## 在.ssh目录中创建config文件并完成相关配置
    每个账号单独配置一个Host，每个Host要取一个别名，每个Host主要配置HostName和IdentityFile两个属性即可
    Host的名字可以单独自定义名称，不过，这个会影响git相关命令，例如：Host mygithub这样定义，则命令改成，git@后面紧跟的名称改成mygithub
    git clone git@mygithub:xxxxxxxxxxx

>HostName   真实的域名地址
IdentityFile    这里是id_rsa的地址
PreferredAuthentications    配置登录时用什么权限认证--可设为publickey,password publickey,keyboard-interactive等
User    配置使用用户名

**`config`内容如下**
```xml
# 配置github.com
Host github.com                 
    HostName github.com
    IdentityFile ~/.ssh/id_rsa_github
    User username1
```

## 解决 no matching host key type found. Their offer: ssh-rsa 异常
```
// 添加以下参数
HostKeyAlgorithms +ssh-rsa
PubkeyAcceptedKeyTypes +ssh-rsa
```

完整示例：  
```
Host xxx.xxx.xxx.xxx
  AddKeysToAgent yes
  UseKeychain yes
  Port xxx
  HostKeyAlgorithms +ssh-rsa
  PubkeyAcceptedKeyTypes +ssh-rsa
  IdentityFile ~/.ssh/github
```

