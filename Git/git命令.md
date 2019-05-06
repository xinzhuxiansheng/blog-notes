**`正文`**
[TOC]

## 初始化项目
```shell
 命令行指令
Git 全局设置

git config --global user.name "用户名"
git config --global user.email "邮箱地址"

创建新版本库

git clone git@git.xxxxx:xxxxx/xxxxxx.git
cd [项目目录]
touch README.md
git add README.md
git commit -m "add README"
git push -u origin master

已存在的文件夹

cd existing_folder
git init
git remote add origin git@git.xxxxx:xxxxx/xxxxxx.git
git add .
git commit -m "Initial commit"
git push -u origin master

已存在的 Git 版本库

cd existing_repo
git remote rename origin old-origin
git remote add origin git@git.xxxxx:xxxxx/xxxxxx.git
git push -u origin --all
git push -u origin --tags


```


## 创建分支
```shell
git branch test    #创建分支
```
## 切换分支
```shell
git checkout test  #切换本地分支
```

## 将远程某个分支拉到本地
```shell
git checkout -b dev origin/dev ，作用是checkout远程的dev分支，在本地起名为dev分支，并切换到本地的dev分支
```

## 删除本地分支
```shell
git branch -d [branchName]
```

## 删除远程分支
```shell
git push origin --delete [branchName]
```

## 撤销git add 的文件
```shell
git reset HEAD 如果后面什么都不跟的话，就是上一次add里面的全部撤销了
git reset HEAD xxx/xxx/x.java 就是对某个文件进行撤销了
#撤销add 操作，不是还原文件
```

## 撤销上次本地的commit


## 删除已经跟踪的文件或者文件夹
```shell
git rm --cached 文件名
git rm --cached -r 文件夹

删除之后，重新提交代码即可
```

## fetch 命令
```shell
#取回所有分支的更新
git fetch

#只想取回特定分支的更新，可以指定分支名
git fetch origin <分支名>
```

## 恢复到某个 commit id去
```shell
git reset --hard commitid        //本地代码回到指定的commitid
git push -f origin branchname    //git服务器代码回到指定的commitid
```


## 同步本地的远程分支 
```shell
#1. 查看本地分支和追踪情况
git remote show origin

#2. 同步删除其他分支
git remote prune origin
```

