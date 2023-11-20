## Chat2DB集成ChatGPT OpenAI

>Chat2DB version: 1.0.9 

### 引言    
引用READM.md关于AIGC介绍：“Chat2DB集成了AIGC的能力，能够将自然语言转换为SQL，也可以将SQL转换为自然语言，可以给出研发人员SQL的优化建议，极大的提升人员的效率，是AI时代数据库研发人员的利器，未来即使不懂SQL的运营业务也可以使用快速查询业务数据、生成报表能力。” 

>本篇主要讲解集成AIGC的实现过程，若你对Prompt还不太了解，可以参考下`ttps://github.com/wangxuqi/Prompt-Engineering-Guide-Chinese`        

### ChatGTP配置
![aigcSQL01](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL01.png)  
* 生成并配置ChatGPT Key，创建地址： https://platform.openai.com/account/api-keys    

![aigcSQL02](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL02.png)  

* 配置本地VPN代理地址   
如果本地VPN未配置`全局生效`，需通过在客户端中设置网络代理HOST和PORT来保证网络联通性         

### AIGC实现    
![aigcSQL03](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL03.png)      

我现在使用`select * from user`来模拟 `SQL优化`功能，下面是接口请求流程      

`step01 请求ChatGPT配置信息`    
```shell  
http://127.0.0.1:10821/api/config/system_config/chatgpt 
# GET请求

# 返回值
{
    "success": true,
    "errorCode": null,
    "errorMessage": null,
    "data":
    {
        "apiKey": "sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
        "apiHost": null,
        "httpProxyHost": "127.0.0.1",
        "httpProxyPort": "7890",
        "aiSqlSource": "OPENAI",
        "restAiUrl": null,
        "restAiStream": true
    },
    "traceId": null
}
```

`step02 请求SQL优化，但HTTP Content-Type：text/event-stream`    
```shell  
http://127.0.0.1:10821/api/ai/chat?dataSourceId=3&databaseName=yzhou_test&promptType=SQL_OPTIMIZER&message=select * from user;&ext=优化sql&=    
# GET请求

# HTTP Content-Type：text/event-stream
``` 

>Server-Sent Events是一种允许服务器向客户端推送事件的技术，通常用于实现实时更新, 这是单向的，而WebSocket是双向通信。可参考该篇Blog对SSE的介绍“https://segmentfault.com/a/1190000020628924”      

所以这里就解决了`流式传输`的技术点。 当在页面触发`SQL优化`等AIGC功能时，页面返回的数据并不是一次性返回的，而是陆陆续续返回。那么我们来看下Chat2DB又是如何借助Spring Boot来实现SSE(Server-Send Events)?  

### SSE的实现   
我们先列举一个Spring Boot使用Spring MVC来实现SSE的例子：     
```java
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

@RestController
public class SseController {

    @GetMapping("/sse")
    public SseEmitter handleSse() {
        SseEmitter emitter = new SseEmitter();

        // Create a new Thread to avoid blocking the Servlet container's main thread
        new Thread(() -> {
            for (int i = 0; i < 10; i++) {
                try {
                    // Send a SSE every second
                    emitter.send("Event " + i);
                    Thread.sleep(1000);
                } catch (Exception e) {
                    emitter.completeWithError(e);
                    return;
                }
            }
            emitter.complete();
        }).start();

        return emitter;
    }
}

```

`实现步骤总结：`   
1.创建SseEmitter对象 sseEmitter     
2.使用SseEmitter#send()方法将数据传输给页面，每调用一次，传输一次数据   
3.使用SseEmitter#complete()方法标记完成。   
4.ChatController#completions()方法返回SseEmitter类型    

以上就算是在完成了，感觉实现起来挺简单 :)    

我们回到Chat2DB项目来，看它是如何实现的?    

### ChatGpt OpenAI调用

**ChatController#completions()**    
```java
@GetMapping("/chat")
@CrossOrigin
public SseEmitter completions(ChatQueryRequest queryRequest, @RequestHeader Map<String, String> headers)
    throws IOException {
    //默认30秒超时,设置为0L则永不超时
    SseEmitter sseEmitter = new SseEmitter(CHAT_TIMEOUT);
    String uid = headers.get("uid");
    if (StrUtil.isBlank(uid)) {
        throw new BusinessException(CommonErrorEnum.COMMON_SYSTEM_ERROR);
    }

    //提示消息不得为空
    if (StringUtils.isBlank(queryRequest.getMessage())) {
        throw new BusinessException(CommonErrorEnum.PARAM_ERROR);
    }

    if (useOpenAI()) {
        return chatWithOpenAiSql(queryRequest, sseEmitter, uid);
    }
    return chatWithRestAi(queryRequest.getMessage(), sseEmitter);
}
```
* 1.创建`SseEmitter sseEmitter = new SseEmitter(CHAT_TIMEOUT);` 
* 2.使用SseEmitter构建OpenAIEventSourceListener 
```java
OpenAIEventSourceListener openAIEventSourceListener = new OpenAIEventSourceListener(sseEmitter);    
```

>okhttp3的EventSourceListener是什么作用?    
OkHttp是一款非常流行的开源HTTP客户端，它用于Java和Android项目。它的主要特性包括对HTTP/2和SPDY的支持，以及对连接池、GZIP和HTTP缓存等特性的支持。 
OkHttp3的`EventSourceListener`是OkHttp的一个接口，主要用于处理Server-Sent Events (SSE)。SSE是一种允许服务器端推送数据到客户端的技术，它使用`text/event-stream`媒体类型，并允许服务器不断向客户端发送新的数据，直到连接被关闭。  

`EventSourceListener`接口定义了几个方法，你可以覆盖这些方法来处理不同的事件：   

- `onOpen(EventSource eventSource, Response response)`：当一个新的SSE连接被打开时，这个方法会被调用。你可以在这个方法中进行一些初始化操作。 
- `onEvent(EventSource eventSource, String id, String type, String data)`：当从服务器接收到一个新的事件时，这个方法会被调用。你可以在这个方法中处理接收到的数据。   
- `onClosed(EventSource eventSource)`：当SSE连接被关闭时，这个方法会被调用。你可以在这个方法中进行一些清理操作。    
- `onFailure(EventSource eventSource, Throwable t, Response response)`：当发生错误时，这个方法会被调用。你可以在这个方法中处理错误。    
总的来说，OkHttp3的`EventSourceListener`主要用于处理Server-Sent Events，你可以通过实现这个接口来处理SSE连接的生命周期，以及从服务器接收到的事件。   

从OKHttp了解可知，Chat2DB利用它来管理SseEmitter的生命周期，简化了SseEmitter的使用，从而更关注ChatGPT OpenAI的调用。  

* 3.构建ChatGPT OpenAI的Request 
![aigcSQL06](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL06.png)    

```java
private SseEmitter chatWithOpenAiSql(ChatQueryRequest queryRequest, SseEmitter sseEmitter, String uid)
        throws IOException {
    String prompt = buildPrompt(queryRequest);
    if (prompt.length() / TOKEN_CONVERT_CHAR_LENGTH > MAX_PROMPT_LENGTH) {
        log.error("提示语超出最大长度:{}，输入长度:{}, 请重新输入", MAX_PROMPT_LENGTH,
            prompt.length() / TOKEN_CONVERT_CHAR_LENGTH);
        throw new BusinessException(CommonErrorEnum.PARAM_ERROR);
    }

    GptVersionType modelType = EasyEnumUtils.getEnum(GptVersionType.class, gptVersion);
    switch (modelType) {
        case GPT3:
            return chatGpt3(prompt, sseEmitter, uid);
        case GPT35:
            List<Message> messages = new ArrayList<>();
            prompt = prompt.replaceAll("#", "");
            log.info(prompt);
            Message currentMessage = Message.builder().content(prompt).role(Message.Role.USER).build();
            messages.add(currentMessage);
            return chatGpt35(messages, sseEmitter, uid);
        default:
            break;
    }
    return chatGpt3(prompt, sseEmitter, uid);
}
```

Rquest交给`com.unfbx.chatgpt` openAI SDK处理请求，SDK使用详细请参考`https://github.com/Grt1228/chatgpt-java`，而openAI的API文档请参考`https://platform.openai.com/docs/api-reference/chat`   

![aigcSQL05](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL05.png)    

* 4.将上下文缓存起来，当页面切换到`chatRobot`模块时，能继承上下文 ，因为在openAI的请求参数messages字段是一个数组。    
```
LocalCache.CACHE.put(uid, JSONUtil.toJsonStr(messages), LocalCache.TIMEOUT);       
```
![aigcSQL08](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL08.png)     

这里仍然有个细节，我们配置了ChatGPT代理，那又是如何在OKHttp调用的？ 请参考`com.unfbx.chatgpt` SDK中关于OKHttp API的使用。   
![aigcSQL07](http://img.xinzhuxiansheng.com/blogimgs/chat2db/aigcSQL07.png)  

>博主对OKHttp不熟悉呀，对于其他AIGC的功能实现过程都类似。      

refer     
1.https://github.com/wangxuqi/Prompt-Engineering-Guide-Chinese/blob/main/guides/prompts-intro.md#%E6%8C%87%E4%BB%A4   
2.https://platform.openai.com/docs/api-reference/chat    
3.https://github.com/Grt1228/chatgpt-java     
4.https://github.com/noobnooc/noobnooc/discussions/9      
5.https://github.com/f/awesome-chatgpt-prompts    
6.https://segmentfault.com/a/1190000020628924     
