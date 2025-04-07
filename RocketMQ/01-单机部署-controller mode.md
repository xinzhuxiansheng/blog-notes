# Rocket 单机部署 - Controller Mode 部署    



---------------------------------------------------------------------- namesrv

# 使用 5.3.2 版本安装包
unzip rocketmq-all-5.3.2-bin-release.zip -d /rocketmq/namesrv

# 精简配置（5.x 版本自动识别主目录）
cat > /rocketmq/namesrv/conf/namesrv.conf <<EOF
listenPort=9876
enableControllerInNamesrv=false  # 5.x 新增参数，关闭 Controller 混合模式
EOF

>Windows 打包 RocketMQ 源码后，需要使用 dos2unix 转格式   

# 启动命令
nohup sh bin/mqnamesrv -c namesrv.conf &


---------------------------------------------------------------------- controller


unzip rocketmq-all-5.3.2-bin-release.zip -d /rocketmq/controller

# 关键配置 controller.conf
cat > /rocketmq/controller/conf/controller.conf <<EOF
controllerClusterName=RocketMQCluster  # 5.x 必须配置集群名称
controllerDLegerGroup=ctrl-group      # DLedger 组标识
controllerDLegerPeers=n0-127.0.0.1:9878  # 单节点部署配置
controllerStorePath=/rocketmq/controller/store
EOF

# 启动 Controller（新增 -a 参数指定角色）
nohup sh /bin/mqcontroller -c controller.conf &


controllerType = jRaft
enableElectUncleanMaster = false
notifyBrokerRoleChanged = true
controllerStorePath = /root/rocketmq/rocketmq-5.3.2/data/jRaftController
jRaftGroupId = jRaft-Controller
jRaftServerId = 127.0.0.1:9880
jRaftInitConf = 127.0.0.1:9880
jRaftControllerRPCAddr = 127.0.0.1:9770
jRaftSnapshotIntervalSecs = 3600


nohup sh bin/mqcontroller -c controller.conf &


---------------------------------------------------------------------- broker 

unzip rocketmq-all-5.3.2-bin-release.zip -d /rocketmq/broker

# broker.conf 关键配置
cat > /rocketmq/broker/conf/broker.conf <<EOF
brokerClusterName=RocketMQCluster  # 必须与 Controller 集群名一致
controllerAddr=127.0.0.1:9878      # 指向 Controller 端口
enableControllerMode=true          # 启用 Controller 模式
brokerName=BrokerA
listenPort=10911
storePathRootDir=/rocketmq/broker/store
EOF

# 启动命令（自动注册到 Controller）
nohup sh bin/mqbroker -c broker.conf &
