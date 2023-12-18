## 使用 pem密钥文件 登录 SSH 提示权限不够

### 异常信息

```shell
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@         WARNING: UNPROTECTED PRIVATE KEY FILE!          @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
Permissions 0664 for 'xxxxxxxxxxxx' are too open.
It is required that your private key files are NOT accessible by others.
This private key will be ignored.
Load key "xxxxxxxxxxxxx": bad permissions
root@xxx.xx.xxx.164: Permission denied (publickey,gssapi-keyex,gssapi-with-mic).
```

### 解决    
```shell
chmod 700 ~/.ssh/
chmod 600 ~/xxx/xx.pem
```
