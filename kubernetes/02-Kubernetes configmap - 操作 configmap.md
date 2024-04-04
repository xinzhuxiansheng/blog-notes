## 操作 ConfigMap    

### 创建 ConfigMap   




查看 yaml 
kubectl -n drc-test get configmap hdfs-conf -p yaml > config.yaml


创建 configmap
kubectl -n drc-test create configmap ker-conf  --from-file=krb5.conf=krb5.conf --from-file=rap.keytab=kafka.keytab

修改 flinkdep 的yaml
kubectl -n drc-test edit flinkdep xsj-kafka2-mysql-02-adf


应用
1.添加 volumeMounts 
```
        volumeMounts:
        - mountPath: /opt/flink/downloads
          name: downloads
        - mountPath: /hdfsconfig
          name: hdfs-config
        - mountPath: /krbconfig
          name: krb-config
```


2.添加 volumes
```
      volumes:
      - configMap:
          defaultMode: 511
          items:
          - key: hdfs-site.xml
            path: hdfs-site.xml
          - key: core-site.xml
            path: core-site.xml
          - key: krb5.conf
            path: krb5.conf
          - key: rap.keytab
            path: rap.keytab
          name: hdfs-conf
        name: hdfs-config
      - configMap:
          defaultMode: 511
          items:
          - key: krb5.conf
            path: krb5.conf
          - key: rap.keytab
            path: rap.keytab
          name: ker-conf
        name: krb-config
```

3.目录在 `/krbconfig/`下 