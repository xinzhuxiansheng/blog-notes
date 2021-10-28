# build a new image with basic centos
FROM thub.xxxxx.com.cn/project/centos:7.5.1804

RUN yum install -y sshd openssl passwd openssh-server vim net-tools wget
RUN ssh-keygen -q -t rsa -b 2048 -f /etc/ssh/ssh_host_rsa_key -N '' && ssh-keygen -q -t ecdsa -f /etc/ssh/ssh_host_ecdsa_key -N '' && ssh-keygen -t dsa -f /etc/ssh/ssh_host_ed25519_key  -N '' && sed -i "s/#UsePrivilegeSeparation.*/UsePrivilegeSeparation no/g" /etc/ssh/sshd_config && sed -i "s/UsePAM.*/UsePAM no/g" /etc/ssh/sshd_config && echo 'root:123123' | chpasswd 

# now add jdk,zk,kafka in the container
ADD ./jdk-8u212-linux-x64.tar.gz /usr/local/

# 配置环境变量

ENV JAVA_HOME /usr/local/jdk1.8.0_212
ENV CLASSPATH "$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar"
ENV PATH $JAVA_HOME/bin:$PATH

COPY ./init_machine.sh /root/
COPY ./export_jvm.sh /etc/profile.d/

#ENTRYPOINT sleep 3650d
ENTRYPOINT ["sh","/root/init_machine.sh"]
