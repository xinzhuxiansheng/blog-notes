## Stream排序和分页

```java
// 排序
List<Student newList = new ArrayList<>(10);
// 升序
list.stream().sorted((v1,v2)->v1.getId().compareTo(
	v2.getId()
)).collect(Collectors.toList());

// 降序
list.stream().sorted((v1,v2)->v2getId().compareTo(
	v1.getId()
)).collect(Collectors.toList());

//根据子对象id，升序排序，Student对象中还有一个Boy的对象属性
list.stream().sorted((v1,v2)->v1.getBoy().getbId().compareTo(
	v2.getBoy().getbId()
	)).collect(Collectors.toList());



// 分页
// skip：跳过n个元素，limit裁剪大小，currentPage当前页，pageSize当前页大小
list.stream().skip((currentPage-1)*pageSize).limit(pageSize).
									collect(Collectors.toList());
```