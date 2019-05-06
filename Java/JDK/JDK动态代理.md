**`正文`**
[TOC]

## JDK动态代理
Java动态代理位于java.lang.reflect包下，一般主要涉及到以下两个类：
* Interface InvocationHandler
    该接口中定义了一个方法：public Object invoke(Object obj,Method method,Object[] args),在使用时，第一个参数obj一般是指代理类，method是被代理的方法，args为该方法的参数数组。这个抽象方法在代理中动态实现
* Proxy
    该类即为动态代理类，static Object newProxyInstance(ClassLoader loader,Class[] interfaces,InvocationHandler h),返回代理类的一个实例，返回后的代理类可以被代理类使用

>JDK动态代理的一般实现步骤如下:
    1.创建一个实现InvocationHandler接口的类，它必须实现invoke方法。
    2.创建被代理的类以及接口
    3.调用Proxy的静态方法newProxyInstance，创建一个代理类
    4.通过代理调用方法

```java
public interface Moveable {
	void move();
}

public class Car implements Moveable {
 
	@Override
	public void move() {
		//实现开车
		try {
			Thread.sleep(new Random().nextInt(1000));
			System.out.println("汽车行驶中....");
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
 
}

public class TimeHandler implements InvocationHandler {
 
	public TimeHandler(Object target) {
		super();
		this.target = target;
	}
 
	private Object target;
	
	/*
	 * 参数：
	 * proxy  被代理对象
	 * method  被代理对象的方法
	 * args 方法的参数
	 * 
	 * 返回值：
	 * Object  方法的返回值
	 * */
	@Override
	public Object invoke(Object proxy, Method method, Object[] args)
			throws Throwable {
		long starttime = System.currentTimeMillis();
		System.out.println("汽车开始行驶....");
		method.invoke(target);
		long endtime = System.currentTimeMillis();
		System.out.println("汽车结束行驶....  汽车行驶时间：" 
				+ (endtime - starttime) + "毫秒！");
		return null;
	}
 
}

public class Test {
 
	/**
	 * JDK动态代理测试类
	 */
	public static void main(String[] args) {
		Car car = new Car();
		InvocationHandler h = new TimeHandler(car);
		Class<?> cls = car.getClass();
		/**
		 * loader  类加载器
		 * interfaces  实现接口
		 * h InvocationHandler
		 */
		Moveable m = (Moveable)Proxy.newProxyInstance(cls.getClassLoader(),cls.getInterfaces(), h);
		m.move();
	}
 
}

```