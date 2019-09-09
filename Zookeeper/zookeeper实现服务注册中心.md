**`正文`**

[TOC]

## zookeeper api基本使用
```java



if(client.checkExists().forPath(ZkNode.SLAVE_NODE_PATH)==null) {
    // 创建临时节点
    client.create()
            .creatingParentsIfNeeded()
            .withMode(CreateMode.EPHEMERAL_SEQUENTIAL)
            .forPath(ZkNode.SLAVE_NODE_PATH, StrUtil.utf8Bytes(node.toString()));
}
```