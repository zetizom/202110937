package hw12_1;
//***************************

//파일명: PizzaStore.java

//작성자: 서형완

//작성일: 2025/11/25

//내용: 피자 주문을 처리하는 메서드 피자 생성 -> 준비 -> 굽기 -> 자르기 -> 포장 과정을 수행

//***************************
public abstract class PizzaStore {

 	public Pizza orderPizza(String type) {
		Pizza pizza = createPizza(type); // 구체적인 피자 객체 생성
		
		System.out.println("--- Making a " + pizza.getName() + " ---");
		
		pizza.prepare(); // 준비
		pizza.bake();    // 굽기
		pizza.cut();     // 자르기
		pizza.box();     // 포장

		return pizza;
	}
 	
    
    // 객체 생성을 서브클래스에게 미루는 팩토리 메서드
	abstract Pizza createPizza(String item);
}
