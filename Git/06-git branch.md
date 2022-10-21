
## 分支操作

### 创建分支

```shell
git checkout -b [new branch]
```

### 切换分支

```shell
git switch [branch name]
```

### 将远程某个分支拉到本地

```shell
git checkout -b dev origin/dev ，作用是checkout远程的dev分支，在本地起名为dev分支，并切换到本地的dev分支
git fetch orgxxx:localxxx   orgxxx表示远程分支名，localxxx表示是本地的分支名
```

### 删除分支

```shell
# 删除本地分支
git branch -D [branchName]

# 删除远程分支
git push origin --delete [branchName]
```