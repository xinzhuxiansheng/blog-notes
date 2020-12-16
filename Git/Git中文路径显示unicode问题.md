
当被修改的文件中带有中文字符时，中文字符会被转换为unicode代码，看不出原来的文件名。
这时，只要配置：
```shell
git config --global core.quotepath false
```