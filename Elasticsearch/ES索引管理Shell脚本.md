**`正文`**

[TOC]

## 批量关闭索引 
```
POST /app-md-2018-12-1*/_close
POST /app-omd-2018-12-1*/_close
POST /app-trace-2018-12-0*/_close
POST /app-start-2018-12-1*/_close
POST /plugin-start-2018-12-1*/_close
```


## 索引管理脚本
```python
#! /usr/bin/env python
#_*_coding:utf-8_*_

import urllib
import requests
import datetime

global nowtime

#某个ES集群全部索引
def getAllIndices(url,indice):

    indice_arr = []
    #请求集群全部索引
    #url  http://xx.xx.xx.xx:9201/_cat/indices/app-md*?h=i
    response = urllib.request.urlopen(url)
    resp =  response.readlines()
    for item_line in resp:
        indice_arr.append(item_line.decode('utf-8').strip('\n'))
    return indice_arr


#URL: ES索引查询地址，删除和关闭的连接， indice：索引名称 ， dateledate: 保留多少天以外， closedate： 关闭多少天以外
def clearlog(indiceurl,deleteandcloseurl,indice,dateformat,deletedate,closedate):
    if deletedate<closedate:
        print(indice + " 参数有误")
        return

    if deletedate<10 or closedate <10:
        print('deletedate 与 closedate 不能低于10天，若有特殊情况，请单独调整 脚本天数限制')
        return

    indice_arr =  getAllIndices(indiceurl,indice)
    for item_indice in indice_arr:
        #print(str[len(item_indice):])
        #print(item_indice[len(indice):])
        indice_time = datetime.datetime.strptime(item_indice[len(indice):],dateformat)
        #print('索引时间： ' + indice_time.strftime('%Y-%m-%d'))
        #比较时间差
        time_interval = (nowtime-indice_time).days
        #print('时间间隔  '+ str(time_interval))
        if time_interval>deletedate:
            deleteIndice(deleteandcloseurl+item_indice)
            pass
        if time_interval<deletedate and time_interval>closedate:
            closeIndice(deleteandcloseurl+item_indice+'/_close')
            pass

#curl -XDELETE 'localhost:9200/my_index'
def deleteIndice(url):
    print('delete: '+url)
    result = requests.delete(url)
    print(result.content)

#curl -XPOST 'localhost:9200/my_index/_close'
def closeIndice(url):
    print('close: '+url)
    result = requests.post(url)
    print(result.content)

if __name__ == '__main__':

    nowtime = datetime.datetime.now()

    print('当前时间： '+nowtime.strftime('%Y-%m-%d'))

    #app-md-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/app-md*?h=i','http://xx.xx.xx.xx:9201/','app-md-','%Y-%m-%d',15,10)
    #app-omd-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/app-omd*?h=i', 'http://xx.xx.xx.xx:9201/', 'app-omd-', '%Y-%m-%d',15, 10)
    #app-mobile-web-log-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/app-mobile-web-log*?h=i', 'http://xx.xx.xx.xx:9201/', 'app-mobile-web-log-', '%Y-%m-%d',20, 10)
    #app-perf-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/app-perf*?h=i', 'http://xx.xx.xx.xx:9201/', 'app-perf-','%Y-%m-%d', 30, 10)
    #app-trace-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/app-trace*?h=i', 'http://xx.xx.xx.xx:9201/', 'app-trace-', '%Y-%m-%d', 30, 10)
    #h5-perflog-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/h5-perflog*?h=i', 'http://xx.xx.xx.xx:9201/', 'h5-perflog-', '%Y-%m-%d', 30, 10)
    #h5-errorlog-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/h5-errorlog*?h=i', 'http://xx.xx.xx.xx:9201/', 'h5-errorlog-', '%Y-%m-%d', 30, 10)
    #imgerrorlog-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/imgerrorlog*?h=i', 'http://xx.xx.xx.xx:9201/', 'imgerrorlog-', '%Y-%m-%d', 30, 10)
    #imgperflog-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/imgperflog*?h=i', 'http://xx.xx.xx.xx:9201/', 'imgperflog-', '%Y-%m-%d', 30, 10)
    #rn-start-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/rn-start*?h=i', 'http://xx.xx.xx.xx:9201/', 'rn-start-', '%Y-%m-%d', 30, 10)
    #rn-error-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/rn-error*?h=i', 'http://xx.xx.xx.xx:9201/', 'rn-error-', '%Y-%m-%d', 30, 10)
    #anblock-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/anblock*?h=i', 'http://xx.xx.xx.xx:9201/', 'anblock-', '%Y-%m-%d', 30, 10)
    #anr-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/anr*?h=i', 'http://xx.xx.xx.xx:9201/', 'anr-', '%Y-%m-%d', 30, 10)
    #crash-xxxx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/crash*?h=i', 'http://xx.xx.xx.xx:9201/', 'crash-', '%Y-%m-%d', 30, 30)
    #app-cdn-xxxx-xx-xx-xx
    clearlog('http://xx.xx.xx.xx:9201/_cat/indices/app-cdn*?h=i', 'http://xx.xx.xx.xx:9201/', 'app-cdn-', '%Y-%m-%d-%H', 15, 15)

    print("执行完毕")
```


## 重新建立索引
```shell

```