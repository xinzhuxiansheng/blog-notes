
# long polling

## 1. long polling介绍

## 2. 结构图
`结构图`    
![long polling结构图](images/longpolling01.png)

## 3. Apollo实现讲解
根据Apollo的官网文档实现细节（https://www.apolloconfig.com/#/zh/design/apollo-design），下面会从代码中分析
```xml
2.1.2 Config Service通知客户端的实现方式

上一节中简要描述了NotificationControllerV2是如何得知有配置发布的，那NotificationControllerV2在得知有配置发布后是如何通知到客户端的呢？

实现方式如下：

1.客户端会发起一个Http请求到Config Service的notifications/v2接口，也就是NotificationControllerV2，参见RemoteConfigLongPollService
2.NotificationControllerV2不会立即返回结果，而是通过Spring DeferredResult把请求挂起
3.如果在60秒内没有该客户端关心的配置发布，那么会返回Http状态码304给客户端
4.如果有该客户端关心的配置发布，NotificationControllerV2会调用DeferredResult的setResult方法，传入有配置变化的namespace信息，同时该请求会立即返回。客户端从返回的结果中获取到配置变化的namespace后，会立即请求Config Service获取该namespace的最新配置。
```

### 3.1 Client
在doLongPollingRefresh()方法中

`流程图` 
![在doLongPollingRefresh()流程图](images/longpolling02.png)

`RemoteConfigLongPollService`  
```java
private void doLongPollingRefresh(String appId, String cluster, String dataCenter, String secret) {
    final Random random = new Random();
    ServiceDTO lastServiceDto = null;
    while (!m_longPollingStopped.get() && !Thread.currentThread().isInterrupted()) {
      if (!m_longPollRateLimiter.tryAcquire(5, TimeUnit.SECONDS)) {
        //wait at most 5 seconds
        try {
          TimeUnit.SECONDS.sleep(5);
        } catch (InterruptedException e) {
        }
      }
      Transaction transaction = Tracer.newTransaction("Apollo.ConfigService", "pollNotification");
      String url = null;
      try {
        if (lastServiceDto == null) {
          List<ServiceDTO> configServices = getConfigServices();
          lastServiceDto = configServices.get(random.nextInt(configServices.size()));
        }

        url =
            assembleLongPollRefreshUrl(lastServiceDto.getHomepageUrl(), appId, cluster, dataCenter,
                m_notifications);

        logger.debug("Long polling from {}", url);

        HttpRequest request = new HttpRequest(url);
        request.setReadTimeout(LONG_POLLING_READ_TIMEOUT);
        if (!StringUtils.isBlank(secret)) {
          Map<String, String> headers = Signature.buildHttpHeaders(url, appId, secret);
          request.setHeaders(headers);
        }

        transaction.addData("Url", url);

        final HttpResponse<List<ApolloConfigNotification>> response =
            m_httpClient.doGet(request, m_responseType);

        logger.debug("Long polling response: {}, url: {}", response.getStatusCode(), url);
        if (response.getStatusCode() == 200 && response.getBody() != null) {
          updateNotifications(response.getBody());
          updateRemoteNotifications(response.getBody());
          transaction.addData("Result", response.getBody().toString());
          notify(lastServiceDto, response.getBody());
        }

        //try to load balance
        if (response.getStatusCode() == 304 && random.nextBoolean()) {
          lastServiceDto = null;
        }

        m_longPollFailSchedulePolicyInSecond.success();
        transaction.addData("StatusCode", response.getStatusCode());
        transaction.setStatus(Transaction.SUCCESS);
      } catch (Throwable ex) {
        lastServiceDto = null;
        Tracer.logEvent("ApolloConfigException", ExceptionUtil.getDetailMessage(ex));
        transaction.setStatus(ex);
        long sleepTimeInSecond = m_longPollFailSchedulePolicyInSecond.fail();
        logger.warn(
            "Long polling failed, will retry in {} seconds. appId: {}, cluster: {}, namespaces: {}, long polling url: {}, reason: {}",
            sleepTimeInSecond, appId, cluster, assembleNamespaces(), url, ExceptionUtil.getDetailMessage(ex));
        try {
          TimeUnit.SECONDS.sleep(sleepTimeInSecond);
        } catch (InterruptedException ie) {
          //ignore
        }
      } finally {
        transaction.complete();
      }
    }
  }
```

### 3.2 Server

`NotificationControllerV2`  
```java
@GetMapping
  public DeferredResult<ResponseEntity<List<ApolloConfigNotification>>> pollNotification(
      @RequestParam(value = "appId") String appId,
      @RequestParam(value = "cluster") String cluster,
      @RequestParam(value = "notifications") String notificationsAsString,
      @RequestParam(value = "dataCenter", required = false) String dataCenter,
      @RequestParam(value = "ip", required = false) String clientIp) {
    List<ApolloConfigNotification> notifications = null;

    try {
      notifications =
          gson.fromJson(notificationsAsString, notificationsTypeReference);
    } catch (Throwable ex) {
      Tracer.logError(ex);
    }

    if (CollectionUtils.isEmpty(notifications)) {
      throw new BadRequestException("Invalid format of notifications: " + notificationsAsString);
    }
    
    Map<String, ApolloConfigNotification> filteredNotifications = filterNotifications(appId, notifications);

    if (CollectionUtils.isEmpty(filteredNotifications)) {
      throw new BadRequestException("Invalid format of notifications: " + notificationsAsString);
    }
    
    DeferredResultWrapper deferredResultWrapper = new DeferredResultWrapper(bizConfig.longPollingTimeoutInMilli());
    Set<String> namespaces = Sets.newHashSetWithExpectedSize(filteredNotifications.size());
    Map<String, Long> clientSideNotifications = Maps.newHashMapWithExpectedSize(filteredNotifications.size());
    
    for (Map.Entry<String, ApolloConfigNotification> notificationEntry : filteredNotifications.entrySet()) {
      String normalizedNamespace = notificationEntry.getKey();
      ApolloConfigNotification notification = notificationEntry.getValue();
      namespaces.add(normalizedNamespace);
      clientSideNotifications.put(normalizedNamespace, notification.getNotificationId());
      if (!Objects.equals(notification.getNamespaceName(), normalizedNamespace)) {
        deferredResultWrapper.recordNamespaceNameNormalizedResult(notification.getNamespaceName(), normalizedNamespace);
      }
    }

    Multimap<String, String> watchedKeysMap =
        watchKeysUtil.assembleAllWatchKeys(appId, cluster, namespaces, dataCenter);

    Set<String> watchedKeys = Sets.newHashSet(watchedKeysMap.values());

    /**
     * 1、set deferredResult before the check, for avoid more waiting
     * If the check before setting deferredResult,it may receive a notification the next time
     * when method handleMessage is executed between check and set deferredResult.
     */
    deferredResultWrapper
          .onTimeout(() -> logWatchedKeys(watchedKeys, "Apollo.LongPoll.TimeOutKeys"));

    deferredResultWrapper.onCompletion(() -> {
      //unregister all keys
      for (String key : watchedKeys) {
        deferredResults.remove(key, deferredResultWrapper);
      }
      logWatchedKeys(watchedKeys, "Apollo.LongPoll.CompletedKeys");
    });

    //register all keys
    for (String key : watchedKeys) {
      this.deferredResults.put(key, deferredResultWrapper);
    }

    logWatchedKeys(watchedKeys, "Apollo.LongPoll.RegisteredKeys");
    logger.debug("Listening {} from appId: {}, cluster: {}, namespace: {}, datacenter: {}",
        watchedKeys, appId, cluster, namespaces, dataCenter);

    /**
     * 2、check new release
     */
    List<ReleaseMessage> latestReleaseMessages =
        releaseMessageService.findLatestReleaseMessagesGroupByMessages(watchedKeys);

    /**
     * Manually close the entity manager.
     * Since for async request, Spring won't do so until the request is finished,
     * which is unacceptable since we are doing long polling - means the db connection would be hold
     * for a very long time
     */
    entityManagerUtil.closeEntityManager();

    List<ApolloConfigNotification> newNotifications =
        getApolloConfigNotifications(namespaces, clientSideNotifications, watchedKeysMap,
            latestReleaseMessages);

    if (!CollectionUtils.isEmpty(newNotifications)) {
      deferredResultWrapper.setResult(newNotifications);
    }

    return deferredResultWrapper.getResult();
  }
```

## 实现步骤
`ConfigServer.java` 

/publishConfig

