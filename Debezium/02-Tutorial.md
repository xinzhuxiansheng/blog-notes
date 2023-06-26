## 官方教程 

> version: 2.3  

本教程演示如何使用 Debezium 监控 MySQL 数据库。当数据库中的数据发生变化时，您将看到生成的事件流。   
在本教程中，您将启动 Debezium 服务，使用简单的示例数据库运行 MySQL 数据库服务器，并使用 Debezium 监视数据库的更改。 

**环境要求**    
* Docker 已安装并运行。
本教程使用 Docker 和 Debezium 容器映像来运行所需的服务。您应该使用最新版本的 Docker。   

### Introduction to Debezium    
Debezium 是一个分布式平台，可将现有数据库中的信息转换为事件流，使应用程序能够检测并立即响应数据库中的行级更改。 

Debezium 构建在 Apache Kafka 之上，并提供一组 Kafka Connect 兼容连接器。每个连接器都与特定的数据库管理系统 (DBMS) 配合使用。连接器通过检测发生的更改并将每个更改事件的记录流式传输到 Kafka 主题来记录 DBMS 中的数据更改历史记录。然后，消费应用程序可以从 Kafka 主题读取生成的事件记录。    

通过利用 Kafka 可靠的流平台，Debezium 使应用程序能够正确、完整地使用数据库中发生的更改。即使您的应用程序意外停止或失去连接，它也不会错过中断期间发生的事件。应用程序重新启动后，它会从上次停止的位置继续读取主题。  

接下来的教程向您展示如何通过简单的配置来部署和使用 Debezium MySQL 连接器。有关部署和使用 Debezium 连接器的更多信息，请参阅连接器文档。      





refer   
1.https://debezium.io/documentation/reference/2.3/tutorial.html 