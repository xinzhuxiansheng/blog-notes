# Kubernetes Operator - 自定义 CRD  

## 如何使用 CRD 
在 Kubernetes系统扩展点中，开发者可以通过 CRD（CustomResourceDefinition）来扩展K8s API，其功能主要由 APIExtensionServer负责。使用 CRD扩展资源分为三步： 
* 注册自定义资源：开发者需要通过K8S提供的方式注册自定义资源，即通过CRD进行注册，注册之后，K8S就知道我们自定义资源的存在了，然后我们就可以像使用K8S内置资源一样使用自定义资源（CR）    
* 使用自定义资源：像内置资源比如 Pod一样声明资源，使用CR声明我们的资源信息  
* 删除自定义资源：当我们不再需要时，可以删除自定义资源 

