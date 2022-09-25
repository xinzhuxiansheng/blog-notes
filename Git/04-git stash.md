## git stash用法

### 1. stash当前修改

git stash会把所有未提交的修改（包括暂存的和非暂存的）都保存起来，用于后续恢复当前工作目录。 比如下面的中间状态，通过git stash命令推送一个新的储藏，当前的工作目录就干净了。

**操作流程案例**    
```shell
# 查看发生改变的文件
git status

# 将改动文件 存储本地缓存中
git stash

# 若给每次stash 加一个注释，例如git commit -m '注释'
git stash save "注释"
```

>这里需要单独说明一个场景 只将部分文件放入stash中，这在github中提交PR是经常存在的   

需要用到 git stash -p的命令，它是一个交互式命令，我们可以一个文件一个文件的遍历，决定每个文件的操作方式
**操作流程案例**        
```shell
git stash -p
```

(1/1) Stash this hunk [y,n,q,a,d,e,?]? 分别代表的含义如下： （输入 ?查看命令含义）

```shell
(1/1) Stash this hunk [y,n,q,a,d,e,?]? ?
y - stash this hunk
n - do not stash this hunk
q - quit; do not stash this hunk or any of the remaining ones
a - stash this hunk and all later hunks in the file
d - do not stash this hunk or any of the later hunks in the file
e - manually edit the current hunk
? - print help
```

* 遇到我们需要stash的文件，输入`y`
* 不需要stash需要commit的文件，输入`n`
* 如果接下来没有需要stash的文件，就直接输入`q`退出就行


### 2. 重新应用缓存的stash

可以通过git stash pop命令恢复之前缓存的工作目录，输出如下：

**操作流程案例**   
```shell
# 移除stash中 最新的一次，并且会删除移除的stash
git stash pop

# 若需要将stash，并不删除移除stash（可以通过指定使用哪个stash，默认使用最新额stash stash@{0} 序号）
git stash apply
```


### 3. 查看现有stash

可以使用git stash list命令，一个典型的输出如下：

**操作流程案例**  
```shell
git stash list

stash@{0}: WIP on master: 049d078 added the index file
stash@{1}: WIP on master: c264051 Revert "added file_size"
stash@{2}: WIP on master: 21d80a5 added number to log
```

## 4. 移除stash

可以使用git stash drop命令，后面可以跟着stash名字。下面是一个示例：

```shell
$ git stash list
stash@{0}: WIP on master: 049d078 added the index file
stash@{1}: WIP on master: c264051 Revert "added file_size"
stash@{2}: WIP on master: 21d80a5 added number to log
$ git stash drop stash@{0}
Dropped stash@{0} (364e91f3f268f0900bc3ee613f9f733e82aaed43)
```

或者使用git stash clear命令，删除所有缓存的stash。

## 5. 查看指定stash的diff

可以使用git stash show命令，后面可以跟着stash名字。示例如下：

```shell
$ git stash show
 index.html | 1 +
 style.css | 3 +++
 2 files changed, 4 insertions(+)
```

在该命令后面添加-p或--patch可以查看特定stash的全部diff，如下：

```shell
$ git stash show -p
diff --git a/style.css b/style.css
new file mode 100644
index 0000000..d92368b
--- /dev/null
+++ b/style.css
@@ -0,0 +1,3 @@
+* {
+  text-decoration: blink;
+}
diff --git a/index.html b/index.html
index 9daeafb..ebdcbd2 100644
--- a/index.html
+++ b/index.html
@@ -1 +1,2 @@
+<link rel="stylesheet" href="style.css"/>
```

## 6. 从stash创建分支

如果你储藏了一些工作，暂时不去理会，然后继续在你储藏工作的分支上工作，你在重新应用工作时可能会碰到一些问题。如果尝试应用的变更是针对一个你那之后修改过的文件，你会碰到一个归并冲突并且必须去化解它。如果你想用更方便的方法来重新检验你储藏的变更，你可以运行 git stash branch，这会创建一个新的分支，检出你储藏工作时的所处的提交，重新应用你的工作，如果成功，将会丢弃储藏。

```shell
$ git stash branch testchanges
Switched to a new branch "testchanges"
# On branch testchanges
# Changes to be committed:
#   (use "git reset HEAD <file>..." to unstage)
#
#      modified:   index.html
#
# Changes not staged for commit:
#   (use "git add <file>..." to update what will be committed)
#
#      modified:   lib/simplegit.rb
#
Dropped refs/stash@{0} (f0dfc4d5dc332d1cee34a634182e168c4efc3359)
```

这是一个很棒的捷径来恢复储藏的工作然后在新的分支上继续当时的工作。

## 7. 暂存未跟踪或忽略的文件

默认情况下，git stash会缓存下列文件：

    添加到暂存区的修改（staged changes）
    Git跟踪的但并未添加到暂存区的修改（unstaged changes）

但不会缓存一下文件：

    在工作目录中新的文件（untracked files）
    被忽略的文件（ignored files）

git stash命令提供了参数用于缓存上面两种类型的文件。使用-u或者--include-untracked可以stash untracked文件。使用-a或者--all命令可以stash当前目录下的所有修改。

至于git stash的其他命令建议参考Git manual