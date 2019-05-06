**`正文`**
[TOC]

## 关闭应用
```shell
#单应用
pid=`ps -ef|grep java|grep mainappops-configmanage-1.0.0-SNAPSHOT.war | awk '{print $2}'`

#echo $pid 
if [[ "$pid" != "" ]]; then
    kill -9 $pid
fi


#多应用
pid=`ps -ef|grep java|grep springcloud-eureka-1.0-SNAPSHOT.jar | awk '{print $2}'`

#echo $pid 
for i in $pid;
do
if [[ "$i" != "" ]]; then
    kill -9 $i
fi
echo $i;
done
```

## 启动应用
```shell
cd /data/app/mainappops-configmanage/
nohup java -jar mainappops-configmanage-1.0.0-SNAPSHOT.war >> app.log &
```