
### 1. 
     flinkVersion   : 1.14.0
     withSavePoint  : false
     withDrain      : false
     k8sNamespace   : yzhou
     appId          : streamx-yzhou-consumer003
     jobId          : 09ef11931e3f05df66b2b89e1ed8142c
-------------------------------------------------------------------------------------------

2021-12-13 02:58:42 | INFO  | streamx-deploy-executor-0 | org.apache.flink.kubernetes.KubernetesClusterDescriptor ] Retrieve flink cluster streamx-yzhou-consumer003 successfully, JobManager Web Interface: http://xx.xx.212.25:31425
2021-12-13 02:58:43 | INFO  | streamx-deploy-executor-0 | com.streamxhub.streamx.console.core.service.impl.ApplicationServiceImpl ] savePoint path:
2021-12-13 02:58:43 | INFO  | streamx-deploy-executor-0 | com.streamxhub.streamx.console.core.service.impl.ApplicationServiceImpl ] savePoint path:
2021-12-13 02:58:43 | INFO  | ForkJoinPool-1-worker-29 | org.apache.hc.client5.http.impl.classic.HttpRequestRetryExec ] Recoverable I/O exception (org.apache.hc.core5.http.NoHttpResponseException) caught when processing request to {}->http://xx.xx.212.25:31425
uu^H^H

java.lang.NullPointerException
        at java.util.Objects.requireNonNull(Objects.java:203)
        at com.streamxhub.streamx.console.core.service.impl.ProjectServiceImpl.jars(ProjectServiceImpl.java:268)
        at com.streamxhub.streamx.console.core.service.impl.ProjectServiceImpl$$FastClassBySpringCGLIB$$125a2513.invoke(<generated>)
        at org.springframework.cglib.proxy.MethodProxy.invoke(MethodProxy.java:218)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.invokeJoinpoint(CglibAopProxy.java:771)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:163)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.transaction.interceptor.TransactionAspectSupport.invokeWithinTransaction(TransactionAspectSupport.java:367)
        at org.springframework.transaction.interceptor.TransactionInterceptor.invoke(TransactionInterceptor.java:118)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:186)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.aop.framework.CglibAopProxy$DynamicAdvisedInterceptor.intercept(CglibAopProxy.java:691)
        at com.streamxhub.streamx.console.core.service.impl.ProjectServiceImpl$$EnhancerBySpringCGLIB$$87724c97.jars(<generated>)
        at com.streamxhub.streamx.console.core.controller.ProjectController.jars(ProjectController.java:112)
        at com.streamxhub.streamx.console.core.controller.ProjectController$$FastClassBySpringCGLIB$$81e457e1.invoke(<generated>)
        at org.springframework.cglib.proxy.MethodProxy.invoke(MethodProxy.java:218)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.invokeJoinpoint(CglibAopProxy.java:771)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:163)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.validation.beanvalidation.MethodValidationInterceptor.invoke(MethodValidationInterceptor.java:119)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:186)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.aop.aspectj.MethodInvocationProceedingJoinPoint.proceed(MethodInvocationProceedingJoinPoint.java:88)
        at com.streamxhub.streamx.console.core.aspect.StreamXConsoleAspect.response(StreamXConsoleAspect.java:64)
        at sun.reflect.GeneratedMethodAccessor302.invoke(Unknown Source)
        at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
        at java.lang.reflect.Method.invoke(Method.java:498)
        at org.springframework.aop.aspectj.AbstractAspectJAdvice.invokeAdviceMethodWithGivenArgs(AbstractAspectJAdvice.java:644)
        at org.springframework.aop.aspectj.AbstractAspectJAdvice.invokeAdviceMethod(AbstractAspectJAdvice.java:633)
        at org.springframework.aop.aspectj.AspectJAroundAdvice.invoke(AspectJAroundAdvice.java:70)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:186)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.aop.interceptor.ExposeInvocationInterceptor.invoke(ExposeInvocationInterceptor.java:95)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:186)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.aop.framework.CglibAopProxy$DynamicAdvisedInterceptor.intercept(CglibAopProxy.java:691)
        at com.streamxhub.streamx.console.core.controller.ProjectController$$EnhancerBySpringCGLIB$$e5fdd803.jars(<generated>)
        at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
        at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
        at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
        at java.lang.reflect.Method.invoke(Method.java:498)
        at org.springframework.web.method.support.InvocableHandlerMethod.doInvoke(InvocableHandlerMethod.java:190)
        at org.springframework.web.method.support.InvocableHandlerMethod.invokeForRequest(InvocableHandlerMethod.java:138)
        at org.springframework.web.servlet.mvc.method.annotation.ServletInvocableHandlerMethod.invokeAndHandle(ServletInvocableHandlerMethod.java:105)
        at org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerAdapter.invokeHandlerMethod(RequestMappingHandlerAdapter.java:878)
        at org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerAdapter.handleInternal(RequestMappingHandlerAdapter.java:792)
        at org.springframework.web.servlet.mvc.method.AbstractHandlerMethodAdapter.handle(AbstractHandlerMethodAdapter.java:87)
        at org.springframework.web.servlet.DispatcherServlet.doDispatch(DispatcherServlet.java:1040)
        at org.springframework.web.servlet.DispatcherServlet.doService(DispatcherServlet.java:943)
        at org.springframework.web.servlet.FrameworkServlet.processRequest(FrameworkServlet.java:1006)
        at org.springframework.web.servlet.FrameworkServlet.doPost(FrameworkServlet.java:909)
        at javax.servlet.http.HttpServlet.service(HttpServlet.java:517)
        at org.springframework.web.servlet.FrameworkServlet.service(FrameworkServlet.java:883)
        at javax.servlet.http.HttpServlet.service(HttpServlet.java:584)
        at io.undertow.servlet.handlers.ServletHandler.handleRequest(ServletHandler.java:74)
        at io.undertow.servlet.handlers.FilterHandler$FilterChainImpl.doFilter(FilterHandler.java:129)
        at org.springframework.web.filter.CorsFilter.doFilterInternal(CorsFilter.java:92)
        at org.springframework.web.filter.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:119)
        at io.undertow.servlet.core.ManagedFilter.doFilter(ManagedFilter.java:61)
        at io.undertow.servlet.handlers.FilterHandler$FilterChainImpl.doFilter(FilterHandler.java:131)
        at org.apache.shiro.web.servlet.ProxiedFilterChain.doFilter(ProxiedFilterChain.java:61)
        at org.apache.shiro.web.servlet.AdviceFilter.executeChain(AdviceFilter.java:108)
        at org.apache.shiro.web.servlet.AdviceFilter.doFilterInternal(AdviceFilter.java:137)
        at org.apache.shiro.web.servlet.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:125)
        at org.apache.shiro.web.servlet.ProxiedFilterChain.doFilter(ProxiedFilterChain.java:66)
        at org.apache.shiro.web.servlet.AdviceFilter.executeChain(AdviceFilter.java:108)
        at org.apache.shiro.web.servlet.AdviceFilter.doFilterInternal(AdviceFilter.java:137)
        at org.apache.shiro.web.servlet.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:125)
        at org.apache.shiro.web.servlet.ProxiedFilterChain.doFilter(ProxiedFilterChain.java:66)
        at org.apache.shiro.web.servlet.AbstractShiroFilter.executeChain(AbstractShiroFilter.java:450)
        at org.apache.shiro.web.servlet.AbstractShiroFilter$1.call(AbstractShiroFilter.java:365)
        at org.apache.shiro.subject.support.SubjectCallable.doCall(SubjectCallable.java:90)
        at org.apache.shiro.subject.support.SubjectCallable.call(SubjectCallable.java:83)
        at org.apache.shiro.subject.support.DelegatingSubject.execute(DelegatingSubject.java:387)
        at org.apache.shiro.web.servlet.AbstractShiroFilter.doFilterInternal(AbstractShiroFilter.java:362)
        at org.apache.shiro.web.servlet.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:125)
        at io.undertow.servlet.core.ManagedFilter.doFilter(ManagedFilter.java:61)
        at io.undertow.servlet.handlers.FilterHandler$FilterChainImpl.doFilter(FilterHandler.java:131)
        at org.springframework.web.filter.RequestContextFilter.doFilterInternal(RequestContextFilter.java:100)
        at org.springframework.web.filter.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:119)
        at io.undertow.servlet.core.ManagedFilter.doFilter(ManagedFilter.java:61)
        at io.undertow.servlet.handlers.FilterHandler$FilterChainImpl.doFilter(FilterHandler.java:131)
        at org.springframework.web.filter.FormContentFilter.doFilterInternal(FormContentFilter.java:93)
        at org.springframework.web.filter.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:119)
        at io.undertow.servlet.core.ManagedFilter.doFilter(ManagedFilter.java:61)
        at io.undertow.servlet.handlers.FilterHandler$FilterChainImpl.doFilter(FilterHandler.java:131)
        at org.springframework.web.filter.CharacterEncodingFilter.doFilterInternal(CharacterEncodingFilter.java:201)
        at org.springframework.web.filter.OncePerRequestFilter.doFilter(OncePerRequestFilter.java:119)
        at io.undertow.servlet.core.ManagedFilter.doFilter(ManagedFilter.java:61)
        at io.undertow.servlet.handlers.FilterHandler$FilterChainImpl.doFilter(FilterHandler.java:131)
        at io.undertow.servlet.handlers.FilterHandler.handleRequest(FilterHandler.java:84)
        at io.undertow.servlet.handlers.security.ServletSecurityRoleHandler.handleRequest(ServletSecurityRoleHandler.java:62)
        at io.undertow.servlet.handlers.ServletChain$1.handleRequest(ServletChain.java:68)
        at io.undertow.servlet.handlers.ServletDispatchingHandler.handleRequest(ServletDispatchingHandler.java:36)
        at io.undertow.servlet.handlers.RedirectDirHandler.handleRequest(RedirectDirHandler.java:68)
        at io.undertow.servlet.handlers.security.SSLInformationAssociationHandler.handleRequest(SSLInformationAssociationHandler.java:111)
        at io.undertow.servlet.handlers.security.ServletAuthenticationCallHandler.handleRequest(ServletAuthenticationCallHandler.java:57)
        at io.undertow.server.handlers.PredicateHandler.handleRequest(PredicateHandler.java:43)
        at io.undertow.security.handlers.AbstractConfidentialityHandler.handleRequest(AbstractConfidentialityHandler.java:46)
        at io.undertow.servlet.handlers.security.ServletConfidentialityConstraintHandler.handleRequest(ServletConfidentialityConstraintHandler.java:64)
        at io.undertow.security.handlers.AuthenticationMechanismsHandler.handleRequest(AuthenticationMechanismsHandler.java:60)
        at io.undertow.servlet.handlers.security.CachedAuthenticatedSessionHandler.handleRequest(CachedAuthenticatedSessionHandler.java:77)
        at io.undertow.security.handlers.AbstractSecurityContextAssociationHandler.handleRequest(AbstractSecurityContextAssociationHandler.java:43)
        at io.undertow.server.handlers.PredicateHandler.handleRequest(PredicateHandler.java:43)
        at io.undertow.server.handlers.PredicateHandler.handleRequest(PredicateHandler.java:43)
        at io.undertow.servlet.handlers.ServletInitialHandler.handleFirstRequest(ServletInitialHandler.java:269)
        at io.undertow.servlet.handlers.ServletInitialHandler.access$100(ServletInitialHandler.java:78)
        at io.undertow.servlet.handlers.ServletInitialHandler$2.call(ServletInitialHandler.java:133)
        at io.undertow.servlet.handlers.ServletInitialHandler$2.call(ServletInitialHandler.java:130)
        at io.undertow.servlet.core.ServletRequestContextThreadSetupAction$1.call(ServletRequestContextThreadSetupAction.java:48)
        at io.undertow.servlet.core.ContextClassLoaderSetupAction$1.call(ContextClassLoaderSetupAction.java:43)
        at io.undertow.servlet.handlers.ServletInitialHandler.dispatchRequest(ServletInitialHandler.java:249)
        at io.undertow.servlet.handlers.ServletInitialHandler.access$000(ServletInitialHandler.java:78)
        at io.undertow.servlet.handlers.ServletInitialHandler$1.handleRequest(ServletInitialHandler.java:99)
        at io.undertow.server.Connectors.executeRootHandler(Connectors.java:370)
        at io.undertow.server.HttpServerExchange$1.run(HttpServerExchange.java:836)
        at org.jboss.threads.ContextClassLoaderSavingRunnable.run(ContextClassLoaderSavingRunnable.java:35)
        at org.jboss.threads.EnhancedQueueExecutor.safeRun(EnhancedQueueExecutor.java:2019)
        at org.jboss.threads.EnhancedQueueExecutor$ThreadBody.doRunTask(EnhancedQueueExecutor.java:1558)
        at org.jboss.threads.EnhancedQueueExecutor$ThreadBody.run(EnhancedQueueExecutor.java:1449)
        at java.lang.Thread.run(Thread.java:748)



### 3. 启动 Docker
Caused by: java.lang.RuntimeException: java.io.IOException: com.sun.jna.LastErrorException: [2] No such file or directory
	at com.github.dockerjava.httpclient5.ApacheDockerHttpClientImpl.execute(ApacheDockerHttpClientImpl.java:187)
	at com.github.dockerjava.httpclient5.ApacheDockerHttpClient.execute(ApacheDockerHttpClient.java:9)
	at com.github.dockerjava.core.DefaultInvocationBuilder.execute(DefaultInvocationBuilder.java:228)
	at com.github.dockerjava.core.DefaultInvocationBuilder.lambda$executeAndStream$1(DefaultInvocationBuilder.java:269)
	... 1 more
Caused by: java.io.IOException: com.sun.jna.LastErrorException: [2] No such file or directory
	at com.github.dockerjava.transport.DomainSocket.<init>(DomainSocket.java:63)
	at com.github.dockerjava.transport.BsdDomainSocket.<init>(BsdDomainSocket.java:43)
	at com.github.dockerjava.transport.DomainSocket.get(DomainSocket.java:138)
	at com.github.dockerjava.transport.UnixSocket.get(UnixSocket.java:27)
	at com.github.dockerjava.httpclient5.ApacheDockerHttpClientImpl$2.createSocket(ApacheDockerHttpClientImpl.java:145)
	at org.apache.hc.client5.http.impl.io.DefaultHttpClientConnectionOperator.connect(DefaultHttpClientConnectionOperator.java:125)
	at org.apache.hc.client5.http.impl.io.PoolingHttpClientConnectionManager.connect(PoolingHttpClientConnectionManager.java:407)
	at org.apache.hc.client5.http.impl.classic.InternalExecRuntime.connectEndpoint(InternalExecRuntime.java:168)
	at org.apache.hc.client5.http.impl.classic.InternalExecRuntime.connectEndpoint(InternalExecRuntime.java:178)
	at org.apache.hc.client5.http.impl.classic.ConnectExec.execute(ConnectExec.java:136)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement.execute(ExecChainElement.java:51)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement$1.proceed(ExecChainElement.java:57)
	at org.apache.hc.client5.http.impl.classic.ProtocolExec.execute(ProtocolExec.java:165)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement.execute(ExecChainElement.java:51)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement$1.proceed(ExecChainElement.java:57)
	at org.apache.hc.client5.http.impl.classic.HttpRequestRetryExec.execute(HttpRequestRetryExec.java:96)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement.execute(ExecChainElement.java:51)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement$1.proceed(ExecChainElement.java:57)
	at org.apache.hc.client5.http.impl.classic.ContentCompressionExec.execute(ContentCompressionExec.java:133)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement.execute(ExecChainElement.java:51)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement$1.proceed(ExecChainElement.java:57)
	at org.apache.hc.client5.http.impl.classic.RedirectExec.execute(RedirectExec.java:115)
	at org.apache.hc.client5.http.impl.classic.ExecChainElement.execute(ExecChainElement.java:51)
	at org.apache.hc.client5.http.impl.classic.InternalHttpClient.doExecute(InternalHttpClient.java:179)
	at org.apache.hc.client5.http.impl.classic.CloseableHttpClient.execute(CloseableHttpClient.java:67)
	at com.github.dockerjava.httpclient5.ApacheDockerHttpClientImpl.execute(ApacheDockerHttpClientImpl.java:183)
	... 4 more
Caused by: com.sun.jna.LastErrorException: [2] No such file or directory
	at com.github.dockerjava.transport.BsdDomainSocket.connect(Native Method)
	at com.github.dockerjava.transport.BsdDomainSocket.connect(BsdDomainSocket.java:49)
	at com.github.dockerjava.transport.DomainSocket.open(DomainSocket.java:69)
	at com.github.dockerjava.transport.DomainSocket.<init>(DomainSocket.java:59)
	... 29 more


### 4. xxx
2021-12-16 15:35:00 | INFO  | XNIO-1 task-1 | com.streamxhub.streamx.flink.packer.docker.DockerTool ] [StreamX] [streamx-packer] docker image built successfully, imageId=b5ab9da2307e, tag=hub.bitxxxxx.com/streamx/streamxflinkjob-yzhou-flink-ns-yzhou-121-producer-01
2021-12-16 15:35:02 | ERROR | XNIO-1 task-1 | com.streamxhub.streamx.flink.packer.docker.DockerTool ] [StreamX] [streamx-packer] push flink job docker image failed.
com.github.dockerjava.api.exception.DockerClientException: Could not push image: unauthorized: unauthorized to access repository: streamx/streamxflinkjob-yzhou-flink-ns-yzhou-121-producer-01, action: push: unauthorized to access repository: streamx/streamxflinkjob-yzhou-flink-ns-yzhou-121-producer-01, action: push
        at com.github.dockerjava.api.command.PushImageCmd$1.throwFirstError(PushImageCmd.java:70)
        at com.github.dockerjava.api.async.ResultCallbackTemplate.awaitCompletion(ResultCallbackTemplate.java:93)
        at com.streamxhub.streamx.flink.packer.docker.DockerTool$$anonfun$buildFlinkImage$5.apply(DockerTool.scala:96)
        at com.streamxhub.streamx.flink.packer.docker.DockerTool$$anonfun$buildFlinkImage$5.apply(DockerTool.scala:94)
        at com.streamxhub.streamx.common.util.Utils$.tryWithResourceException(Utils.scala:112)
        at com.streamxhub.streamx.flink.packer.docker.package$.usingDockerClient(package.scala:49)
        at com.streamxhub.streamx.flink.packer.docker.DockerTool$.buildFlinkImage(DockerTool.scala:98)
        at com.streamxhub.streamx.flink.submit.impl.KubernetesNativeApplicationSubmit$.doSubmit(KubernetesNativeApplicationSubmit.scala:115)
        at com.streamxhub.streamx.flink.submit.trait.FlinkSubmitTrait$class.submit(FlinkSubmitTrait.scala:73)
        at com.streamxhub.streamx.flink.submit.impl.KubernetesNativeApplicationSubmit$.submit(KubernetesNativeApplicationSubmit.scala:42)
        at com.streamxhub.streamx.flink.submit.FlinkSubmit$.submit(FlinkSubmit.scala:34)
        at com.streamxhub.streamx.flink.submit.FlinkSubmit.submit(FlinkSubmit.scala)
        at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
        at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
        at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
        at java.lang.reflect.Method.invoke(Method.java:498)
        at com.streamxhub.streamx.flink.submit.FlinkSubmitHelper$$anonfun$submit$1.apply(FlinkSubmitHelper.scala:50)
        at com.streamxhub.streamx.flink.submit.FlinkSubmitHelper$$anonfun$submit$1.apply(FlinkSubmitHelper.scala:45)
        at com.streamxhub.streamx.flink.proxy.FlinkShimsProxy$$anonfun$proxy$1.apply(FlinkShimsProxy.scala:67)
        at com.streamxhub.streamx.common.util.ClassLoaderUtils$.runAsClassLoader(ClassLoaderUtils.scala:40)
        at com.streamxhub.streamx.flink.proxy.FlinkShimsProxy$.proxy(FlinkShimsProxy.scala:67)
        at com.streamxhub.streamx.flink.submit.FlinkSubmitHelper$.submit(FlinkSubmitHelper.scala:45)
        at com.streamxhub.streamx.flink.submit.FlinkSubmitHelper.submit(FlinkSubmitHelper.scala)
        at com.streamxhub.streamx.console.core.service.impl.ApplicationServiceImpl.start(ApplicationServiceImpl.java:1290)
        at com.streamxhub.streamx.console.core.service.impl.ApplicationServiceImpl$$FastClassBySpringCGLIB$$4cca1efc.invoke(<generated>)
        at org.springframework.cglib.proxy.MethodProxy.invoke(MethodProxy.java:218)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.invokeJoinpoint(CglibAopProxy.java:771)
        at org.springframework.aop.framework.ReflectiveMethodInvocation.proceed(ReflectiveMethodInvocation.java:163)
        at org.springframework.aop.framework.CglibAopProxy$CglibMethodInvocation.proceed(CglibAopProxy.java:749)
        at org.springframework.aop.aspectj.MethodInvocationProceedingJoinPoint.proceed(MethodInvocationProceedingJoinPoint.java:88)
        at com.streamxhub.streamx.console.core.aspect.StreamXConsoleAspect.lambda$refreshCache$0(StreamXConsoleAspect.java:89)
        at com.streamxhub.streamx.console.core.task.FlinkTrackingTask.refreshTracking(FlinkTrackingTask.java:634)
        at com.streamxhub.streamx.console.core.aspect.StreamXConsoleAspect.refreshCache(StreamXConsoleAspect.java:87)
        at sun.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
        at sun.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:62)
        at sun.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
        at java.lang.reflect.Method.invoke(Method.java:498)
        at org.springframework.aop.aspectj.AbstractAspectJAdvice.invokeAdviceMethodWithGivenArgs(AbstractAspectJAdvice.java:644)


#### 5. xxxx
2021-12-16 17:51:41 | WARN  | XNIO-1 task-5 | org.apache.flink.kubernetes.KubernetesClusterDescriptor ] Please note that Flink client operations(e.g. cancel, list, stop, savepoint, etc.) won't work from outside the Kubernetes cluster since 'kubernetes.rest-service.exposed.type' has been set to ClusterIP.
2021-12-16 17:51:41 | INFO  | XNIO-1 task-5 | org.apache.flink.kubernetes.KubernetesClusterDescriptor ] Create flink application cluster yzhou-121-producer-01 successfully, JobManager Web Interface: http://yzhou-121-producer-01-rest.yzhou-flink-ns:8081
2021-12-16 17:51:41 | WARN  | XNIO-1 task-5 | org.apache.flink.kubernetes.KubernetesClusterDescriptor ] Please note that Flink client operations(e.g. cancel, list, stop, savepoint, etc.) won't work from outside the Kubernetes cluster since 'kubernetes.rest-service.exposed.type' has been set to ClusterIP.