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


# tag 相关操作  (需要整理)
https://blog.csdn.net/DinnerHowe/article/details/79082769