
## idea读取properties文件中文乱码问题

### 背景
在spring boot项目中使用 `@Value`关键字做属性值注入到字段时发现中文乱码。

### 解决方法

* 在菜单栏按照右侧路径打开 File Encodings `File` -> `Settings` -> `Editor` -> `File Encodings` 。
* 在窗口下方的`Properties Files`栏中，勾选`Transparent native-to-ascii convertion`即可

>注意： 如果勾选完后若还是乱码，并且重启也不好使。可能是因为缓存导致。这需要在properties文件中随便添加点文字，再测试一遍应该是可以了，这样可以让之前缓存失效。
