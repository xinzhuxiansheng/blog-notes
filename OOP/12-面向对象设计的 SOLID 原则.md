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

