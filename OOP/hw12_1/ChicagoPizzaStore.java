package hw12_1;

//***************************

//파일명: ChicagoPizzaStore.java

//작성자: 서형완

//작성일: 2025/11/25

//내용: 시카고 스타일의 피자를 생성하는 구체적인 팩토리

//***************************

public class ChicagoPizzaStore extends PizzaStore {

	@Override
	Pizza createPizza(String item) {
		// 요청된 종류에 따라 적절한 시카고 스타일 피자 객체를 반환
		if (item.equals("cheese")) {
			return new ChicagoCheesePizza();
		} else if (item.equals("pepperoni")) {
			return new ChicagoPepperoniPizza();
		} else return null;
	}
}
