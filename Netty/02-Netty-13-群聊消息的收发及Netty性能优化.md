# Netty - 群聊消息的收发及Netty性能优化    

## 引言         
已经学习了如何创建群聊并通知群聊的各位成员。本章中我们来实现群成员管理，包括群的加入、退出和获取群成员列表等功能。     


## 运行演示         

**NettyServer**     
```bash
Thu Jan 02 00:55:24 CST 2025: 端口[8000]绑定成功!
[yzhou]登录成功
[jj]登录成功
[lisi]登录成功
群创建成功，id 为 e782f80c, 群里面有：[yzhou, jj, lisi]
```

**NettyClient01**
```bash
Thu Jan 02 00:56:09 CST 2025: 连接成功，启动控制台线程……
输入用户名登录: yzhou
[yzhou]登录成功，userId 为: b6a104e5
createGroup
【拉人群聊】输入 userId 列表，userId 之间英文逗号隔开：b6a104e5,976133d7,867a3734
群创建成功，id 为[e782f80c], 群里面有：[yzhou, jj, lisi]
sendToGroup
发送消息给某个某个群组：e782f80c 大家好
收到群[e782f80c]中[b6a104e5:yzhou]发来的消息：大家好
```

**NettyClient02**
```bash
Thu Jan 02 00:56:18 CST 2025: 连接成功，启动控制台线程……
输入用户名登录: jj
[jj]登录成功，userId 为: 976133d7
群创建成功，id 为[e782f80c], 群里面有：[yzhou, jj, lisi]
收到群[e782f80c]中[b6a104e5:yzhou]发来的消息：大家好
```

**NettyClient03**
```bash
Thu Jan 02 00:56:24 CST 2025: 连接成功，启动控制台线程……
输入用户名登录: lisi
[lisi]登录成功，userId 为: 867a3734
群创建成功，id 为[e782f80c], 群里面有：[yzhou, jj, lisi]
收到群[e782f80c]中[b6a104e5:yzhou]发来的消息：大家好
```

