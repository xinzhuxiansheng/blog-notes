# 面向对象设计的 SOLID 原则     

## 引言 
许多开发者关于面向对象编程的一种常见说法是，面向对象编程的一个主要优势是它能够模拟现实世界。我承认，在讨论传统面向对象概念时，我经常使用这些话。根据罗伯特·马丁（Robert Martin）在至少一次我在YouTube上看到的讲座中提到，认为面向对象更贴近我们的思考方式只是市场营销。相反，他指出，面向对象是通过反转关键依赖来管理依赖关系，以防止代码变得僵硬、脆弱和不可重用。         

例如，在传统的面向对象编程课程中，实践往往是将代码直接模拟到现实生活情境中。例如，如果狗是哺乳动物，那么这种关系显然是继承的不错选择。严格的has-a和is-a试金石测试多年来一直是面向对象思维的一部分。         

然而，正如我们在本书中所见，试图强制继承关系可能会导致设计问题（还记得那只不会叫的狗吗？）。试图将不会叫的狗与会叫的狗，或会飞的鸟与不会飞的鸟分开，是一个明智的继承设计选择吗？这一切都是面向对象营销者安排的吗？好吧；忘掉这些炒作。正如我们在上一章中看到的，`也许专注于严格的has-a和is-a决策并不一定是最佳方法`。也许我们应该更多地关注解耦类。                    

在我之前提到的讲座中，常被称为Uncle Bob的罗伯特·马丁定义了这三个术语来描述不可重用的代码：          
* 僵硬性——当对程序的一个部分进行更改可能会破坏另一个部分时          
* 脆弱性——当在无关的地方出现故障时          
* 不可移植性——当代码无法在其原始环境之外重用时          

SOLID被引入是为了解决这些问题并努力实现这些目标。它定义了五个设计原则，这些原则由罗伯特·马丁引入，旨在“使软件设计更加易于理解、灵活和可维护。”根据罗伯特·马丁的说法，虽然这些原则适用于任何面向对象的设计，但SOLID原则也可以形成敏捷开发或适应性软件开发等方法论的核心哲学。SOLID这个首字母缩略词是由迈克尔·费瑟斯（Michael Feathers）引入的。              

五大 SOLID 原则包括：
- **SRP—单一职责原则**：一个类应该只有一个引起变化的原因。          
- **OCP—开闭原则**：软件实体应当对扩展开放，对修改关闭。            
- **LSP—里氏替换原则**：子类应能在不改变程序正确性的前提下替换基类。            
- **ISP—接口隔离原则**：客户不应依赖它不使用的接口。            
- **DIP—依赖倒置原则**：高层模块不应依赖低层模块，两者都应依赖抽象；抽象不应依赖细节，细节应依赖抽象。          

本章重点介绍这五个原则，并将它们与几十年来一直存在的经典面向对象原则联系起来。我在介绍 SOLID 的目标是通过非常简单的例子来解释这些概念。网上有很多内容，包括一些非常好的 YouTube 视频。这些视频大多面向开发者，并非专门针对编程新手。            

正如我在本书中所有例子的尝试，我的意图不是过于复杂，而是为了教育目的将例子简化到最低公共分母。              

## 面向对象设计的 SOLID 原则        
在第11章“避免依赖和高度耦合的类”中，我们讨论了一些基本概念，为我们讨论五大 SOLID 原则做准备。在这一章中，我们将深入探讨每一个 SOLID 原则的详细内容。所有 SOLID 的定义都来源于 Uncle Bob 的网站：http://butunclebob.com/ArticleS.UncleBob.PrinciplesOfOod。          

### 1) SRP: 单一职责原则        
单一职责原则规定一个类应该只有一个引起变化的原因。程序中的每个类和模块都应该专注于一个任务。因此，不要将因不同原因而变化的方法放在同一个类中。如果类的描述中包含“和”，你可能就违反了 SRP。换句话说，每个模块或类都应对软件提供的功能的一个单一部分负责，并且该职责应完全封装在类中。        

创建形状层次结构是继承的经典示例之一，经常用作教学示例，我在本章（以及整本书）中频繁使用。在这个例子中，Circle 类继承自一个抽象的 Shape 类。Shape 类提供一个名为 calcArea() 的抽象方法作为子类的合约。任何继承自 Shape 的类都必须提供 calcArea() 的自己的实现：                 
```java
abstract class Shape{
 protected String name;
 protected double area;
 public abstract double calcArea();
}
``` 

在这个示例中，我们有一个 Circle 类继承自 Shape，并且按要求提供了 calcArea() 的实现：            
```java
class Circle extends Shape {
    private double radius;
    
    public Circle(double r) {
        radius = r;
    }
    
    public double calcArea() {
        area = 3.14 * (radius * radius);
        return (area);
    }
}
```

>注意：在这个示例中，我们只包含 Circle 类，以便专注于单一职责原则并尽可能保持示例的简单性。         

一个名为 CalculateAreas 的第三个类总结了包含在 Shape 数组中的不同形状的面积。Shape 数组大小不限，可以包含不同的形状，如正方形和三角形。         
```java
class CalculateAreas {
    Shape[] shapes;
    double sumTotal = 0;
    
    public CalculateAreas(Shape[] sh) {
        this.shapes = sh;
    }
    
    public double sumAreas() {
        sumTotal = 0;
        for (int i = 0; i < shapes.length; i++) {
            sumTotal += shapes[i].calcArea();
        }
        return sumTotal;
    }
    
    public void output() {
        System.out.println("Total of all areas = " + sumTotal);
    }
}
```

请注意，CalculateAreas 类还处理应用程序的输出，这是有问题的。面积计算行为和输出行为是耦合的——包含在同一个类中。         

我们可以通过以下名为 TestShape 的测试应用程序来验证这段代码的功能：         
```java
public class TestShape {
 public static void main(String args[]) {
    System.out.println("Hello World!");
    Circle circle = new Circle(1);
    Shape[] shapeArray = new Shape[1];
    shapeArray[0] = circle;
    CalculateAreas ca = new CalculateAreas(shapeArray);
    ca.sumAreas();
    ca.output();
 }
}
```
现在有了测试应用程序，我们可以关注单一职责原则的问题。再次强调，问题在于 CalculateAreas 类，这个类包含了求和各种面积以及输出的行为。                

这里的基本点（和问题）是：如果你想改变 output() 方法的功能，无论求和面积的方法是否变化，都需要改变 CalculateAreas 类。例如，如果在某个时刻我们想以 HTML 而不是简单文本的形式在控制台呈现输出，我们必须重新编译和重新部署求和面积的代码，因为这些职责是耦合的。          

根据单一职责原则，目标是一个方法的改变不会影响另一个方法，从而避免不必要的重新编译。"一个类应该只有一个，且仅有一个，改变的理由——一个单一的职责去改变。"            

为了解决这个问题，我们可以将这两个方法放在不同的类中，一个用于原始的控制台输出，另一个用于新加入的 HTML 输出：          
```java
class CalculateAreas {
 Shape[] shapes;
 double sumTotal=0;
 public CalculateAreas(Shape[] sh){
 this.shapes = sh;
 }
 public double sumAreas() {
 sumTotal=0;
 for (int i=0; i<shapes.length; i++) {
 sumTotal = sumTotal + shapes[i].calcArea();
 }
 return sumTotal;
 }
}
class OutputAreas {
 double areas=0;
 public OutputAreas(double a){
 this.areas = a;
 }
 public void console() {
 System.out.println("Total of all areas = " + areas);
 }
 public void HTML() {
 System.out.println("<HTML>");
 System.out.println("Total of all areas = " + areas);
 System.out.println("</HTML>");
 }
}
```

现在，使用新编写的类，我们可以添加 HTML 输出功能，而不会影响面积求和的代码：    
```java
public class TestShape {
 public static void main(String args[]) {
 System.out.println("Hello World!");
 Circle circle = new Circle(1);
 Shape[] shapeArray = new Shape[1];
 shapeArray[0] = circle;
 CalculateAreas ca = new CalculateAreas(shapeArray);
 CalculateAreas sum = new CalculateAreas(shapeArray);
 OutputAreas oAreas = new OutputAreas(sum.sumAreas());
 oAreas.console(); // 输出到控制台
 oAreas.HTML(); // 输出到HTML
 }
}
``` 

这里的主要点是，你现在可以根据需求将输出发送到不同的目的地。如果你想添加另一种输出可能性，比如 JSON，你可以将其添加到 OutputAreas 类中，而无需更改 CalculateAreas 类。结果是，你可以独立地重新分发 CalculateAreas 类，而不必对其他类做任何事情。

### 2) OCP：开闭原则    
开闭原则指出，你应该能够扩展一个类的行为，而无需修改它。        

让我们再次回顾形状的例子。在以下代码中，我们有一个名为 ShapeCalculator 的类，它接受一个 Rectangle 对象，计算该对象的面积，然后返回该值。这是一个简单的应用程序，但它只适用于矩形。          
```java
class Rectangle {
 protected double length;
 protected double width;
 public Rectangle(double l, double w) {
 length = l;
 width = w;
 }
}

class CalculateAreas {
 private double area;
 public double calcArea(Rectangle r) {
 area = r.length * r.width;
 return area;
 }
}

public class OpenClosed {
 public static void main(String args[]) {
 System.out.println("Hello World");
 Rectangle r = new Rectangle(1,2);
 CalculateAreas ca = new CalculateAreas();
 System.out.println("Area = " + ca.calcArea(r));
 }
}
```
这个应用程序仅适用于矩形的事实带来了一个限制，它说明了开闭原则：如果我们想在 CalculateArea 类中添加一个 Circle（改变它的功能），我们必须修改模块本身。显然，这与开闭原则相矛盾，该原则规定我们不应该因为改变它的功能而需要改变模块。                 

为了符合开闭原则，我们可以重新审视我们熟悉的形状示例，其中创建了一个名为Shape的抽象类，然后所有形状都必须继承自Shape类，该类具有一个名为getArea()的抽象方法。           

到这一步，我们可以添加尽可能多的不同类，而无需更改Shape类本身（例如，一个Circle）。现在我们可以说Shape类是封闭的。
以下代码实现了这个解决方案，用于矩形和圆，并允许创建无限多的形状：              
```java
abstract class Shape {
    public abstract double getArea();
}

class Rectangle extends Shape {
    protected double length;
    protected double width;
    
    public Rectangle(double l, double w) {
        length = l;
        width = w;
    }
    
    public double getArea() {
        return length * width;
    }
}

class Circle extends Shape {
    protected double radius;
    
    public Circle(double r) {
        radius = r;
    }
    
    public double getArea() {
        return radius * radius * 3.14;
    }
}

class CalculateAreas {
    private double area;
    
    public double calcArea(Shape s) {
        area = s.getArea();
        return area;
    }
}
```         

```java
public class OpenClosed {
    public static void main(String args[]) {
        System.out.println("Hello World");
        CalculateAreas ca = new CalculateAreas();
        Rectangle r = new Rectangle(1, 2);
        System.out.println("Area = " + ca.calcArea(r));
        Circle c = new Circle(3);
        System.out.println("Area = " + ca.calcArea(c));
    }
}
```

在这个实现中，当你添加一个新的形状时，CalculateAreas()方法不需要改变。              

您可以扩展您的代码，而无需担心遗留代码。在其核心，开闭原则规定您应该通过子类来扩展代码，而原始类不需要更改。然而，"扩展"这个词在与SOLID相关的讨论中有几个问题。正如我们将在后面详细介绍的那样，如果我们更倾向于组合而不是继承，那么这将如何影响开闭原则？               

在遵循SOLID原则之一时，代码也可能符合其他SOLID原则之一。例如，当设计遵循开闭原则时，代码也可能符合单一职责原则。            

### LSP：里氏替换原则       
里氏替换原则指出，设计必须提供用子类的实例来替换父类的实例的能力。如果父类可以做某事，那么子类也必须能够做到。
让我们来看一些可能看起来合理但却违反了里氏替换原则的代码。在下面的代码中，我们有一个典型的抽象类叫做Shape。然后Rectangle继承自Shape，并覆盖其抽象方法calcArea()。而Square又继承自Rectangle。            

```java
abstract class Shape{
    protected double area;
    public abstract double calcArea();
}

class Rectangle extends Shape {
    private double length;
    private double width;

    public Rectangle(double l, double w) {
        length = l;
        width = w;
    }

    public double calcArea() {
        area = length * width;
        return (area);
    };
}

class Square extends Rectangle {
    public Square(double s) {
        super(s, s);
    }
}

public class LiskovSubstitution {
    public static void main(String args[]) {
        System.out.println("Hello World");
        Rectangle r = new Rectangle(1, 2);
        System.out.println("Area = " + r.calcArea());
        Square s = new Square(2);
        System.out.println("Area = " + s.calcArea());
    }
}
```

到目前为止一切都很顺利：一个矩形是一个形状，所以一切看起来都很好。因为一个正方形是一个矩形，我们仍然很好——或者我们还好吗？
现在我们进入了一个有些哲学性的讨论：一个正方形真的是一个矩形吗？许多人会说是的。然而，虽然正方形很可能是矩形的一种特殊类型，但它确实具有与矩形不同的属性。矩形是一个平行四边形（对边是相等的），正方形也是。然而，正方形也是一个菱形（所有边都是相等的），而矩形不是。因此，存在一些差异。          

当涉及到面向对象设计时，几何学并不是真正的问题。问题在于我们如何构建矩形和正方形。以下是Rectangle类的构造函数： 
```java
public Rectangle(double l, double w){
 length = l;
 width = w;
}
```

Rectangle类的构造函数显然需要两个参数。然而，Square类的构造函数只需要一个参数，尽管其父类Rectangle期望两个参数。            
```java 
class Square extends Rectangle{
 public Square(double s){
 super(s, s);
 }
``` 

实际上，计算面积的功能对于这两个类是微妙不同的。事实上，Square类通过两次传递相同的参数来欺骗Rectangle类。这可能看起来是一个可以接受的解决方案，但实际上可能会使维护代码的人感到困惑，并且可能会在未来造成意想不到的维护问题。这至少是一个不一致的问题，也许是一个值得怀疑的设计决定。当你看到一个构造函数调用另一个构造函数时，最好暂停一下，重新考虑设计——它可能不是一个合适的子类。       
如何解决这个特定的困境呢？简单地说，正方形不能替代长方形，因此它们不应该是同一个子类。因此，它们应该是单独的类。        
```java
 abstract class Shape {
 protected double area;
 public abstract double calcArea();
}
class Rectangle extends Shape {
 private double length;
 private double width;
 public Rectangle(double l, double w) {
 length = l;
 width = w;
 }
 public double calcArea() {
 area = length*width;
 return (area);
 };
}
class Square extends Shape {
 private double side;
 public Square(double s){
 side = s;
 } 
 public double calcArea() {
 area = side*side;
 return (area);
 };
}

public class LiskovSubstitution {
 public static void main(String args[]) {
 System.out.println("Hello World");
 Rectangle r = new Rectangle(1,2);
 System.out.println("Area = " + r.calcArea());
 Square s = new Square(2);
 System.out.println("Area = " + s.calcArea());
 }
}
```     

### 4) 接口隔离原则（Interface Segregation Principle）  
接口隔离原则指出，与其拥有少数较大的接口，不如拥有许多小的接口更好。        

在这个示例中，我们正在创建一个包含哺乳动物多种行为的单一接口，即吃（eat）和发出噪音（makeNoise）：      
```java
interface IMammal {
 public void eat();
 public void makeNoise();
}
class Dog implements IMammal {
 public void eat() {
 System.out.println("Dog is eating");
 }
 public void makeNoise() {
 System.out.println("Dog is making noise");
 }
}
public class MyClass {
 public static void main(String args[]) {
 System.out.println("Hello World");
 Dog fido = new Dog();
 fido.eat();
 fido.makeNoise();
 }
}
```     

与其为哺乳动物创建一个单一接口，我们可以为所有行为创建单独的接口：              
```java
interface IEat {
 public void eat(); 
}
interface IMakeNoise {
 public void makeNoise();
}
class Dog implements IEat, IMakeNoise {
 public void eat() {
 System.out.println("Dog is eating");
 }
 public void makeNoise() {
 System.out.println("Dog is making noise");
 }
}
public class MyClass {
 public static void main(String args[]) {
 System.out.println("Hello World");
 Dog fido = new Dog();
 fido.eat();
 fido.makeNoise();
 }
}
```
实际上，我们正在将行为从哺乳动物类中解耦。因此，我们不是通过继承（实际上是接口）来创建单一的哺乳动物实体，而是转向基于组合的设计，类似于前一章节的策略。        

简而言之，通过使用这种方法，我们可以使用组合构建哺乳动物，而不是被迫利用包含在单一哺乳动物类中的行为。例如，假设有人发现了一种不吃东西而是通过皮肤吸收营养的哺乳动物。如果我们从包含 eat() 行为的单一 Mammal 类继承，新的哺乳动物就不需要这个行为。但是，如果我们将所有行为分别分离到单独的接口中，我们可以按照每个哺乳动物的实际情况构建它。       

### 5) 依赖倒置原则（Dependency Inversion Principle）               
依赖倒置原则指出，代码应该依赖于抽象。通常似乎依赖倒置和依赖注入这两个术语是可以互换使用的；然而，在我们讨论这个原则时，有一些关键术语需要理解：            
- 依赖倒置：反转依赖的原则          
- 依赖注入：反转依赖的行为          
- 构造函数注入：通过构造函数执行依赖注入        
- 参数注入：通过方法的参数执行依赖注入，比如通过 setter 方法        

依赖倒置的目标是将依赖关系与抽象而不是具体的实现耦合在一起。        
虽然显然你必须在某个时候创建具体的东西，但我们努力尽可能将具体对象（使用 new 关键字）创建在尽可能高的层次，比如在 main() 方法中。也许更好的思考方式是重新审视第8章“框架和重用：使用接口和抽象类设计”中提出的讨论，我们在那里讨论了在运行时加载类，以及在第9章“构建对象和面向对象设计”中谈到的解耦和创建具有有限职责的小类。         

同样，依赖倒置原则的一个目标是在运行时而不是在编译时选择对象（你可以在运行时改变程序的行为）。甚至你可以编写新的类而无需重新编译旧的类（实际上，你可以编写新的类并注入它们）。          
这次讨论的基础很大程度上是在第11章“避免依赖和高耦合类”中奠定的。让我们在考虑依赖倒置原则时继续构建。            

### 步骤1：初始示例         
作为此示例的第一步，我们再次回顾本书中经常使用的一个经典面向对象设计示例，即一个哺乳动物类（Mammal），以及继承自哺乳动物的狗（Dog）和猫（Cat）类。哺乳动物类是抽象的，并包含一个名为 makeNoise() 的方法。           
```java
abstract class Mammal {
    public abstract String makeNoise();
}
```
子类，比如 Cat，使用继承来利用 Mammal 的行为 makeNoise()：          
```java
class Cat extends Mammal {
    public String makeNoise() {
        return "Meow";
    }
}
```
然后主应用程序实例化一个 Cat 对象并调用 makeNoise() 方法：              
```java
Mammal cat = new Cat();
System.out.println("Cat says " + cat.makeNoise());
```


以下是第一步的完整应用程序代码：        
```java
public class TestMammal {
    public static void main(String args[]) {
        System.out.println("Hello World\n");
        Mammal cat = new Cat();
        Mammal dog = new Dog();
        System.out.println("Cat says " + cat.makeNoise());
        System.out.println("Dog says " + dog.makeNoise());
    }
}

abstract class Mammal {
    public abstract String makeNoise();
}

class Cat extends Mammal {
    public String makeNoise() {
        return "Meow";
    }
}

class Dog extends Mammal {
    public String makeNoise() {
        return "Bark";
    }
}
```

#### 第2步：分离行为        
上述代码存在一个潜在严重的缺陷：它将哺乳动物和行为（makingNoise）耦合在一起。将哺乳动物的行为与它们本身分离可能有重大优势。为了实现这一点，我们创建一个名为 MakingNoise 的类，可以被所有哺乳动物以及非哺乳动物使用。        

在这种模型中，猫、狗或鸟可以扩展 MakeNoise 类并创建符合其需求的自己的制造噪音行为，例如以下代码片段用于猫：         
```java
abstract class MakingNoise {
    public abstract String makeNoise();
}
```

以下是第二步的完整应用程序代码：                
```java
public class TestMammal {
    public static void main(String args[]) {
        System.out.println("Hello World\n");
        Mammal cat = new Cat();
        Mammal dog = new Dog();
        System.out.println("Cat says " + cat.makeNoise());
        System.out.println("Dog says " + dog.makeNoise());
    }
}

abstract class Mammal {
    public abstract String makeNoise();
}

abstract class MakingNoise {
    public abstract String makeNoise();
}

class Cat extends Mammal {
    CatNoise behavior = new CatNoise();
    
    public String makeNoise() {
        return behavior.makeNoise();
    }
}

class CatNoise extends MakingNoise {
    public String makeNoise() {
        return "Meow";
    }
}
```

通过将 MakingNoise 行为与 Cat 类分离，我们可以在 Cat 类中使用 CatNoise 类来代替硬编码的行为，如下所示。         

以下是第三步的代码示例：        
```java
interface NoiseMaker {
    String makeNoise();
}

class CatNoise implements NoiseMaker {
    public String makeNoise() {
        return "Meow";
    }
}

class DogNoise implements NoiseMaker {
    public String makeNoise() {
        return "Bark";
    }
}

abstract class Mammal {
    private NoiseMaker noiseMaker;

    public Mammal(NoiseMaker noiseMaker) {
        this.noiseMaker = noiseMaker;
    }

    public String makeNoise() {
        return noiseMaker.makeNoise();
    }
}

class Cat extends Mammal {
    public Cat() {
        super(new CatNoise());
    }
}

class Dog extends Mammal {
    public Dog() {
        super(new DogNoise());
    }
}

public class TestMammal {
    public static void main(String args[]) {
        System.out.println("Hello World\n");
        Mammal cat = new Cat();
        Mammal dog = new Dog();
        System.out.println("Cat says " + cat.makeNoise());
        System.out.println("Dog says " + dog.makeNoise());
    }
}
```

在这个示例中，我们使用了接口 `NoiseMaker` 来表示能够发出噪音的实体。`CatNoise` 和 `DogNoise` 类实现了这个接口，分别代表了猫和狗的噪音行为。然后，我们将 `Mammal` 类修改为接收一个 `NoiseMaker` 对象作为构造函数的参数，并在 `makeNoise()` 方法中调用 `NoiseMaker` 对象的 `makeNoise()` 方法来获取噪音。最后，在 `Cat` 和 `Dog` 类的构造函数中，我们分别传入了 `CatNoise` 和 `DogNoise` 对象，实现了通过构造函数进行依赖注入的功能。             

这里的重要转变是放弃特定的哺乳动物（如猫和狗），而是简单地使用如下所示的 Mammal 类：            
```java
class Mammal {
    MakingNoise speaker;

    public Mammal(MakingNoise sb) {
        this.speaker = sb;
    }

    public String makeNoise() {
        return this.speaker.makeNoise();
    }
}
```

现在，我们可以实例化一个 Cat 噪音行为并将其提供给 Animal 类，以创建一个行为类似猫的哺乳动物。事实上，您可以始终通过注入行为来组装一个 Cat，而不是使用传统的类构建技术。             
```java
Mammal cat = new Mammal(new CatNoise());
```

以下是最终步骤的完整应用程序：          
```java
public class TestMammal {
    public static void main(String args[]) {
        System.out.println("Hello World\n");
        Mammal cat = new Mammal(new CatNoise());
        Mammal dog = new Mammal(new DogNoise());
        System.out.println("Cat says " + cat.makeNoise());
        System.out.println("Dog says " + dog.makeNoise());
    }
}

class Mammal {
    MakingNoise speaker;

    public Mammal(MakingNoise sb) {
        this.speaker = sb;
    }

    public String makeNoise() {
        return this.speaker.makeNoise();
    }
}
```

在这个例子中，我们将噪音行为注入到 Mammal 类中，以实现松耦合的设计。这样一来，我们可以更灵活地创建不同类型的哺乳动物，而不需要依赖于具体的类结构。             
```java
interface MakingNoise
{
 public String makeNoise();
}
class CatNoise implements MakingNoise
{
 public String makeNoise()
 {
 return "Meow";
 }
}
class DogNoise implements MakingNoise
{
 public String makeNoise()
 {
 return "Bark";
 }
}
```
在讨论依赖注入时，何时实际实例化对象现在是一个关键考虑因素。尽管目标是通过注入组合对象，但显然你必须在某个时候实例化对象。因此，设计决策围绕着何时进行这种实例化展开。      
如本章前面所述，依赖倒置的目标是与抽象而不是具体的东西耦合，即使显然你必须在某个时候创建具体的东西。因此，一个简单的目标是尽可能在上层（如 main() 方法）创建一个具体对象（通过使用 new 关键字）。当你看到 new 关键字时，总是要评估一下情况。        

## 结论        
这就结束了对 SOLID 原则的讨论。SOLID 原则是当今最具影响力的一组面向对象指导原则之一。有趣的是，研究这些原则与基本的面向对象封装、继承、多态和组合的关系，特别是在组合与继承的辩论中。       

对我来说，从 SOLID 讨论中最有趣的一点是，没有什么是一成不变的。从组合优于继承的讨论中可以明显看出，即使是古老的基本面向对象概念也是可以重新解释的。正如我们所见，一点时间，伴随着各种思想过程的相应演变，对创新是有好处的。             

