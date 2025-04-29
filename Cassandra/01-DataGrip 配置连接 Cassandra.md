# DataGrip 配置连接 Cassandra   

## 配置参数  
如图片所示，配置以下参数。   
![lianjie01](http://img.xinzhuxiansheng.com/blogimgs/cassandra/lianjie01.jpg)  

特别注意：若 Cassandra Driver 版本选择不对，则会报以下异常信息：        
```bash
DBMS: Apache Cassandra (no ver.)
Case sensitivity: plain=mixed, delimited=exact
Unexpected error while creating connection.
Since you provided explicit contact points, the local DC must be explicitly set (see basic.load-balancing-policy.local-datacenter in the config, or set it programmatically with SessionBuilder.withLocalDatacenter). Current contact points are: Node(endPoint=10.xx.xx.xx/<unresolved>:8635, hostId=xxxxx-a24d-4fff-be41-xxxx, hashCode=2795ae0d)=xxx. Current DCs in this cluster are: xxx.  
```

只需要将 Cassandra Driver 版本选择 1.4 就可以了，如下图所示    
![lianjie02](http://img.xinzhuxiansheng.com/blogimgs/cassandra/lianjie02.jpg)
