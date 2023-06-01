## 解决GoLand提示dlv版本过低问题   

### 异常描述    
在GoLand IDE中调试go代码时，控制台会提示下面的警告信息，这是因为GoLand内置有一个delve，由于我下载的GoLand是老版本，而Go是新版本，所以它提示我`内置delve的版本过低`。

```shell
API server listening at: 127.0.0.1:54523
WARNING: undefined behavior - version of Delve is too old for Go version 1.20.4 
(maximum supported version 1.19)
```

### 解决方法    
首先，先通过`go install`下载最新的`dlv.exe`。   
```
go install github.com/go-delve/delve/cmd/dlv@latest
``` 

该文件目录是：`$GOPATH/bin/dlv.exe` ，然后在GoLand IDE中添加`dlv.path={新的dlv路径}`自定义属性即可。    

IDE 操作路径为： Help ->  Edit Custom Properties... 新增以下内容：  
```shell
dlv.path=E:\\Code\\Go\\bin\\dlv.exe
```

>请注意：Windows下路径一定要用\\来分隔，不然不能生效。

添加好之后，重启Goland即可。
