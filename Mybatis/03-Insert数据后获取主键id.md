## Insert数据后获取主键id

>version: 3.5.14-SNAPSHOT

在Mybatis使用过程中，当使用insert数据到Table后，希望自增的主键id能自动映射到实体类的形参上，这样免去我们再去查询一遍id主键或者在主动给实体类setId(),`下面介绍下insert数据后获取主键id在mybatis中如何使用？`

**UserMapper.addUser**
```java
@Insert("INSERT INTO user (name, age) VALUES (#{name}, #{age})")
@Options(useGeneratedKeys = true, keyProperty = "id", keyColumn = "id")
int addUser(User user);
```

添加`@Options(useGeneratedKeys = true, keyProperty = "id", keyColumn = "id")`即可，现在我们回到Mybatis的`MapperMethod.execute()`方法，所有的mapper的动态代理类最终都会调用execute()方法，现在我们看下insert数据时的执行过程：
```java
 public Object execute(SqlSession sqlSession, Object[] args) {
    Object result;
    switch (command.getType()) {
      case INSERT: {
        Object param = method.convertArgsToSqlCommandParam(args);
        result = rowCountResult(sqlSession.insert(command.getName(), param));
        break;
      }
      case UPDATE: {
        Object param = method.convertArgsToSqlCommandParam(args);
        result = rowCountResult(sqlSession.update(command.getName(), param));
        break;
      }
      case DELETE: {
        Object param = method.convertArgsToSqlCommandParam(args);
        result = rowCountResult(sqlSession.delete(command.getName(), param));
        break;
      }
      case SELECT:
        if (method.returnsVoid() && method.hasResultHandler()) {
          executeWithResultHandler(sqlSession, args);
          result = null;
        } else if (method.returnsMany()) {
          result = executeForMany(sqlSession, args);
        } else if (method.returnsMap()) {
          result = executeForMap(sqlSession, args);
        } else if (method.returnsCursor()) {
          result = executeForCursor(sqlSession, args);
        } else {
          Object param = method.convertArgsToSqlCommandParam(args);
          result = sqlSession.selectOne(command.getName(), param);
          if (method.returnsOptional() && (result == null || !method.getReturnType().equals(result.getClass()))) {
            result = Optional.ofNullable(result);
          }
        }
        break;
      case FLUSH:
        result = sqlSession.flushStatements();
        break;
      default:
        throw new BindingException("Unknown execution method for: " + command.getName());
    }
    if (result == null && method.getReturnType().isPrimitive() && !method.returnsVoid()) {
      throw new BindingException("Mapper method '" + command.getName()
          + "' attempted to return null from a method with a primitive return type (" + method.getReturnType() + ").");
    }
    return result;
  }
```

通过调试sqlSession.insert(...),会一直跟进到`Jdbc3KeyGenerator`的processBatch()方法，此时stmt.getGeneratedKeys()就是获取id的方法,代码如下
```java
try (ResultSet rs = stmt.getGeneratedKeys()) {
    final ResultSetMetaData rsmd = rs.getMetaData();
    final Configuration configuration = ms.getConfiguration();
    if (rsmd.getColumnCount() < keyProperties.length) {
    // Error?
    } else {
    assignKeys(configuration, rs, rsmd, keyProperties, parameter);
    }
} catch (Exception e) {
    throw new ExecutorException("Error getting generated key or setting result to parameter object. Cause: " + e, e);
}
```

stmt是java.sql.Statement类型参数，也可以用jdbc非常直观的实现出来：  
```java
private static void getGeneratedKeysTest() {
    try {
        Class.forName("com.mysql.cj.jdbc.Driver");
        Connection conn = DriverManager.getConnection("jdbc:mysql://localhost:3306/yzhou_test", "root", "12345678");

        String sql = "INSERT INTO user (name, age) VALUES (?, ?)";
        PreparedStatement pstmt = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS);
        pstmt.setString(1, "JDBCTest");
        pstmt.setString(2, "30");
        pstmt.executeUpdate();

        ResultSet generatedKeys = pstmt.getGeneratedKeys();
        if (generatedKeys.next()) {
            long id = generatedKeys.getLong(1);
            System.out.println("Generated key: " + id);
        }
        pstmt.close();
        conn.close();
    } catch (Exception e) {
        e.printStackTrace();
    }
}
```

pstmt.getGeneratedKeys()最终会调用StatementImpl.getGeneratedKeysInternal()方法，它使用了一个 synchronized 块来确保线程安全。它获取了数据库连接的互斥锁，确保在执行此方法时，不会有其他线程修改连接状态。从会话中获取数据库字符集设置的元数据编码（encoding）和元数据排序规则（collationIndex）。    
* 创建一个字段数组（fields），包含一个类型为 MysqlType.BIGINT_UNSIGNED 的字段。这个字段将用于存储生成的主键值。
* 创建一个名为 rowSet 的 ArrayList<Row>，用于存储生成的主键值。
* 调用 `getLastInsertID()` 获取最后一个插入记录的自增主键值（beginAt）。
* 检查结果集（this.results）中是否包含有关生成的主键数量的信息（numKeys）。如果有，并从结果集的服务器信息中获取生成的记录数量。
* 使用一个循环将所有生成的主键值添加到 rowSet 中。循环次数等于生成的主键数量（numKeys）。在循环内部，将每个生成的主键值转换为字节数组并创建一个新的 ByteArrayRow 对象，将其添加到 rowSet 中。在每次迭代中，beginAt 值会根据 connection.getAutoIncrementIncrement() 增加。最后，使用 rowSet 和 fields 创建一个 ResultSetImpl 对象（gkRs）。这个结果集对象将包含所有生成的主键值。返回此结果集对象。

**StatementImpl.getGeneratedKeysInternal()**

```java
protected ResultSetInternalMethods getGeneratedKeysInternal(long numKeys) throws SQLException {
    synchronized (checkClosed().getConnectionMutex()) {
        String encoding = this.session.getServerSession().getCharsetSettings().getMetadataEncoding();
        int collationIndex = this.session.getServerSession().getCharsetSettings().getMetadataCollationIndex();
        Field[] fields = new Field[1];
        fields[0] = new Field("", "GENERATED_KEY", collationIndex, encoding, MysqlType.BIGINT_UNSIGNED, 20);

        ArrayList<Row> rowSet = new ArrayList<>();

        long beginAt = getLastInsertID();

        if (this.results != null) {
            String serverInfo = this.results.getServerInfo();

            //
            // Only parse server info messages for 'REPLACE' queries
            //
            if ((numKeys > 0) && (this.results.getFirstCharOfQuery() == 'R') && (serverInfo != null) && (serverInfo.length() > 0)) {
                numKeys = getRecordCountFromInfo(serverInfo);
            }

            if ((beginAt != 0 /* BIGINT UNSIGNED can wrap the protocol representation */) && (numKeys > 0)) {
                for (int i = 0; i < numKeys; i++) {
                    byte[][] row = new byte[1][];
                    if (beginAt > 0) {
                        row[0] = StringUtils.getBytes(Long.toString(beginAt));
                    } else {
                        byte[] asBytes = new byte[8];
                        asBytes[7] = (byte) (beginAt & 0xff);
                        asBytes[6] = (byte) (beginAt >>> 8);
                        asBytes[5] = (byte) (beginAt >>> 16);
                        asBytes[4] = (byte) (beginAt >>> 24);
                        asBytes[3] = (byte) (beginAt >>> 32);
                        asBytes[2] = (byte) (beginAt >>> 40);
                        asBytes[1] = (byte) (beginAt >>> 48);
                        asBytes[0] = (byte) (beginAt >>> 56);

                        BigInteger val = new BigInteger(1, asBytes);

                        row[0] = val.toString().getBytes();
                    }
                    rowSet.add(new ByteArrayRow(row, getExceptionInterceptor()));
                    beginAt += this.connection.getAutoIncrementIncrement();
                }
            }
        }

        ResultSetImpl gkRs = this.resultSetFactory.createFromResultsetRows(ResultSet.CONCUR_READ_ONLY, ResultSet.TYPE_SCROLL_INSENSITIVE,
                new ResultsetRowsStatic(rowSet, new DefaultColumnDefinition(fields)));

        return gkRs;
    }
}
```

`LAST_INSERT_ID()` 是 MySQL 中的一个函数，用于返回最后一次插入操作所生成的自增主键值。这个值是会话特定的，也就是说，每个数据库连接都有自己的 LAST_INSERT_ID() 值，互不影响。

```
官方文档中的解释如下：

    With no argument, LAST_INSERT_ID() returns a BIGINT UNSIGNED (64-bit) value representing the first automatically generated value successfully inserted for an AUTO_INCREMENT column as a result of the most recently executed INSERT statement.

    The value of LAST_INSERT_ID() is not changed if you set the AUTO_INCREMENT column of the most recently inserted row with an explicit value.

您可以通过以下链接查看关于 LAST_INSERT_ID() 的 MySQL 官方文档：

MySQL 5.7 Reference Manual: 12.16.3.3. LAST_INSERT_ID()

MySQL 8.0 Reference Manual: 12.16.3.3. LAST_INSERT_ID()

```


**StatementImpl.getLastInsertID()**

```java
/**
   * getLastInsertID returns the value of the auto_incremented key after an
   * executeQuery() or excute() call.
   * 
   * <p>
   * This gets around the un-threadsafe behavior of "select LAST_INSERT_ID()" which is tied to the Connection that created this Statement, and therefore could
   * have had many INSERTS performed before one gets a chance to call "select LAST_INSERT_ID()".
   * </p>
   * 
   * @return the last update ID.
   */
  public long getLastInsertID() {
      synchronized (checkClosed().getConnectionMutex()) {
          return this.lastInsertId;
      }
  }
```
