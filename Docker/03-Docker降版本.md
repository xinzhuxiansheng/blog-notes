## Docker 降版本    

yum list docker-ce --showduplicates | sort -r


yum downgrade --setopt=obsoletes=0 -y docker-ce-20.10.9-3.el7 docker-ce-cli-20.10.9-3.el7 containerd.io


systemctl start docker
docker version
