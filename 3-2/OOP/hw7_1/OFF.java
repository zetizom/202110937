//***************************

// 파일명: OFF.java

// 작성자: 서형완

// 작성일: 2025/10/15

// 내용: 버튼을 눌렸을 때, light 객체의 상태에 따라서 ON하거나 번화를 주지 않았다. 

//***************************



package hw7_1;

//OFF 상태
public class OFF implements LightState {
	public void on_button_pushed(Light light) { 
		System.out.println("Light On!!");
		light.setState(new ON());
	}
	public void off_button_pushed(Light light) {
		System.out.println("반응 없음");
		light.setState(new OFF());
	}
}