**`正文`**

[TOC]

### 先认识下 Iterator ，Iterable
```java
//Iterator
public interface Iterator<E> {
    boolean hasNext();
    E next();
    default void remove(){throw new UnsupportedOperationExcetion("remove");}
    default void forEachRemaining(Consumer<? super E>action){
        Object.requireNonNull(action);
        while(hasNext())
            action.accept(next());
    }
}

//Iterable
public interface Iterable<T>{
    Iterator<T> iterator();

    //default 接口默认方法
    default void forEach(Consumer<? super T> action){
        Objects.requireNonNull(action);
        for(T t: this){
            action.accept(t);
        }
    }
    default Spliterator<T> spliterator() {
        return Spliterators.spliteratorUnknownSize(iterator(), 0);
    }
}
```

Iterator:
Iterable:


