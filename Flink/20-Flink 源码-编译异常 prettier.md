# Flink 源码 - 编译异常 Delete "␍"  prettier/prettier

>Flink version: 1.15.4  

## 问题描述  
```bash
[INFO] > flink-dashboard@2.0.0 lint
[INFO] > eslint --cache src --ext .ts,.html && stylelint "**/*.less"
[INFO] 
[INFO] 
[INFO] src/app/app.component.less
[INFO]    1:3   ✖  Delete "␍"  prettier/prettier
[INFO]    2:62  ✖  Delete "␍"  prettier/prettier
[INFO]    3:64  ✖  Delete "␍"  prettier/prettier
[INFO]    4:57  ✖  Delete "␍"  prettier/prettier
```  

## 解决  
参考 flink 根目录下 `flink-runtime-web/web-dashboard` 模块的 package.json 文件中的 scripts。 它定义了 `lint:fix` 命令帮我们修复 windows 与 Linux 行末换行的差异。  

```json
{
  "name": "flink-dashboard",
  "version": "2.0.0",
  "scripts": {
    "ng": "node node_modules/@angular/cli/bin/ng",
    "start": "ng serve",
    "build": "ng build --configuration production --base-href ./",
    "test": "ng test",
    "lint": "eslint --cache src --ext .ts,.html && stylelint \"**/*.less\"",
    "lint:fix": "eslint --fix --cache src --ext .ts,.html && stylelint \"**/*.less\" --fix",   // 该命令
    "ci-check": "npm run lint && npm run build",
    "proxy": "ng serve --proxy-config proxy.conf.json",
    "prepare": "cd ../.. && husky install flink-runtime-web/web-dashboard/.husky",
    "lint-staged": "lint-staged"
  },
``` 

在命令行执行 `npm run lint:fix` 即可修复。 
```bash
E:\Code\Java\flink-all\flink_release-1.15.4\flink-runtime-web\web-dashboard>npm run lint:fix

> flink-dashboard@2.0.0 lint:fix
> eslint --fix --cache src --ext .ts,.html && stylelint "**/*.less" --fix
```