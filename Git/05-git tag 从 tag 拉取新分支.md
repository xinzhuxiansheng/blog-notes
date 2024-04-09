# git tag 从 tag 拉取新分支 

## 操作 
```shell
# 拉取 branch、tag
git fetch --all

# 根据tag创建新的分支 
git branch <new-branch-name> <tag-name>

# 使用 switch 切换到新的分支 
git checkout newbranch  

# 把本地创建的分支提交到远程仓库 
git push origin newbranch
```