package hw12_1;

//***************************

//파일명: ChicagoCheesePizza.java

//작성자: 서형완

//작성일: 2025/11/25

//내용: 구체적인 시카고 치즈 피자를 정의

//***************************
public class ChicagoCheesePizza extends Pizza {
	public ChicagoCheesePizza() { 
		name = "Chicago Style Deep Dish Cheese Pizza";
		dough = "Extra Thick Crust Dough"; // 아주 두꺼운 도우
		sauce = "Plum Tomato Sauce";       // 플럼 토마토 소스
	}
 
	// 시카고 피자는 사각형으로 자름
	public void cut() {
		System.out.println("Cutting the pizza into square slices");
	}
}
