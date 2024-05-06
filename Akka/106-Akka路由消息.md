## Akka路由消息

>Akka version: 2.6.19

### 使用AKka Router实现负载均衡 

`Akka中可用的Router列表`      
| 逻辑      |    池 | 群组  |  说明  |
| :-------- | --------:| :--: |  ----- |
| RoundRobinRouting-Logic  | RoundRobin-Pool |  RoundRobinGroup   | 此逻辑把先收到的消息发送给第一个routee，再收到的消息发送给第二个routee，依次类推。当所有routee都收到消息，第一个routee再接收下一个，然后继续     |
| RandomRouting-Logic | RandomPool |  RandomGroup  |   这个逻辑把每个收到的消息发送给随机选定的routee   |
| SmallestMailbox-RoutingLogic  | SmallestMail-boxPool  | 无  |  Router检查所有routee的邮箱，选择邮箱最小的routee。群组版本的不可用，因为它内部使用选择Actor的功能，用这些引用不能获取邮箱的大小    | 
| 无  | BalancingPool  | 无  |  这个Router把消息分发给空闲的routee。这是它的内部实现，与其他Router不同。对于所有的routee使用一个邮箱。该Router使用routee的特殊分发器实现这一逻辑。这也是只有池Router可用的原因    | 
| BroadcastRouting-Logic  | BroadcastPool  | BroadcastGroup  |  把收到的消息发送到所有的routee。这与企业集成模式中定义的 Router不同，它只是实现了接收者列表    | 
| ScatterGather-FirstComplete-dRoutingLogic  | ScatterGather-FirstCompleted-Pool  | ScatterGather-First-Completed-Group  |  这个Router把消息发送到所有的routee，并把第一个响应发送给原发送者。从技术上讲，这是一个使用竞争任务的分发-收集模式，也就是选择最佳的结果，在本例中是最快的响应    | 
| ConsistentHas-hing-RoutingLogic  | Consistent-HashingPool  | Consistent-Hashing-Group  |  该Router使用消息的一致性散列选择routee。这用于把不同的消息路由到同一 routee，但到底是哪个routee并没有关系   | 





refer 
1.《Akka实战》  
