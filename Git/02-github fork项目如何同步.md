## github fork项目如何同步

>根据"01-github 如何发起PR.md" 我们知道如何fork了，那如何同步原始仓库?

`fork github项目只会将目标仓库的default分支给同步到自己的仓库中`

### 同步tags

```shell
#同步upstream的tag
git fetch upstream --tags
#同步到origin中去
git push --tags
```

此时，你可以看到github 自己的项目中的tag，已经将原仓库的tag全部同步

### 同步分支

这里通过checkout 克隆出upstream分支，然后将分支push到origin