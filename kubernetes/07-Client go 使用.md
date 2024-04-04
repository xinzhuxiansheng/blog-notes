# Kubernetes API 

## 开启 proxy   
```bash
kubectl proxy           
```

## 访问 Kubernetes REST API         
```bash
➜  ~ curl http://127.0.0.1:8001/api/v1/namespaces/default/pods
{
  "kind": "PodList",
  "apiVersion": "v1",
  "metadata": {
    "resourceVersion": "5072"
  },
  "items": []
}
```





```bash
curl http://127.0.0.1:8001/api/v1/namespaces/default/pods
```

