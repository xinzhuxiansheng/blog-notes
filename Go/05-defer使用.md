## defer

在Go语言中，`defer`是一个关键字，用于延迟执行一个函数调用。它用于在当前函数执行完毕之前，设置一个函数调用在函数返回后执行。 

使用`defer`关键字可以确保在函数执行结束时，无论是正常返回还是发生错误，被延迟的函数调用都会被执行。这在一些需要进行资源释放、清理或记录日志等操作时非常有用。   

以下是一些`defer`的用法示例：

```go
func foo() {
    defer fmt.Println("Deferred function")  // 在函数返回前执行
    fmt.Println("Normal function")          // 正常执行的函数调用
}

func bar() {
    defer func() {
        if r := recover(); r != nil {
            fmt.Println("Recovered:", r)    // 在函数返回前执行，并处理恢复
        }
    }()
    // 其他函数调用，可能会引发 panic
}

func main() {
    foo()
    bar()  // 可能会引发 panic，但被 defer 恢复
}
```

在上述示例中，`foo`函数中的`defer`语句会在`foo`函数返回之前执行，即使在`foo`函数内部发生了错误或使用了`return`语句。    

而在`bar`函数中，使用了`defer`和匿名函数结合的方式，来进行恢复（recover）处理。如果在`bar`函数内部发生了 panic，`defer`中的匿名函数将被执行，并打印出 panic 的信息。    

`defer`语句的执行顺序是后进先出的，即最后一个`defer`语句将在最先执行。这可以用于处理资源释放的逆序问题，确保在函数返回前进行必要的清理工作。    

总之，`defer`语句允许我们在函数返回前执行一些必要的操作，无论是正常的返回还是异常情况下的返回。 