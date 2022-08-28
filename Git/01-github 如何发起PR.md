## git new pull request 操作流程
```shell
fork 原始仓库
clone 自己的仓库
查看你的远程仓库的路径
git  remote -v

配置原仓库的路径
git  remote add upstream git-ssh-地址

再次查看远程目录地址
Git  remote -v

获取原仓库的修改文件
Git fetch upstream


#在 master 分支添加原始仓库为远程分支 git remote add upstream 远程仓库

自己分支开发，如 dev 分支开发：git checkout -b dev
本地 dev 提交
切换 master 分支，同步原始仓库：git checkout master， git pull upstream master
切换本地 dev 分支，合并本地 master 分支（已经和原始仓库同步），可能需要解冲突
提交本地 dev 分支到自己的远程 dev 仓库
现在才是给原始仓库发 pull request 请求
等待原作者回复（接受/拒绝）

```