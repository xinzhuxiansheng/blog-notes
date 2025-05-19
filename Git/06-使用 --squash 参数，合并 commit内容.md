```bash
方法 1：使用 git merge --squash（推荐）
适用场景：
你有一个分支 feature，想合并到 main，但不想保留 feature 的所有 commit 历史。
你希望 feature 的所有更改在 main 上只显示为一个 commit。
操作步骤
切换到目标分支（main）：
git checkout main
执行 git merge --squash：
git merge --squash feature
这个命令会把 feature 的所有更改暂存（staged），但不会自动提交。
手动提交合并后的更改：
git commit -m "Merge feature branch (squashed)"
这样，feature 的所有 commit 会被压缩成一个 commit 合并到 main。
```