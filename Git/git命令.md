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

git fetch orgxxx:localxxx   orgxxx表示远程分支名，localxxx表示是本地的分支名
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

## tag相关操作
```shell
git tag -a v1.0 -m 'tagging Version 1.0'
git checkout tags/v1.0
git checkout tags/v1.0 -b NewBranch

#新建tag
git tag tag_name
#将本地tag推送到远程仓库
git push origin tagname
#删除本地tag
git tag -d tag_name
#删除远程tag
git push origin :refs/tags/tag_name

#将分支推送到origin
git push origin remote_name --tags

#同步upstream的tag
git fetch upstream --tags
#同步到origin中去
git push --tags
```


# tag 相关操作
https://blog.csdn.net/DinnerHowe/article/details/79082769



# git 从远程仓库获取所有分支

git clone xxx
git branch -r | grep -v '\->' | while read remote; do git branch --track "${remote#origin/}" "$remote"; done
git fetch --all
git pull --all


# git 修改remote

方式1、直接修改：

git remote set-url origin xxxxx.git

方式2、先删后加 ：

git remote rm origin
git remote add origin xxxxx.git

修改默认pull和push分支：

git branch --set-upstream-to=origin/develop develop
origin/develop develop为要设置的默认分支


# git 只提交部分文件
 

1、git status //查看修改文件状态

2、git add  //将想要提交的文件add到本地库

4、git status  //查看修改文件状态

3、git commit  //提交add到本地库的文件

5、git  stash //将其他修改文件

6、git pull  origin dev //拉取远程代码合并到本地

7、git push  提交commit的文件

8、git stash pop //回复存储区的文件




# 查看当前分支从哪个分支或者tag checkout的
git log --oneline --graph --decorate



# Git 上传过大文件后，影响后续提交
```
remote: error: See http://git.io/iEPt8g for more information.
remote: error: File Kafka/dockerimage/jdk-8u212-linux-x64.tar.gz is 185.98 MB; this exceeds GitHub's file size limit of 100.00 MB
remote: error: GH001: Large files detected. You may want to try Git Large File Storage - https://git-lfs.github.com.
To github.com:xinzhuxiansheng/blog-notes.git
 ! [remote rejected] master -> master (pre-receive hook declined)
error: 推送一些引用到 'github.com:xinzhuxiansheng/blog-notes.git' 失败
```

将 大文件删除，光使用 	
git filter-branch -f --index-filter 'git rm --cached --ignore-unmatch YOUR-FILE'



# Git 撤销更改
我查了下有两个相关的命令：
1. git clean -df
2. git reset --hard
但问题是第一个命令只删除所有untracked的文件，如果文件已经被tracked, 修改过的文件不会被回退。而第二个命令只把tracked的文件revert到前一个版本，对于untracked的文件(比如编译的临时文件)都不会被删除。

## PR的提交记录，之前如何清楚

//在每次提完PR之后，checkout到源分支上reset head，再push -f
git checkout origin/2.0   //我得源分支
git log                   //查看最初的head
git reset commitId --hard //强制reset
git push -f               //强制提交



### 修改分支 
https://www.jianshu.com/p/cc740394faf5