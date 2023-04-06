## 集合转Map

假设存在 Student类, students集合包含多个学生类
```java

List<Student> students = Arrays.asList(
        new Student(1, "Alice", 90),
        new Student(2, "Bob", 85),
        new Student(3, "Charlie", 95)
);


```


```java
public class Student {
    private int id;
    private String name;
    private int score;

    public Student(int id, String name, int score) {
        this.id = id;
        this.name = name;
        this.score = score;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public int getScore() {
        return score;
    }
}

```

### 构造

使用了 Collectors.toMap() 方法将Stream转换为Map，第一个参数是键的映射函数，这里使用 Student::getId，表示使用每个学生对象的ID作为键；第二个参数是值的映射函数，这里使用 Function.identity()，表示使用学生对象本身作为值。
```java
Map<Integer, Student> studentMap = students.stream()
        .collect(Collectors.toMap(Student::getId, Function.identity()));

```

使用了 Collectors.toMap() 方法将Stream转换为Map，第一个参数是键的映射函数，这里使用 Student::getId，表示使用每个学生对象的ID作为键；第二个参数是值的映射函数，这里使用 Student::getName，表示使用每个学生对象的名称作为值。
```java
Map<Integer, String> nameMap = students.stream()
        .collect(Collectors.toMap(Student::getId, Student::getName));

```
