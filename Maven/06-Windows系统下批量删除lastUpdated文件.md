## Windows系统下批量删除lastUpdated文件

创建`clearLastUpdated.bat`,内容如下：
```
@echo  off
set REPOSITORY_PATH=D:\DevSoftware\maven-repository
rem 正在搜索...
for /f "delims=" %%i in ('dir /b /s "%REPOSITORY_PATH%\*lastUpdated*"') do (
    del /s /q %%i
)
rem 搜索完毕
pause
```

>注意将内容中的REPOSITORY_PATH改成自己本地maven路径即可     

