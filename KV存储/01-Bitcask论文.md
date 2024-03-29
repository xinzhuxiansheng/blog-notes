## Bitcask论文（A Log-Structured Hash Table for Fast Key/Value Data）  

>论文出处 https://riak.com/assets/bitcask-intro.pdf 

该篇主要是“翻译”          

Bitcask的起源与Riak分布式数据库的历史有关。在Riak键/值集群中，每个节点使用可插拔的本地存储;几乎任何k/v形状的东西都可以用作每台主机的存储引擎。这种可插拔性允许Riak的进程并行化，这样就可以在不影响其余代码库的情况下改进和测试存储引擎。许多这样的本地键/值存储已经存在，包括但不限于Berkeley DB, Tokyo Cabinet和Innostore。
>Risk公司官网 https://riak.com/index.html       

![bitcask01](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask01.png)  

在评估这种存储引擎时，我们寻求的目标有很多，包括:   

* 每项读取或写入的低延迟    
* 高吞吐量，特别是在写入随机项的传入流      
* 处理比RAM大得多的数据集的能力 
* 崩溃友好，无论是在快速恢复和不丢失数据    
* 易于备份和恢复    
* 相对简单、可理解(因而可支持)的代码结构和数据格式  
* 在高访问负载或大容量下的可预测行为    
* 允许在Riak中轻松默认使用的许可    
 
实现其中一些目标很容易。实现所有这些目标就不那么容易了。    

没有一个可用的本地`key/value`存储系统(包括但不限于作者编写的那些)对于上述所有目标都是理想的。我们曾与Eric Brewer讨论过这个问题，`当时他对哈希表日志合并有一个重要的见解:这样做可能会比LSM树更快`。  

这使我们以一种新的视角来探索日志结构文件系统中使用的一些技术，这些技术最初是在20世纪80年代和90年代开发的。这种探索推动了bitcask的开发，这是一个非常符合上述所有目标的存储系统。虽然Bitcask最初是为了在Riak下使用而开发的，但它是通用的，也可以作为其他应用程序的本地 key/value 存储。   

我们最终采用的模型在概念上非常简单。Bitcask实例是一个目录，我们强制只有一个操作系统进程在给定时间打开该Bitcask进行写操作。你可以将这些进程视为“数据库服务”。在任何时候，该目录中都有一个文件处于“活动（active）”状态可供服务写入。当该文件达到大小阈值时，它将被关闭，并创建一个新的活动文件。一旦文件被关闭，无论是故意关闭还是服务退出，它都被认为是不可变的，并且永远不会再打开以进行写入。  

![bitcask02](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask02.png)      

`Active`文件仅通过追加写入，这意味着顺序写入不需要磁盘查找。每个key/value数据格式很简单:    
![bitcask03](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask03.png)      

每次写入时，都会向活动文件添加一个新条目。请注意，删除只是写入一个特殊的墓碑值，它将在下一次合并时被删除。因此，Bitcask数据文件只不过是这些条目的线性序列:  
![bitcask04](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask04.png)      

追加操作完成后，一个名为“keydir”的内存结构被更新。keydir是一个简单的哈希表，它将Bitcask中的每个键映射到一个固定大小的结构，给出该键的文件、偏移量和最近写入条目的大小。 
![bitcask05](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask05.png)      

当发生写操作时，keydir会自动更新为最新数据的位置。旧的数据仍然存在于磁盘上，但是任何新的读取都将使用keydir中可用的最新版本。正如我们稍后将看到的，合并过程最终将删除旧值。  

读取一个值很简单，而且只需要一次磁盘寻道。我们在keydir中查找键，然后使用查找返回的文件id、位置和大小从那里读取数据。在许多情况下，操作系统的文件系统预读缓存使得这个操作比预期的要快得多。      
![bitcask06](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask06.png)  

随着时间的推移，这个简单的模型可能会占用大量的空间，因为我们只是写出新的值，而不涉及旧的值。我们称之为“合并”的压缩过程解决了这个问题。合并过程迭代一个Bitcask中所有非活动(即不可变)的文件，并生成一组数据文件作为输出，其中仅包含每个当前key的“实时”或最新版本。 

完成后，我们还在每个数据文件旁边创建一个“提示文件”。这些本质上类似于数据文件，但它们包含相应数据文件中值的位置和大小，而不是值。
![bitcask07](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask07.png)  

当一个Bitcask被一个Erlang进程打开时，它会检查在同一个VM中是否已经有另一个Erlang进程正在使用这个Bitcask。如果是，它将与该进程共享keydir。如果没有，它会扫描目录中的所有数据文件，以便构建一个新的keydir。对于任何包含提示文件的数据文件，将扫描提示文件，以加快启动时间。        

这些基本操作是bitcask系统的本质。显然，我们并没有试图在本文档中公开操作的每一个细节;我们的目标是帮助你了解bitcask的一般机制。关于我们轻松过去的几个领域的一些额外说明可能是有序的:  

* 我们提到过，我们依赖操作系统的文件系统缓存来实现读取性能。我们也讨论过在bitcask内部添加一个读缓存，但是考虑到我们现在免费获得了多少里程，还不清楚这能带来多少回报。     

* 我们将很快提供针对各种api类似的本地存储系统的基准测试。然而，我们最初的目标不是成为最快的存储引擎，而是获得“足够”的速度、高质量和简单的代码、设计和文件格式。也就是说，在我们最初的简单基准测试中，我们看到Bitcask在许多场景下都轻松优于其他快速存储系统。    

* 一些最难的实现细节也是大多数外部人员最不感兴趣的，所以我们没有在这个简短的文档中包括(例如)内部keydir锁定方案的描述。Bitcask不执行任何数据压缩，因为这样做的成本/收益非常取决于应用程序。  

* 每项读取或写入的低延迟    
Bitcask速度很快。我们计划很快进行更彻底的基准测试，但是在我们早期的测试中，典型的中位延迟为亚毫秒(以及相当高的百分位数)，我们相信它可以满足我们的目标。     

* 高吞吐量，特别是在写入随机项的传入流时    
在使用慢速磁盘的笔记本电脑的早期测试中，我们看到吞吐量为每秒5000-6000次写。 

* 处理比RAM大得多的数据集的能力     
上面提到的测试在相关系统上使用了超过10xRAM的数据集，并且在那时没有显示出任何改变行为的迹象。考虑到Bitcask的设计，这与我们的期望是一致的。   

* 崩溃友好，无论是在快速恢复和不丢失数据  
由于数据文件和提交日志在Bitcask中是一样的，所以恢复是微不足道的，不需要“重播”。提示文件可以用来加快启动过程。       

* 易于备份和恢复由于文件在旋转后是不可变的，因此备份可以轻松地使用操作人员喜欢的任何系统级机制。恢复只需要将数据文件放在所需的目录中。  

* 相对简单、可理解(因而可支持)的代码结构和数据格式Bitcask在概念上简单，代码干净，数据文件非常容易理解和管理。我们很乐意支持一个基于Bitcask的系统。  

* 在高访问负载或大容量下的可预测行为。  

在繁重的访问负载下，我们已经看到Bitcask表现良好。到目前为止，它只看到了两位数的gb容量，但我们很快就会测试更多。Bitcask的形状是这样的，我们不希望它在更大的容量下表现得太不一样，只有一个可预测的例外，即keydir结构随着密钥的数量增长了一小部分，并且必须完全适合RAM。这个限制在实践中是很小的，因为即使有数百万个键，当前的实现也只使用不到GB的内存。   

总而言之，考虑到这一特定的目标，比特桶比我们现有的任何东西都更适合我们的需求。  

**API示例**     
![bitcask08](http://img.xinzhuxiansheng.com/blogimgs/kv/bitcask08.png)  