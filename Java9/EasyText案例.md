**`正文`**

[TOC]

## 简介
>EasyText 是一个分析文本复杂性的应用程序， 而我们关注的不是文本分析算法，而是构成EasyText应用程序的模块组合，主要目标是使用Java模块来创建灵活且可维护的应用程序；

`以下是EasyText模块化实现所需满足的要求：`
* 必须能够在不修改或者不重新编译现有模块的情况下添加新的分析算法。
* 不同的前端（如GUI和命令行） 必须能够重用相同的分析逻辑
* 必须支持不同的配置，同时无需重新编译，也无需部署每个配置的所有代码。

> 文本的复杂度 请参考《Java9 模块开发 核心原则与实践》 p42

`从功能角度来看，文本分析由以下步骤组成：`
1) 读取输入文本(从文件，GUI或者其他地方读取)
2) 将文本拆分成句子和单词(因为许多可读性公式需要使用句子或单词)
3) 对文本进行一次或者多次分析
4) 向用户显示结果




