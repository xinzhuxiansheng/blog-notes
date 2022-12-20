
## File常用操作

```java
File f = new File("文件的绝对路径且加上文件名"); // eg: /data/files/README.md

// 创建文件并且返回创建结果
f.createNewFile()

// 判断f是否具有读写权限
f.canRead() || f.canWrite()

// 利用RandomAccessFile来对f进行写入
// RandomAccessFile可以自由访问文件的任意位置。
// RandomAccessFile允许自由定位文件记录指针。
// RandomAccessFile只能读写文件而不是流

FileChannel fc = null;
RandomAccessFile raf = null;
try {
    raf = new RandomAccessFile(f, "rw");
    fc = raf.getChannel();
} catch (FileNotFoundException e) {
    e.printStackTrace();
}

// 写入固定字节长度，用于例如 文件类型
ByteBuffer buf = ByteBuffer.wrap(new byte[固定长度]);
try {
    fc.position(0); // 从0位置开始写入 固定长度的buf
    fc.write(buf);
} catch (IOException e) {
    e.printStackTrace();
}

```