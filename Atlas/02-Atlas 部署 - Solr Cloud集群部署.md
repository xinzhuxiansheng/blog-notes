# Atlas - Solr Cloud 集群部署   

>Solr on Kubernetes, Solr version: 8.11.1    


在 Atlas 部署之前，需要提前在 Solr 创建好 collection。进入 Solr Pod 执行以下命令：  
```bash
bin/solr create -c vertex_index -shards 3 -replicationFactor 2
bin/solr create -c edge_index -shards 3 -replicationFactor 2
bin/solr create -c fulltext_index -shards 3 -replicationFactor 2
```
