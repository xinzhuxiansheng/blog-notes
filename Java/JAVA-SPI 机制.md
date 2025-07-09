# JAVA - SPI 机制  

SPI（Service Provider Interface）是Java中比较经典的一种机制类型，概括来说，SPI是“基于接口的编程+策略模式+配置文件”组合实现的动态加载机制。在1.9之后的Flink版本中对SPI的使用随处可见，Table Connector的加载是其中的一个典型使用。