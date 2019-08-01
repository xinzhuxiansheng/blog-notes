**`正文`**
[TOC]

```java
InputStream in = Thread.currentThread().getContextClassLoader().getResourceAsStream(idcEnum.getFileName());

InputStream in = XXXX.class.getClassLoader().getResourceAsStream(idcEnum.getFileName());
```




