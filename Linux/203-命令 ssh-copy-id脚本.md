## ssh-copy-id 脚本使用

`ssh-copy-id` 是一个用于安装你的公钥到远程服务器的脚本，它允许你通过 SSH 连接到远程服务器而无需每次输入密码。这可以大大提高远程服务器操作的便捷性。以下是如何使用 `ssh-copy-id` 的基本步骤：    

**1.在本地机器上创建 SSH 公钥和私钥**：如果你还没有 SSH 公钥和私钥，你可以使用 `ssh-keygen` 命令来创建。这将会在你的用户目录下的 `.ssh` 文件夹中创建两个文件：`id_rsa`（私钥）和 `id_rsa.pub`（公钥）。         
```shell
ssh-keygen
``` 

在执行 `ssh-keygen` 命令时，你可以选择输入一个密码，也可以选择不输入密码。如果你输入了一个密码，那么每次使用私钥时都需要输入这个密码。如果你选择不输入密码，那么任何人都可以使用这个私钥。      

**2.复制公钥到远程服务器**：使用 `ssh-copy-id` 命令来复制你的公钥到远程服务器。你需要提供你的用户名和远程服务器的地址。例如，如果你的用户名是 `user`，远程服务器的地址是 `remote.server.com`，你可以使用以下命令：       
```shell
ssh-copy-id user@remote.server.com  
```     

在执行这个命令时，你需要输入你在远程服务器的密码。这个密码是用于验证你有权限安装公钥的。一旦公钥被安装，你就不再需要这个密码了。    

**3.验证 SSH 公钥登录**：现在你可以尝试使用 SSH 公钥登录到远程服务器。你应该可以在不输入密码的情况下登录到远程服务器。       
```shell
ssh user@remote.server.com  
``` 

请注意，`ssh-copy-id` 命令可能在某些操作系统中不可用。如果你无法使用 `ssh-copy-id` 命令，你可以手动复制公钥到远程服务器。你只需要将公钥复制到远程服务器的 `~/.ssh/authorized_keys` 文件中即可。你可以使用以下命令来做这件事：       

```shell
cat ~/.ssh/id_rsa.pub | ssh user@remote.server.com "mkdir -p ~/.ssh && chmod 700 ~/.ssh && cat >> ~/.ssh/authorized_keys"
```

这个命令首先在远程服务器上创建了 `~/.ssh` 文件夹（如果还没有的话），然后将你的公钥追加到 `~/.ssh/authorized_keys` 文件中。          

