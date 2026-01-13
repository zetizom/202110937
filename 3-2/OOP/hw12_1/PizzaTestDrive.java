//***************************

// 파일명: PizzaTestDrive.java

// 작성자: 서형완

// 작성일: 2025/11/25

// 내용: list를 통해서 필요한 토핑을 추가하고 토핑에 따라 필요한 문구를 출력

//***************************


package hw12_1;


public class PizzaTestDrive {

	public static void main(String[] args) {
		
		System.out.println("hw12_1: 서형완\n\n");
		
		// 각 스타일의 구체적인 Store(Factory) 생성
		PizzaStore nyStore = new NYPizzaStore();
		PizzaStore chicagoStore = new ChicagoPizzaStore();
 
		// NY 스타일 주문
		Pizza pizza = nyStore.orderPizza("cheese");
		System.out.println("We ordered a " + pizza.getName() + "\n");
 
		// Chicago 스타일 주문
		pizza = chicagoStore.orderPizza("cheese");
		System.out.println("We ordered a " + pizza.getName() + "\n");

		// NY 페페로니 주문
		pizza = nyStore.orderPizza("pepperoni");
		System.out.println("We ordered a " + pizza.getName() + "\n");
 
		// Chicago 페페로니 주문
		pizza = chicagoStore.orderPizza("pepperoni");
		System.out.println("We ordered a " + pizza.getName() + "\n");
	}
}