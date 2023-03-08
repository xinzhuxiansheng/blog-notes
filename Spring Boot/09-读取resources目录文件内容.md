## Spring boot读取resources目录文件内容

## 方法一

```java

ClassPathResource classPathResource = new ClassPathResource("clientuseexample/" + fileName);
    InputStream inputStream = classPathResource.getInputStream();
    InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
    BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
    StringBuilder stringBuilder = new StringBuilder();
    String line;
    while ((line = bufferedReader.readLine()) != null) {
        stringBuilder.append(line);
        stringBuilder.append(System.lineSeparator()); // 添加换行符
    }
    inputStream.close();
    String fileContent = stringBuilder.toString();
    return fileContent;

```