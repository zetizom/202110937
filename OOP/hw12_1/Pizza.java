package hw12_1;
//***************************

//파일명: Pizza.java

//작성자: 서형완

//작성일: 2025/11/25

//내용: 모든 피자가 공통적으로 가져야 할 속성과 행동을 정의하는 클래스

//***************************

public class Pizza {
	String name;  // 피자 이름
	String dough; // 도우 종류
	String sauce; // 소스 종류

	public String getName() {
		return name;
	}

	// 이름 출력
	public void prepare() {
		System.out.println("Preparing " + name);
	}

	// 굽기 과정
	public void bake() {
		System.out.println("Baking " + name);
	}

	// 자르기 과정
	public void cut() {
		System.out.println("Cutting " + name);
	}

	// 포장 과정
	public void box() {
		System.out.println("Boxing " + name);
	}

	// 피자의 정보를 문자열로 반환
	public String toString() {
		StringBuffer display = new StringBuffer();
		display.append("---- " + name + " ----\n");
		display.append(dough + "\n");
		display.append(sauce + "\n");
		return display.toString();
	}
}