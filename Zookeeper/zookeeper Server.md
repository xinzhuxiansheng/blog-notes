**`正文`**

## CreateMode

### 

PERSISTENT



```java

@InterfaceAudience.Public
public enum CreateMode {
    
    /**
     * The znode will not be automatically deleted upon client's disconnect.
     */
    PERSISTENT (0, false, false),
    /**
    * The znode will not be automatically deleted upon client's disconnect,
    * and its name will be appended with a monotonically increasing number.
    */
    PERSISTENT_SEQUENTIAL (2, false, true),
    /**
     * The znode will be deleted upon the client's disconnect.
     */
    EPHEMERAL (1, true, false),
    /**
     * The znode will be deleted upon the client's disconnect, and its name
     * will be appended with a monotonically increasing number.
     */
    EPHEMERAL_SEQUENTIAL (3, true, true);
```