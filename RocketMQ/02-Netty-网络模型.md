# RocketMQ - Netty 网络模型  

## NettyRemotingServer



public interface RemotingService {
    void start();

    void shutdown();

    void registerRPCHook(RPCHook rpcHook);

    void setRequestPipeline(RequestPipeline pipeline);

    /**
     * Remove all rpc hooks.
     */
    void clearRPCHook();
}




RocketMQ提供两种解码方式：分别是 JSON、RocketMQ。默认使用JSON序列化。  
```java
org.apache.rocketmq.remoting.protocol.RemotingCommand#headerDecode()
```

RequestCode.java -> 对应不同请求的 Code