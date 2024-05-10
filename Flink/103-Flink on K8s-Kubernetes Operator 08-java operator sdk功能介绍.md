# Flink on Kubernetes - Kubernetes Operator - java operator sdk 功能介绍 


## Reconciler 调节器

## 实现 Reconciler and/or Cleaner  
从 Operator 的角度来看，`Kubernetes 资源的生命周期可以根据 资源是被创建或更新，还是被标记为删除，清楚地分为两个阶段`。                  

框架会自动处理与此相关的逻辑。框架总是会调用 `reconcile()` 方法，除非自定义资源被标记为删除。另一方面，如果资源被标记为删除且 调解器实现了 `Cleaner` 接口，那么会调用 `cleanup()` 方法。实现 `Cleaner` 接口可使开发者能够让 SDK 清理相关状态（例如，集群外的资源）。 因此，SDK 将自动添加与你的 Reconciler 关联的 Finalizer，以便 Kubernetes 服务器在你的 Reconciler 有机会清理之前不会删除您的资源。有关更多详细信息，请参阅 `Finalizer Support`。                     

>### Finalizer Support          
```bash
Kubernetes finalizers 确保在资源标记为删除后，在资源实际被删除之前，你的 Reconciler 有机会执行操作。如果没有终结器，资源将直接被 Kubernetes 服务器删除。                 

根据你的使用情况，你可能需要或不需要使用 finalizers 。特别是，如果你的 operator 不需要清理任何`不会被 Kubernetes 集群自动管理的状态`（例如，外部资源），你可能不需要使用 Finalizer。你应该尽可能使用 Kubernetes 垃圾回收机制，通过为你的 `secondary resources` 设置所有者引用，这样当关联的`primary resource`被删除时，集群就会自动删除它们。需要注意的是，设置所有者引用是 Reconciler 实现的责任，尽管依赖资源使该过程变得更容易。     

如果确实需要清理这种状态，你需要使用 finalizers ，以防止 Kubernetes 集群在你的 operator 准备好允许删除资源之前将其删除。这样，即使`你的 operator 在用户“删除”资源时处于停机状态，清理仍然可以进行`。         

JOSDK 通过在需要时自动处理管理 finalizers ，以这种方式清理资源变得更容易。你唯一需要做的就是让 SDK 知道你 operator 的 `primary resources` 实现 `Cleaner<P>` 接口。如果你的 Reconciler 没有实现 `Cleaner` 接口，SDK 将认为你在资源删除时不需要执行任何清理，并因此不会启用 finalizers support 。换句话说，只有当你的 Reconciler 实现了 `Cleaner` 接口时，才会添加 finalizers support。                 

框架会在创建资源之后，第一次协调之前，自动添加 finalizers 。finalizers 是通过单独的 Kubernetes API 调用添加的。由于这次更新，finalizers 将出现在资源上。然后，reconciliation 可以像往常一样进行。   

在 Reconciler 上执行清理后，自动添加的 finalizers 也将被删除。如上所述，这种行为是可自定义的，当我们讨论使用 DeleteControl 时有解释。     
```

## 使用 UpdateControl and DeleteControl        
这两个类用于`控制 reconciliation 后的结果`或`期望的行为`。   
`UpdateControl` 可以指示框架更新资源的 `status` 子资源，并/或以所需的时间延迟重新安排一次 reconciliation 。          

注意，尽管如此，应该优先使用 `EventSources` 而不是重新调度，因为这样 reconciliation 操作将只会在需要时触发，而不是定期触发。   

以上是资源更新的典型用例，但在某些情况下，controller 可能想要更新资源本身（例如添加注释），或不执行任何更新，这也是支持的。   

也可以使用 `updateResourceAndStatus()` 方法同时更新状态和资源。在这种情况下，资源首先被更新，然后更新状态，使用两个单独的请求与 Kubernetes API 通信。   

你应该始终使用 `UpdateControl` 来声明意图，并让 SDK 处理实际的更新，而不是直接使用 Kubernetes 客户端执行这些更新，这样 SDK 可以相应地更新其内部状态。   

资源更新使用`optimistic version control`进行保护，以确保不会覆盖同时在服务器上发生的其他更新。这是通过在处理的资源上设置 `resourceVersion` 字段来确保的。                     

`DeleteControl` 通常指示框架在清理依赖资源后，在 `cleanup` 实现中移除 finalizers 。               


>### Reconciliation     
```bash
在 Kubernetes 中，**调解**（Reconciliation）是指一种持续、周期性地检查和修复资源状态的过程，以确保实际状态符合用户期望的状态。这个过程通常由自定义控制器或操作员（Operator）实现，最终的目的是让实际的集群状态与声明性配置保持一致。            

调解过程主要涉及以下几点：          
1. **获取当前状态**：控制器从 Kubernetes 集群中获取资源的当前状态（通常是通过观察相应的自定义资源）。                
2. **与期望状态进行比较**：比较当前状态与期望状态之间的差异。
3. **采取行动**：基于差异，采取必要的行动来使实际状态符合期望状态，例如创建、更新或删除资源等。             

在 Kubernetes Operator SDK 中，`Reconciler` 接口的 `reconcile` 方法用于执行这种调解操作。当 Kubernetes API 服务器检测到自定义资源（CR）的变更时，框架会调用 `reconcile` 方法来调整资源状态。            

**例子**：      
- 如果一个自定义控制器负责管理一个应用程序的部署资源，调解器会在检测到自定义资源的期望副本数与当前实际副本数不同步时，调整部署的副本数，使其达到预期的配置。                    
```


## 自动观察生成处理            
最佳实践是在资源状态上设置 `.observedGeneration 值`，以指示 controller 已成功协调的资源的 last generation 。这有助于用户/管理员诊断潜在问题。           

为了使此功能生效：          
- status class（而不是资源本身）必须实现 `ObservedGenerationAware` 接口。也可以参考 `ObservedGenerationAwareStatus` 便捷实现，在自己的状态类实现中可以进行扩展。          
- 另一个条件是 `CustomResource.getStatus()` 方法不应返回 null。因此，在使用 `UpdateControl` 返回对象时，应实例化状态。      

如果满足这些条件并且生成感知已激活，则在调用 reconcile() 方法后，框架会自动设置观察到的生成。请注意，即使从 reconciler 返回 `UpdateControl.noUpdate()`，观察到的生成也会更新。在 WebPage 示例中可以看到此功能的运作方式。           

可以通过重写 `CustomResource` 的 `initStatus()` 方法来自动初始化自定义资源的状态。但是，不建议这样做，因为如果你使用 `UpdateControl.patchStatus` ，则会破坏状态修补。另请参见 javadocs 。     

如果满足这些条件并且 generation awareness is activated ，则在调用`reconcile()`方法后框架会自动设置观察到的生成。请注意，即使从 reconciler 返回 UpdateControl.noUpdate()，观察到的生成也会更新。See this feature at work in the WebPage example 。    


## Support for Well Known (non-custom) Kubernetes Resources   
Controller 可以注册非自定义资源，例如众所周知的 Kubernetes 资源，例如（入口、部署等）。请注意，这些资源不支持自动观察生成处理，但在这种情况下，观察生成的处理可能由主控制器处理。
    

## Max Interval Between Reconciliations     
当通知者/事件源设置正确并且协调器实现正确时，不需要额外的协调触发器。然而，通常的做法是设置故障安全定期触发器，只是为了确保资源在一定时间后得到协调。默认情况下，此功能已到位，具有相当长的时间间隔（当前为 10 小时），之后即使没有其他事件，也会自动触发对帐。了解如何使用标准注释来覆盖它     

该事件不是以固定速率传播，而是在每次协调之后安排。所以下一次对账最多会在上次对账后指定的时间间隔内发生。      

可以通过将 maxReconciliationInterval 设置为 Constants.NO_MAX_RECONCILIATION_INTERVAL 或任何非正数来关闭此功能。 

自动重试不受此功能的影响，因此无论此最大间隔设置如何，都会根据指定的重试策略在出错时重新触发协调。      

## Automatic Retries on Error   
每当你的 Reconciler 抛出异常时，JOSDK 将安排自动重试协调。重试行为是可配置的，但提供了涵盖大多数典型用例的默认实现，请参阅 GenericRetry 。    

您还可以使用 @GradualRetry 注释配置默认重试行为。     

可以使用 @ControllerConfiguration 注释的重试字段并指定自定义实现的类来提供自定义实现。请注意，此类需要提供一个可访问的无参数构造函数以进行自动实例化。此外，您的实现可以通过您可以提供的注释自动配置，方法是让您的 Retry 实现实现 AnnotationConfigurable 接口，并使用您的注释类型进行参数化。有关更多详细信息，请参阅 GenericRetry 实现。     

有关当前重试状态的信息可以从 Context 对象访问。值得注意的是，特别有趣的是 isLastAttempt 方法，它可以允许您的协调器根据此状态实现不同的行为，方法是在资源状态中设置错误消息，例如，在尝试最后一次重试时。      

但请注意，达到重试限制不会阻止新事件的处理。像往常一样，新事件将进行新的对账。但是，如果还发生通常会触发重试的错误，则 SDK 此时不会安排重试，因为已达到重试限制。   

成功执行会重置重试状态。

## 上次重试后设置错误状态
为了方便错误报告，Reconciler可以实现ErrorStatusHandler接口：    

如果协调器抛出异常，则会调用 updateErrorStatus 方法。即使没有配置重试策略，也会在协调器执行后立即调用它。第一次协调尝试后，RetryInfo.getAttemptCount() 为零，因为它不是重试的结果（无论是否配置了重试策略）。     

ErrorStatusUpdateControl 用于告诉 SDK 要做什么以及如何对主资源执行状态更新，始终作为状态子资源请求执行。请注意，此更新请求还将生成一个事件，并且如果控制器不支持生成，则将导致协调。      

此功能仅适用于 Reconciler 接口的协调方法，因为不应更新已标记为删除的资源。    

如果发生不可恢复的错误，可以跳过重试：        

### Correctness and Automatic Retries   


## Handling Related Events with Event Sources   
Event sources 是一个相对简单但功能强大且可扩展的概念，用于触发 controller 执行，通常基于对依赖资源的更改。当 secondary resources 发生可能影响 primary resource 状态的情况时，如果您希望触发 Reconciler ，则通常需要 event source 。这是必需的，因为给定的 Reconciler 默认情况下只会侦听影响其配置的 primary resource 类型的事件。Event sources 充当影响这些 secondary resources 的事件的监听，以便在需要时可以触发关联的 primary resource 的协调。请注意，这些 secondary resources 不一定是 Kubernetes 资源。通常，在处理非 Kubernetes 对象或服务时，我们可以扩展操作符来处理 Webhook 或 Websocket，或者对来自我们交互的服务的任何事件做出反应。这允许非常高效的 controller 实现，因为只有当资源上发生影响我们的 primary resource 的事情时才会触发协调，从而消除了定期重新安排协调的需要。   

这里有几个有趣的点：      
CustomResourceEventSource event source 是一个特殊的事件源，负责处理与影响我们的 primary resources 的更改相关的事件。 SDK 始终自动为每个 controller 注册此 EventSource。`请务必注意，事件始终与给定的 primary resource 相关`。即使存在 EventSource 实现，仍然会为您处理并发性，并且 SDK 仍然保证任何给定 primary resource 的 controller 不会并发执行（当然，与其他主资源相关的事件的并发/并行执行）资源仍然按预期发生）。        

## Managing Relation between Primary and Secondary Resources    
`Event sources 让你的 operator 知道 secondary resource 何时发生更改，并且你的 operator 可能需要协调此新信息`。然而，为了做到这一点，SDK 需要以某种方式检索与触发事件的 secondary resource 关联的 primary resource 。在上面的 Tomcat 示例中，当跟踪的 Deployment 上发生事件时，SDK 需要能够识别哪个 Tomcat 资源受到该更改的影响。        

经验丰富的 Kubernetes 用户已经知道一种跟踪这种父子关系的方法：`using owner references`。事实上，这也是 SDK 默认处理这种情况的方式，也就是说，如果您的控制器在 primary resource 上正确设置了所有者引用，那么 SDK 将能够自动跟踪该引用回到您的主资源，而无需您担心关于它。    

但是，owner references 并不总是可以使用，因为它们仅限于在单个命名空间内运行（即，你不能拥有对不同命名空间中的资源的所有者引用），并且本质上仅限于 Kubernetes 资源，因此你无法使用，如果你的secondary resources 位于集群之外，那就幸运了。       

这就是为什么 JOSDK 提供 SecondayToPrimaryMapper 接口，以便你可以为 SDK 提供替代方法来识别当 secondary resources 发生问题时需要协调哪个 primary resource 。我们甚至在 Mappers 类中提供了一些替代方案。       

请注意，虽然返回一组 ResourceID，但该组通常仅包含一个元素。然而，可以返回多个值，甚至根本不返回任何值，以覆盖一些罕见的极端情况。返回空集意味着映射器认为 secondary resource 事件不相关，因此在这种情况下 SDK 将不会触发 primary resource 的协调。      

当 primary resource 和 secondary resource 之间存在一对多关系时，添加 secondaryToPrimaryMapper 通常就足够了。secondary resource 可以映射到其主要所有者，这足以从传递到协调器的 Context 对象中获取这些 secondary resources。      

然而，在某些情况下，这还不够，您需要使用 PrimaryToSecondaryMapper 接口的实现来提供 primary resource 与其关联的 secondary resource 之间的显式映射。当 primary resource 和 secondary resource 之间存在多对一或多对多关系时，通常需要这样做，例如当 primary resource 引用 secondary resource 时。请参阅 PrimaryToSecondaryIT 集成测试示例。        





refer       
1.https://javaoperatorsdk.io/docs/getting-started                        
