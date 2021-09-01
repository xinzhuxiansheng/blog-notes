

有序的 znode 访问




## 配置参数
ZooKeeper中的配置文件zoo.cfg中参数含义解读如下:
* tickTime = 2000: 通信心跳时间，ZooKeeper服务器与客户端心跳时间，单位毫秒
* initLimit = 10: LF初始通信时限
Leader和Follower初始连接时能容忍的最多心跳数
* syncLimit = 5: LF同步通信时限
Leader和Follower之间通信时间如果超过syncLimit*tickTime,Leader认为Follower死掉，从服务器列表中删除Follower   
* dataDir: 保存ZooKeeper中的数据
* clientPort = 2181： 客户端连接端口


## 节点信息


## 
顺序对 ZooKeeper 非常重要；几乎接近强迫症。所有更新都是完全有序的。ZooKeeper 实际上用反映此顺序的数字标记每个更新。我们称这个数字为 zxid（ZooKeeper 事务 ID）。每次更新都会有一个唯一的 zxid。读取（和观察）按更新顺序排列。读取响应将被标记为服务读取的服务器处理的最后一个 zxid


>参考链接: https://colobu.com/2014/12/15/zookeeper-recipes-by-example-5/

## Path Cache

## Node Cache

## Tree Node