

```bash
FROM rockylinux/rockylinux:9.6

COPY ./jdk-11.0.26 /root/jdk-11.0.26

ENV JAVA_HOME=/root/jdk-11.0.26
ENV PATH=${JAVA_HOME}/bin:${PATH}

RUN java -version

CMD ["sleep", "infinity"]
```