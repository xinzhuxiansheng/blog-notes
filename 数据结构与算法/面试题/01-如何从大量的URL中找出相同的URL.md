## 如何在大量的URL中找出相同的URL

## 题目描述
给定 a、b 两个文件，各存放 50 亿个 URL，每个 URL 各占 64B，内存限制是 4G。请找出 a、b 两个文件共同的 URL。

## 解答思路
每个 URL 占 64B，那么 50 亿个 URL占用的空间大小约为 320GB。
5, 000, 000, 000  64B ≈ 5GB  64 = 320GB
由于内存大小只有 4G，因此，我们不可能一次性把所有 URL 加载到内存中处理。对于这种类型的题目，一般采用分治策略，即：把一个文件中的 URL 按照某个特征划分为多个小文件，使得每个小文件大小不超过 4G，这样就可以把这个小文件读到内存中进行处理了。

## 思路如下：
首先遍历文件 a，对遍历到的 URL 求 hash(URL) % 1000 ，根据计算结果把遍历到的 URL 存储到 a0, a1, a2, ..., a999，这样每个大小约为 300MB。使用同样的方法遍历文件 b，把文件 b 中的 URL 分别存储到文件 b0, b1, b2, ..., b999 中。这样处理过后，所有可能相同的 URL 都在对应的小文件中，即 a0 对应 b0, ..., a999 对应 b999，不对应的小文件不可能有相同的 URL。那么接下来，我们只需要求出这 1000 对小文件中相同的 URL 就好了。
接着遍历 ai( i∈[0,999] )，把 URL 存储到一个 HashSet 集合中。然后遍历 bi 中每个 URL，看在 HashSet 集合中是否存在，若存在，说明这就是共同的 URL，可以把这个 URL 保存到一个单独的文件中。

## 方法总结
分而治之，进行哈希取余；
对每个子文件进行 HashSet 统计。


## yzhou 理解思路：
在Java中，如果想在读取文件时减少内存消耗，可以使用流（Stream）和缓冲区， 那么利用缓冲区'BufferedReader', 将大文件拆分成1000个子文件
```java
public static void splitAndHashUrls(String inputFileName, String outputFilePrefix) throws IOException {
        Map<Integer, BufferedWriter> writers = new HashMap<>();
        try (BufferedReader reader = new BufferedReader(new FileReader(inputFileName))) {
            String line;
            while ((line = reader.readLine()) != null) {
                int hashCode = line.hashCode() % NUM_SUB_FILES;
                BufferedWriter writer = writers.computeIfAbsent(hashCode, k -> {
                    try {
                        return new BufferedWriter(new FileWriter(outputFilePrefix + k + ".txt", true));
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                });
                writer.write(line);
                writer.newLine();
            }
        }

        // 关闭所有打开的BufferedWriter
        for (BufferedWriter writer : writers.values()) {
            writer.close();
        }
    }
```

将a，b 两个大文件都按照 ` line.hashCode() % NUM_SUB_FILES;` 拆分成 a0~a999, b0~b999,  当然这里用hashCode()方法做数据打散处理，但也保不齐会出现倾斜问题。 先忽略倾斜问题，也请注意，子文件存的仍然是 url

此时按照hashcode相同逻辑对比，当然这里也存在碰撞问题，但毕竟概念低

将a，b子文件，按照 a0对比b0, a1对比b1策略，直到 a999对比b999。

```java
    public static void findCommonUrls(String fileAPrefix, String fileBPrefix) throws IOException {
        for (int i = 0; i < NUM_SUB_FILES; i++) {
            Set<String> urlSetA = loadUrlsIntoSet(fileAPrefix + i + ".txt");
            Set<String> urlSetB = loadUrlsIntoSet(fileBPrefix + i + ".txt");

            urlSetA.retainAll(urlSetB);

            System.out.println("共同的URL（子文件" + i + "）：");
            for (String url : urlSetA) {
                System.out.println(url);
            }
        }
    }

    public static Set<String> loadUrlsIntoSet(String fileName) throws IOException {
        Set<String> urlSet = new HashSet<>();
        try (BufferedReader reader = new BufferedReader(new FileReader(fileName))) {
            String line;
            while ((line = reader.readLine()) != null) {
                urlSet.add(line);
            }
    }
    return urlSet;
    }
```

### 完整代码

```java
public class FindCommonUrls {
    public static final int NUM_SUB_FILES = 1000;

    public static void main(String[] args) throws IOException {
        splitAndHashUrls("file_a.txt", "hashed_a_");
        splitAndHashUrls("file_b.txt", "hashed_b_");

        findCommonUrls("hashed_a_", "hashed_b_");
    }

    public static void splitAndHashUrls(String inputFileName, String outputFilePrefix) throws IOException {
        Map<Integer, BufferedWriter> writers = new HashMap<>();
        try (BufferedReader reader = new BufferedReader(new FileReader(inputFileName))) {
            String line;
            while ((line = reader.readLine()) != null) {
                int hashCode = line.hashCode() % NUM_SUB_FILES;
                BufferedWriter writer = writers.computeIfAbsent(hashCode, k -> {
                    try {
                        return new BufferedWriter(new FileWriter(outputFilePrefix + k + ".txt", true));
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                });
                writer.write(line);
                writer.newLine();
            }
        }

        // 关闭所有打开的BufferedWriter
        for (BufferedWriter writer : writers.values()) {
            writer.close();
        }
    }

    public static void findCommonUrls(String fileAPrefix, String fileBPrefix) throws IOException {
        for (int i = 0; i < NUM_SUB_FILES; i++) {
            Set<String> urlSetA = loadUrlsIntoSet(fileAPrefix + i + ".txt");
            Set<String> urlSetB = loadUrlsIntoSet(fileBPrefix + i + ".txt");

            urlSetA.retainAll(urlSetB);

            System.out.println("共同的URL（子文件" + i + "）：");
            for (String url : urlSetA) {
                System.out.println(url);
            }
        }
    }

    public static Set<String> loadUrlsIntoSet(String fileName) throws IOException {
        Set<String> urlSet = new HashSet<>();
        try (BufferedReader reader = new BufferedReader(new FileReader(fileName))) {
            String line;
            while ((line = reader.readLine()) != null) {
                urlSet.add(line);
            }
        }
        return urlSet;
    }
}
```
