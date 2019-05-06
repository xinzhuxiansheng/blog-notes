**`正文`**
[TOC]

## Log4j2的onMatch和onMismatch属性值讲解
>onMatch和onMismatch都有三个属性值，分别为Accept、DENY和NEUTRAL

分别介绍这两个配置项的三个属性值：

onMatch="ACCEPT" 表示匹配该级别及以上
onMatch="DENY" 表示不匹配该级别及以上
onMatch="NEUTRAL" 表示该级别及以上的，由下一个filter处理，如果当前是最后一个，则表示匹配该级别及以上
onMismatch="ACCEPT" 表示匹配该级别以下
onMismatch="NEUTRAL" 表示该级别及以下的，由下一个filter处理，如果当前是最后一个，则不匹配该级别以下的
onMismatch="DENY" 表示不匹配该级别以下的
