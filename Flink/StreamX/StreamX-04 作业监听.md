
## ingress创建
~~~
apiVersion: extensions/v1beta1
kind: Ingress
metadata:
  annotations:
    kubernetes.io/ingress.class: traefik
  labels:
    app: flink-yzhoutest-cluster0003
  name: flink-yzhoutest-cluster0003
  namespace: yzhou
spec:
  rules:
  - host: flink-yzhoutest-cluster0003.ark-tke-8215.bitxxxxx.com
    http:
      paths:
      - backend:
          serviceName: flink-yzhoutest-cluster0003-rest
          servicePort: 8081
        path: /
~~~



curl -H "Content-Type: application/json" -X POST  -d '{"user_id": "123", "coin":100, "success":1, "msg":"OK!" }' "https://chrome-infra-packages.appspot.com/prpc/cipd.Repository/ResolveVersion"


"https://chrome-infra-packages.appspot.com/prpc/cipd.Repository/ResolveVersion"