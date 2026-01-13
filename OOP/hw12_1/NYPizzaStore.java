package hw12_1;

//***************************

//파일명: NYPizzaStore.java

//작성자: 서형완

//작성일: 2025/11/25

//내용: 뉴욕 스타일의 피자를 생성하는 구체적인 팩토리

//***************************
public class NYPizzaStore extends PizzaStore {

	@Override
	Pizza createPizza(String item) {
		// 요청된 종류에 따라 적절한 뉴욕 스타일 피자 객체를 반환
		if (item.equals("cheese")) {
			return new NYCheesePizza();
		} else if (item.equals("pepperoni")) {
			return new NYPepperoniPizza();
		} else return null;
	}
}