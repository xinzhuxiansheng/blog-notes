# Spark - Spark on Yarn Job 示例  

>Spark version: 3.5.5  


## 安装 Spark on Yarn 环境   
访问 `https://archive.apache.org/dist/spark/spark-3.5.5/spark-3.5.5-bin-hadoop3.tgz` 下载 Spark 3.5.5 安装包   

### 修改 conf/spark-env.sh 配置文件  
```bash
# 重命名 spark-env.sh.template 为 spark-env.sh  
mv conf/spark-env.sh.template conf/spark-env.sh

# 修改 spark-env.sh 文件 
vim conf/spark-env.sh 
# 内容如下：  
export JAVA_HOME=/opt/module/jdk1.8.0_451
export HADOOP_CONF_DIR=/opt/module/hadoop-3.2.0/etc/hadoop
```  

### 启动 on Yarn Job 
在 spark 目录下执行以下命令提交 Spark Job   
```bash
./bin/spark-submit \
    --class org.apache.spark.examples.SparkPi \
    --master yarn \
    --deploy-mode cluster \
    examples/jars/spark-examples_2.12-3.5.5.jar \
    2
```

输出结果如下：  
![sparkonyarn01](images/sparkonyarn01.png)  

在 访问 Yarn WEB UI (`http://bigdata02:8088`)，查看作业运行情况。    
![sparkonyarn02](images/sparkonyarn02.png)       
