

## 1. 介绍
本文档是为希望利用ZooKeeper协调服务的分布式应用程序的开发人员提供的指南。它包含概念和使用信息。 
本指南的前四部分对各种ZooKeeper概念进行了更高层次的讨论。这些对于理解ZooKeeper如何工作以及如何使用它都是必要的。        
>第一组的部分是：
ZooKeeper Data Model
ZooKeeper Sessions
Zookeeper Watches
Consistency Guarantees(一致性保证)





## ZooKeeper Data Model
ZooKeeper有个分层的命名空间，很像一个分布式文件系统。唯一的区别是命名空间中的每个节点都可以拥有与其关联的数据以及子节点。这就像拥有一个允许文件也成为目录的文件系统。节点的路径总是表示为规范、绝对、斜线分隔的路径。任何unicode字符都可以受以下约束的路径中使用：  
* 空字符(\u0000)不能是路径名的一部分
* 不能使用以下字符，因为它们不能很好地显示或以令人困惑的方式呈现: \u0001 - \u001F和\u007F、\u009F
* 不允许使用以下字符： \ud800 - uF8FF、\uFFFO - uFFFF
* "."字符可以用作名称的一部分，但".",".."不能单独用于指示路径的节点，因为ZooKeeper不使用相对路径。以下内容无效: "/a/b/./c"或"/a/b/../c"
* /zookeeper 路径是保留的

### ZNodes
ZooKeeper 树中的每个节点都成为znode， znodes维护了一个stat结构，其中包括数据更改、acl更改的版本号。stat结构也有时间戳。版本号与时间戳一起允许Zookeeper验证缓存并协调更新。每次znode的数据更改时，版本号都会增加。例如，每当客户端检索数据时，它也会收到数据的版本。并且当客户端执行更新或删除时，它必须维提供它正在更改的znode数据的版本。如果它提供的版本与数据的实际版本不匹配，则更新将失败。    

#### Watches
客户端可以在znode上设置监视，对该znode的更改会触发监视。当watch触发时，ZooKeeper会向客户端发送通知。    

ZooKeeper 中的所有读取操作 - getData()、getChildren()和exists() - 都可以选择将监视设置为副作用。    

以下是 ZooKeeper 对 watch 的定义：watch 事件是一次性触发器，发送到设置 watch 的客户端，当设置 watch 的数据发生变化时发生。在watch的定义中，需要考虑三个关键点： 

* 一次性触发(One-time trigger)        
当数据发生变化时，会向客户端发送一个 watch 事件。例如，如果客户端执行 getData("/znode1", true) 并且稍后更改或删除了 /znode1 的数据，则客户端将获得 /znode1 的监视事件。如果 /znode1 再次更改，则不会发送监视事件，除非客户端进行了另一次读取以设置新监视。  

* 发送到客户端(Sent to the client)  
这意味着事件正在到达客户端的途中，但在更改操作的成功返回码到达发起更改的客户端之前可能无法到达客户端。Watches 异步发送给观察者。ZooKeeper 提供了排序保证：客户端永远不会看到它设置了监视的更改，直到它第一次看到监视事件。网络延迟或其他因素可能会导致不同的客户端在不同时间查看更新并返回代码。关键是不同客户端看到的所有东西都会有一个一致的顺序。  

* watch设置的数据(The data for which the watch was set) 
这是指节点可以改变的不同方式。将 ZooKeeper 视为维护两个监视列表会有所帮助：数据监视和子监视。getData() 和 exists() 设置data watches。getChildren() 设置 child watches。或者，考虑根据返回的数据类型设置watches可能会有所帮助。getData() 和 exists() 返回有关节点数据的信息，而 getChildren() 返回子节点列表。因此， setData() 将触发正在设置的 znode 的数据监视（假设设置成功）。成功的 create() 将触发正在创建的 znode 的data watch和父 znode 的child watch。成功的 delete() 将同时触发data watch和child watch（因为不能再有子节点）被删除的 znode 以及父 znode 的child watch。    

Watches 在客户端连接到的 ZooKeeper 服务器上本地维护。这允许watches在设置、维护和调度方面是轻量级的。当客户端连接到新服务器时，任何会话事件都会触发监视。与服务器断开连接时将不会收到Watches。当客户端重新连接时，如果需要，任何先前注册的watches将被重新注册和触发。一般来说，这一切都是透明的。有一种情况可能会丢失监视：如果在断开连接的情况下创建和删除 znode，则会丢失尚未创建的 znode 存在的监视。

>3.6.0 中的新功能：客户端还可以在 znode 上设置永久的递归监​​视，这些监视在触发时不会被删除，并且会以递归方式触发注册的 znode 以及任何子 znode 上的更改。 

**Watches的语义**   
我们可以使用三个读取 ZooKeeper 状态的调用来设置监视：exists、getData 和 getChildren。以下列表详细说明了监视可以触发的事件以及启用它们的调用：

* Created event：通过调用exists 启用。
* Deleted event：通过调用exists、getData 和getChildren 启用。
* Changed event：通过调用exists 和getData 启用。
* Child event：通过调用 getChildren 启用。

**持久的、递归Watches**
3.6.0 中的新功能：现在在上述标准watch上有一个变体，你可以设置一个在触发时不会被移除的手表。此外，这些监视会触发事件类型NodeCreated、NodeDeleted和NodeDataChanged ，并且可以选择递归地针对所有从监视注册的 znode 开始的 znode。`请注意，NodeChildrenChanged事件不会为持久递归观察触发，因为它是多余的`。

使用addWatch()方法设置持久监视。触发语义和保证（一次性触发除外）与标准手表相同。关于事件的唯一例外是递归持久观察者永远不会触发子更改事件，因为它们是多余的。使用具有观察者类型`WatcherType.Any 的removeWatches()`移除持久观察。   

**移除Watches**    
我们可以通过调用 removeWatches 来删除在 znode 上注册的watches。此外，即使没有服务器连接，ZooKeeper 客户端也可以通过将本地标志设置为 true 来删除本地监视。以下列表详细说明了成功移除手表后将触发的事件。

* Child Remove event：通过调用 getChildren 添加的 Watcher。
* Data Remove event：通过调用exists或getData添加的观察者。
* Persistent Remove event：通过调用添加持久watch来添加的 Watcher。

**ZooKeeper针对Watches的Guarantees**     
关于watches，ZooKeeper维护这些Guarantees：

* Watches是根据其他事件、其他watches和异步回复进行排序的。ZooKeeper 客户端库确保按顺序分派所有内容。
* 在看到对应于该 znode 的新数据之前，客户端将看到它正在监视的 znode 的监视事件。
* 来自 ZooKeeper 的观察事件的顺序与 ZooKeeper 服务看到的更新顺序相对应。

**关于Watches的注意事项**       
* Standard watches是一次性触发器；如果你收到一个 watch 事件，并且希望收到有关未来更改的通知，则必须设置另一个 watch。   

* 因为Stndard watches是一次性触发器，并且在获取事件和发送新请求以获取watch之间存在延迟，所以你无法可靠地看到 ZooKeeper 中节点发生的每一个变化。准备好处理 znode 在获取事件和再次设置 watch 之间多次更改的情况。（你可能不在乎，但至少意识到它可能会发生。）

* 一个watch对象，或function/context pair，对于给定的通知只会被触发一次。例如，如果为同一个文件注册了同一个监视对象，并且为同一个文件调用了 getData，然后删除了该文件，则该监视对象将仅被调用一次，并带有该文件的删除通知。

* 当你与服务器断开连接时（例如，当服务器出现故障时），你将无法获得任何监视，直到重新建立连接。出于这个原因，会话事件被发送到所有未完成的监视处理程序。使用会话事件进入安全模式：断开连接时你将不会接收事件，因此你的进程应该在该模式下谨慎行事。


## ZooKeeper的ACL访问控制
ZooKeeper 使用 ACL 来控制对其 znode（ZooKeeper 数据树的数据节点）的访问。ACL 实现与 UNIX 文件访问权限非常相似：它使用权限位来允许/禁止针对节点的各种操作以及这些位适用的范围。与标准 UNIX 权限不同，ZooKeeper 节点不受用户（文件所有者）、group和world（其他）三个标准范围的限制。ZooKeeper 没有 znode 所有者的概念。相反，ACL 指定了与这些 ids 相关联的 ids 和权限集。

另请注意，ACL 仅适用于特定的 znode。尤其不适用于children。例如，如果/app只能被 ip:172.16.16.1 读取，而/app/status是全局可读的，那么任何人都可以读取/app/status；ACL 不是递归的。

ZooKeeper 支持可插入的身份验证方案。Ids 使用形式scheme:expression指定，其中scheme是 id 对应的身份验证方案。有效表达式集由方案定义。例如，ip:172.16.16.1是使用ip scheme地址为172.16.16.1的主机的 ID ，而digest:bob:password是使用摘要方案的名称为bob的用户的 ID 。

当客户端连接到 ZooKeeper 并对其进行身份验证时，ZooKeeper 将与客户端对应的所有 id 与客户端连接相关联。当客户端尝试访问节点时，会根据 znode 的 ACL 检查这些 id。ACL 由成对的(scheme:expression, perms) 组成。表达式的格式特定于方案。例如，对(ip:19.22.0.0/16, READ)向IP 地址以 19.22 开头的任何客户端授予READ权限

### ACL权限     
ZooKeeper 支持以下权限：

* CREATE：你可以创建一个子节点
* READ：你可以从节点获取数据并列出其子节点
* WRITE：你可以为节点设置数据
* DELETE: 可以删除子节点
* ADMIN：你可以设置权限     

在CREATE和DELETE权限已被打破了，只单独写的细粒度的访问控制权限。针对CREATE和DELETE有以下几方面：

你希望 A 能够在 ZooKeeper 节点上执行设置，但不能创建或删除子节点。
`CREATE without DELETE`：客户端通过在父目录中创建 ZooKeeper 节点来创建请求。你希望所有客户端都可以添加，但只有请求处理器可以删除。（这有点像文件的 APPEND 权限。）
此外，由于 ZooKeeper 没有文件所有者的概念，因此存在ADMIN权限。在某种意义上，ADMIN权限将实体指定为所有者。ZooKeeper 不支持 LOOKUP 权限（对目录执行权限位以允许你 LOOKUP，即使你无法列出目录）。每个人都隐含地拥有 LOOKUP 权限。这允许你统计一个节点，但仅此而已。（问题是，如果你想在一个不存在的节点上调用 zoo_exists() ，没有权限检查。）
ADMIN权限在 ACL 方面也有特殊作用：为了检索 znode 用户的 ACL，必须具有READ或ADMIN权限，但没有ADMIN权限，摘要哈希值将被屏蔽。

**内置ACL方案** 
ZooKeeper 具有以下内置方案：

* `world`有一个 id，任何人，代表任何人。
* `auth`是一种特殊方案，它忽略任何提供的表达式，而是使用当前用户、凭据和方案。在保留 ACL 时，ZooKeeper 服务器将忽略提供的任何表达式（无论是用户喜欢 SASL 身份验证还是user:password喜欢 DIGEST 身份验证）。但是，表达式仍必须在 ACL 中提供，因为 ACL 必须与格式scheme:expression:perms匹配。提供此方案是为了方便，因为它是用户创建 znode 并将对该 znode 的访问权限仅限于该用户的常见用例。如果没有经过身份验证的用户，则使用 auth 方案设置 ACL 将失败。
* `digest`使用用户名：密码字符串生成 MD5 哈希，然后将其用作 ACL ID 身份。身份验证是通过以明文形式发送用户名：密码来完成的。在 ACL 中使用时，表达式将是username:base64编码的SHA1密码摘要。
* `ip`使用客户端主机 IP 作为 ACL ID 身份。的ACL表达的形式的地址/位，其中最显著位的地址是针对最显著匹配位的客户端主机的IP。
* `x509`使用客户端 X500 Principal 作为 ACL ID 身份。ACL 表达式是客户端的准确 X500 主体名称。使用安全端口时，客户端会自动进行身份验证并设置其 x509 方案的身份验证信息。

ZooKeeper C 客户端 API
ZooKeeper C 库提供了以下常量：

* const int ZOO_PERM_READ; //可以读取节点的值并列出其子节点
* const int ZOO_PERM_WRITE;// 可以设置节点的值
* const int ZOO_PERM_CREATE; //可以创建孩子
* const int ZOO_PERM_DELETE;// 可以删除孩子
* const int ZOO_PERM_ADMIN; //可以执行set_acl()
* const int ZOO_PERM_ALL;// 以上所有标志或运算在一起    

以下是标准的 ACL ID：

* struct Id ZOO_ANYONE_ID_UNSAFE; //('world','anyone')
* struct Id ZOO_AUTH_IDS;// ('auth','') 
ZOO_AUTH_IDS 空身份字符串应解释为“创建者的身份”。

**ZooKeeper 客户端带有三个标准 ACL：**

* struct ACL_vector ZOO_OPEN_ACL_UNSAFE; //(ZOO_PERM_ALL,ZOO_ANYONE_ID_UNSAFE)
* struct ACL_vector ZOO_READ_ACL_UNSAFE;// (ZOO_PERM_READ, ZOO_ANYONE_ID_UNSAFE)
* struct ACL_vector ZOO_CREATOR_ALL_ACL; //(ZOO_PERM_ALL,ZOO_AUTH_IDS)  
ZOO_OPEN_ACL_UNSAFE 对所有 ACL 完全免费开放：任何应用程序都可以在节点上执行任何操作，并且可以创建、列出和删除其子节点。ZOO_READ_ACL_UNSAFE 是任何应用程序的只读访问权限。CREATE_ALL_ACL 将所有权限授予节点的创建者。创建者必须已经通过服务器的身份验证（例如，使用“摘要”方案）才能使用此 ACL 创建节点。

以下 ZooKeeper 操作处理 ACL：

* int zoo_add_auth (zhhandle_t *zh, const char * scheme, const char * cert, int certLen, void_completion_t completion, const void *data);
应用程序使用 zoo_add_auth 函数向服务器验证自身。如果应用程序想要使用不同的方案和/或身份进行身份验证，则可以多次调用该函数。

* int zoo_create (zhhandle_t *zh, const char *path, const char *value, int valuelen, const struct ACL_vector *acl, int flags, char *realpath, int max_realpath_len);
zoo_create(...) 操作创建一个新节点。acl 参数是与节点关联的 ACL 列表。父节点必须设置 CREATE 权限位。

* int zoo_get_acl (zhandle_t *zh, const char *path, struct ACL_vector *acl, struct Stat *stat);
此操作返回节点的 ACL 信息。该节点必须具有 READ 或 ADMIN 权限集。没有管理员权限，摘要哈希值将被屏蔽。

* int zoo_set_acl (zhandle_t *zh, const char *path, int version, const struct ACL_vector *acl);
此函数用新的列表替换节点的 ACL 列表。该节点必须具有 ADMIN 权限集。

这是一个示例代码，它利用上述 API 使用“ foo ”方案进行身份验证，并创建一个具有仅创建权限的临时节点“/ xyz”。



## 可插入的ZooKeeper身份验证
ZooKeeper 运行在各种不同的环境中，具有各种不同的身份验证方案，因此它具有完全可插拔的身份验证框架。甚至内置的身份验证方案也使用可插入的身份验证框架。

要了解身份验证框架的工作原理，首先必须了解两个主要的身份验证操作。框架首先必须对客户端进行身份验证。这通常在客户端连接到服务器后立即完成，包括验证从客户端发送或收集的有关客户端的信息并将其与连接相关联。框架处理的第二个操作是在 ACL 中查找对应于客户端的条目。`ACL entries是 < idspec, permissions > 对`。该*idspec*可以是与连接关联的身份验证信息匹配的简单字符串，也可以是针对该信息进行评估的表达式。由身份验证插件的实现来进行匹配。这是身份验证插件必须实现的接口：
```java
public interface AuthenticationProvider {
    String getScheme();
    KeeperException.Code handleAuthentication(ServerCnxn cnxn, byte authData[]);
    boolean isValid(String id);
    boolean matches(String id, String aclExpr);
    boolean isAuthenticated();
}
```
第一个方法`getScheme()`返回标识插件的字符串。由于我们支持多种身份验证方法，因此身份验证凭据或idspec将始终以scheme:为前缀。ZooKeeper 服务器使用身份验证插件返回的方案来确定该方案适用于哪些 id。

当客户端发送与连接相关联的身份验证信息时，将调用`handleAuthentication()`。客户端指定信息对应的scheme。ZooKeeper 服务器将信息传递给身份验证插件，其getScheme与客户端传递的方案相匹配。handleAuthentication的实现者通常会在确定信息错误时返回错误，或者使用cnxn.getAuthInfo().add(new Id(getScheme(), data))将信息与连接相关联。

身份验证插件涉及设置和使用 ACL。当为 znode 设置 ACL 时，ZooKeeper服务器会将entry的 id 部分传递给isValid(String id)方法。由插件来验证 id 是否具有正确的形式。例如，ip:172.16.0.0/16是一个有效的 id，但ip:host.com不是。如果新 ACL 包含“auth”entry，则使用isAuthenticated来查看是否应将与连接关联的此方案的身份验证信息添加到 ACL。某些方案不应包含在 auth. 例如，如果指定了auth，则客户端的IP地址不被视为应添加到ACL的id。

ZooKeeper在检查 ACL 时调用matches(String id, String aclExpr)。它需要将客户端的认证信息与相关的 ACL entries进行匹配。为了找到适用于客户端的entries，ZooKeeper服务器将找到每个entry的scheme，如果该方案有来自该客户端的身份验证信息，则将调用matches(String id, String aclExpr)并将id设置为身份验证先前由handleAuthentication添加到连接的信息，并将aclExpr设置为 ACL 条目的 id。身份验证插件使用自己的逻辑和匹配方案来确定id是否包含在aclExpr中。

有两个内置的身份验证插件：ip和digest。可以使用系统属性添加其他插件。在启动时，ZooKeeper 服务器将查找以“zookeeper.authProvider”开头的系统属性。并将这些属性的值解释为身份验证插件的类名。可以使用-Dzookeeeper.authProvider.X=com.f.MyAuth或在服务器配置文件中添加如下条目来设置这些属性：
``` 
authProvider.1=com.f.MyAuth
authProvider.2=com.f.MyAuth2
```
应注意确保属性上的后缀是唯一的。如果存在重复项，例如-Dzookeeeper.authProvider.X=com.f.MyAuth -Dzookeeper.authProvider.X=com.f.MyAuth2，则只会使用一个。此外，所有服务器都必须定义相同的插件，否则使用插件提供的身份验证方案的客户端将无法连接到某些服务器。

>在 3.6.0 中添加：替代抽象可用于可插拔身份验证。它提供了额外的参数。    

```java
public abstract class ServerAuthenticationProvider implements AuthenticationProvider {
    public abstract KeeperException.Code handleAuthentication(ServerObjs serverObjs, byte authData[]);
    public abstract boolean matches(ServerObjs serverObjs, MatchValues matchValues);
}
```
不是实现 AuthenticationProvider，而是扩展 ServerAuthenticationProvider。然后您的 handleAuthentication() 和matches() 方法将接收额外的参数（通过ServerObjs 和MatchValues）。

* `ZooKeeperServer`       ZooKeeperServer 实例
* `ServerCnxn`            当前连接
* `path`                  正在操作的 ZNode 路径（如果未使用，则为 null）
* `perm`                  操作值或 0
* `setAcls`               当 setAcl() 方法被操作时，正在设置的 ACL 列表







## 配置参数
ZooKeeper中的配置文件zoo.cfg中参数含义解读如下:
* tickTime = 2000: 通信心跳时间，ZooKeeper服务器与客户端心跳时间，单位毫秒
* initLimit = 10: LF初始通信时限
Leader和Follower初始连接时能容忍的最多心跳数
* syncLimit = 5: LF同步通信时限
Leader和Follower之间通信时间如果超过syncLimit*tickTime,Leader认为Follower死掉，从服务器列表中删除Follower   
* dataDir: 保存ZooKeeper中的数据
* clientPort = 2181： 客户端连接端口


## 节点信息


## 
顺序对 ZooKeeper 非常重要；几乎接近强迫症。所有更新都是完全有序的。ZooKeeper 实际上用反映此顺序的数字标记每个更新。我们称这个数字为 zxid（ZooKeeper 事务 ID）。每次更新都会有一个唯一的 zxid。读取（和观察）按更新顺序排列。读取响应将被标记为服务读取的服务器处理的最后一个 zxid


>参考链接: https://colobu.com/2014/12/15/zookeeper-recipes-by-example-5/

## Path Cache

## Node Cache

## Tree Node