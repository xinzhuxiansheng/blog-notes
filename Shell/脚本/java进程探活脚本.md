#!/bin/sh
elasticPid=`ps -ef|grep elasticsearch|grep -v grep|awk '{print $2}'`
echo "elasticPid is ${elasticPid}"
if [[ $elasticPid -eq '' ]];then
        echo "not alive"
        elasticsearch -d
fi
dsymPublishPid=`ps -ef|grep dsymPublish|grep -v grep|awk '{print $2}'`
if [[ $dsymPublishPid -eq '' ]];then
        cd /Users/crash/dsymPublish;
        nohup java -jar /Users/crash/dsymPublish/dsymPublish-1.0.jar > app.log &
fi
crashWorkerPid=`ps -ef|grep crashWorker|grep -v grep|awk '{print $2}'`
echo "crashworkerPid is ${crashWorkerPid}"
if [[ $crashWorkerPid -eq '' ]];then
        cd /Users/crash/crash_worker_new;
        nohup java -jar  crashWorker.jar > app.log &
fi