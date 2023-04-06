## 1. 
For over 20 years I’ve written imperative code where it was easy — and extraordinarily common — to mutate existing data. For instance, once upon a time I had a niece named “Emily Maness”: 

val emily = Person("Emily", "Maness") 

Then one day she got married and her last name became “Wells”, so it seemed logical to update her last name, like this: 

emily.setLastName("Wells").


The way you “update as you copy” in Scala/FP is with the copy method that comes with case classes. First, you start with a case class: 

case class Person (firstName: String, lastName: String) 

Then, when your niece is born, you write code like this: 

val emily1 = Person("Emily", "Maness")
Later, when she gets married and changes her last name, you write this: 

val emily2 = emily1.copy(lastName = "Wells") 

After that line of code, emily2.lastName has the value Wells


>特别需要注意： 对象内部嵌套

“Update as you copy” gets worse with nested objects The “Update as you copy” technique isn’t too hard when you’re working with this simple Person object, but think about this: What happens when you have nested objects, such as a Family that has a Person who has a Seq[CreditCard], and that person wants to add a new credit card, or update an existing one? (This is like an Amazon Prime member who adds a family member to their account, and that person has one or more credit cards.) Or what if the nesting of objects is even deeper? In short, this is a real problem that results in some nasty-looking code, and it gets uglier with each nested layer. Fortunately, other FP developers ran into this problem long before I did, and they came up with ways to make this process easier. I cover this problem and its solution in the “Lens” lesson later in this book.
 