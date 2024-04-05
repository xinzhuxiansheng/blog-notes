

## 如何操作自定义资源   
client-go 为每种K8S 内置资源提供对应的`clientset`和`informer`。那如果我们要监听和操作自定义资源对象，应该如何做呢？ 这里我们有两种方式：    
* 方式一：使用 client-go 提供的 `dynamicClient`来操作自定义资源对象，当然由于 dynamicClient 是基于 RESTClient 实现的，所以我们也可以使用 RESTClient来达到同样的目的。     
* 方式二：使用`code-generator`来帮助我们生成我们需要的代码，这样我们就可以像使用 client-go 为K8S内置资源对象提供的方式监听和操作自定义资源了。          

## code-generator 使用  
* 1.获取 code-generator的代码，并切换到 v0.23.3 的tag上。       
```shell
git checkout v0.23.3    
```

* 2.编译项目，安装代码生成工具  
```shell
go install code-generator/cmd/{client-gen,lister-gen,informer-gen,deepcopy-gen}     
```

* 3.使用工具 code-generator/generate-groups.sh  
```shell
code-generator/generate-groups.sh deepcopy,client,informer MOD_NAME/pkg/generated MOD_NAME/pkg/apis foo.example.com:v1 --output-base MOD_DIR/.. --go-header-file "code-generator/hack/boilerplate.go"
```