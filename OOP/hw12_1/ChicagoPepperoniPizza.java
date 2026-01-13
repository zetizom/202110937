package hw12_1;

//***************************

//파일명: ChicagoPepperoniPizza.java

//작성자: 서형완

//작성일: 2025/11/25

//내용: 구체적인 시카고 패퍼로니 피자를 정의

//***************************
public class ChicagoPepperoniPizza extends Pizza {
	public ChicagoPepperoniPizza() {
		name = "Chicago Style Pepperoni Pizza";
		dough = "Extra Thick Crust Dough";
		sauce = "Plum Tomato Sauce";
 
	}
 
	// 시카고 피자는 사각형으로 자름
	public void cut() {
		System.out.println("Cutting the pizza into square slices");
	}
}
