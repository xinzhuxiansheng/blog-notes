# Rocky Linux - 替换归档后的 YUM 数据源  


sed -i -e 's/^\s*mirrorlist=http/#mirrorlist=http/g' -e 's/^#\s*baseurl=http/baseurl=http/g' /etc/yum.repos.d/rocky*.repo

