## 在 Spring Boot启动时触发某些方法执行 

### interface ApplicationListener   
在Spring框架中，事件监听器是一个非常有用的特性，用于处理应用上下文的生命周期事件。`ApplicationListener<ApplicationReadyEvent>` 是其中的一个监听器，专门用于处理 `ApplicationReadyEvent`。

当你实现了 `ApplicationListener<ApplicationReadyEvent>`，这意味着你想在Spring Boot应用启动并初始化完成后，立即执行某些动作。

具体来说，`ApplicationReadyEvent` 在以下情况下发布：

- 所有应用程序和命令行运行程序的 beans 都被创建完毕
- `ApplicationContext` 已经被刷新
- 所有 `ApplicationRunner` 和 `CommandLineRunner` beans 已经被调用

`ApplicationReadyEvent` 是在应用程序完全启动后发布的，这意味着这是一个非常适合执行后台任务、日志记录、启动应用程序内部的服务或任何其他启动后立即需要运行的任务的地方。

以下是一个简单的例子，展示如何使用这个监听器：

```java
import org.springframework.boot.context.event.ApplicationReadyEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.stereotype.Component;

@Component
public class ApplicationReadyEventListener implements ApplicationListener<ApplicationReadyEvent> {
    
    @Override
    public void onApplicationEvent(ApplicationReadyEvent event) {
        // 这里的代码会在应用程序完全启动后立即运行
        System.out.println("应用程序已经准备好了！");
    }
}
```

### interface ApplicationRunner 
`ApplicationRunner` 是 Spring Boot 提供的一个接口，允许开发者在 Spring Boot 应用程序启动后执行一些代码逻辑。当所有的 Spring Bean 都初始化完成，并且 Spring 应用程序上下文完成加载后，所有实现了 `ApplicationRunner` 的 Bean 的 `run` 方法会被自动执行。

`ApplicationRunner` 主要提供了对原始应用参数的访问，使用 `ApplicationArguments` 类型的参数。

这个接口通常用于在应用程序启动后执行一些初始化工作，例如读取数据，初始化缓存，启动一些预加载任务，或其他任何你希望应用程序启动后立即执行的代码。

下面是一个简单的例子，展示如何使用 `ApplicationRunner`：

```java
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;

@Component
public class MyApplicationRunner implements ApplicationRunner {

    @Override
    public void run(ApplicationArguments args) throws Exception {
        System.out.println("Spring Boot应用已启动，现在开始执行一些初始化工作...");
        
        // 在这里执行你的初始化代码
    }
}
```

在上述示例中，当 Spring Boot 应用启动完成后，它会输出指定的消息，并可以在此处执行其他初始化工作。

值得注意的是，除了 `ApplicationRunner`，Spring Boot 还提供了 `CommandLineRunner` 接口，与 `ApplicationRunner` 类似，但它提供的是一个原始的 `String[] args` 参数，允许你访问启动应用程序时传入的原始命令行参数。根据你的需求和你想如何处理参数，你可以选择使用其中之一。


### ApplicationRunner 与 ApplicationListener 两者区别   
`ApplicationRunner` 和 `ApplicationListener` 都是 Spring Boot 提供的接口，允许开发者在应用程序的生命周期的某个特定时间点执行代码。尽管它们都能实现类似的功能，但它们的使用场景和关注点是不同的。下面列出了这两者之间的主要区别：

1. **触发时机**：
   - `ApplicationRunner`（以及 `CommandLineRunner`）：在 Spring Boot 应用启动并初始化完成后执行。具体来说，当所有 Spring Beans 都初始化完毕、Spring 应用上下文加载完成后，`run` 方法被调用。
   - `ApplicationListener`：是用于监听各种 Spring 上下文事件的接口。通过实现 `ApplicationListener` 并指定某个事件，你可以在这个特定事件发生时执行代码。例如，通过实现 `ApplicationListener<ApplicationReadyEvent>`，你可以在应用准备好接受请求时执行代码。

2. **灵活性**：
   - `ApplicationRunner`：主要关注在应用启动后执行代码。
   - `ApplicationListener`：更为通用和灵活，因为它可以监听多种事件，如 `ApplicationContext` 启动、刷新、停止等。

3. **参数**：
   - `ApplicationRunner`：提供 `ApplicationArguments` 类型的参数，使你可以访问应用启动时的参数。
   - `ApplicationListener`：提供特定的事件类型作为参数，例如 `ApplicationReadyEvent`。

4. **用途**：
   - `ApplicationRunner`：通常用于执行一些应用启动后的初始化工作。
   - `ApplicationListener`：除了上述的初始化工作，还可以在其他应用生命周期事件（如上下文停止、上下文刷新等）发生时执行特定代码。

5. **实现方式**：
   - `ApplicationRunner`：需要实现 `run` 方法。
   - `ApplicationListener`：需要实现 `onApplicationEvent` 方法。

综上所述，选择使用哪一个主要取决于你的需求。如果你只想在应用启动后执行一些代码，`ApplicationRunner` 可能更为简洁。但如果你需要在多个应用生命周期事件发生时响应，那么 `ApplicationListener` 提供了更多的灵活性。