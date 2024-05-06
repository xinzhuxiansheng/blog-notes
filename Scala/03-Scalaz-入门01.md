

def sum(xs: List[Int]): Int = xs.foldLeft(0){_ + _}         

foldLeft 与 monoid 两者之间关系     
1.其实位置 
2.二元运算   

需要它的 trait 内容如下：       
```bash 
trait Monoid[A] {
    def mappend(a1: A, a2: A): A
    def mzero: A
}
``` 
通过 sum(List(1,2,3))(intMonoid)， 增加了类型变量T、输入参数M。 在调用 sum()中，会增加1个参数 
eg: 
```bash 
def sum[A](xs: List[A])(m: Monoid[A]): A = xs.foldLeft(m.mzero)(m.mappend)
                                                   //> sum: [A](xs: List[A])(m: scalaz.learn.ex2.Monoid[A])A
sum(List(1,2,3))(intMonoid)                       //> res0: Int = 6
sum(List("Hello,"," how are you"))(stringMonoid)  //> res1: String = Hello, how are you
```