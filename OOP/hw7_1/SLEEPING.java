//***************************

// 파일명: SLEEPING.java

// 작성자: 서형완

// 작성일: 2025/10/15

// 내용: 버튼을 눌렸을 때, light 객체의 상태에 따라서 OFF하거나 ON 상태로 전환 했다. 

//***************************


package hw7_1;

public class SLEEPING implements LightState{
	public void on_button_pushed(Light light) {
		System.out.println("Light On Back!!");
		light.setState(new ON());
	}
	public void off_button_pushed(Light light) {
		System.out.println("Light Off!");
		light.setState(new OFF());
	}
}
