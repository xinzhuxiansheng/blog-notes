
## 集群特点：
* 分布式多台物理主机部署
* 依赖于Java8或者Java11 JDK环境
* 仅支持Session模式提交Job
* 支持高可用配置


## Standalone模式(单节点)
1. 下载`flink-1.14.0-bin-scala_2.12.tgz`，执行 tar -zxf flink-1.14.0-bin-scala_2.12.tgz解压flink。
2. 执行 ./start-cluster.sh脚本，启动flink

>注意：
Standalone模式启动 start-cluster.sh报错
```
# sh start-cluster.sh
/data/installDirs/flink-1.14.0/bin/config.sh: line 32: syntax error near unexpected token `<'
/data/installDirs/flink-1.14.0/bin/config.sh: line 32: `    done < <(find "$FLINK_LIB_DIR" ! -type d -name '*.jar' -print0 | sort -z)'
```
不要使用sh start-cluster.sh运行，直接运行 ./start-cluster.sh

启动完成后，浏览器访问 http://localhost:8081,访问flink页面。

**注意参数有**  
```
# The number of task slots that each TaskManager offers. Each slot runs one parallel pipeline.  

taskmanager.numberOfTaskSlots: 1  

# The parallelism used for programs that did not specify and other parallelism.    

parallelism.default: 1
```