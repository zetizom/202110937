//***************************

// 파일명: ON.java

// 작성자: 서형완

// 작성일: 2025/10/15

// 내용: 버튼을 눌렸을 때, light 객체의 상태에 따라서 OFF하거나 SLEEPING 상태로 전환 했다. 

//***************************

package hw7_1;

// ON 상태
public class ON implements LightState {
	public void on_button_pushed(Light light) {
		System.out.println("취침등 상태");
		light.setState(new SLEEPING());
	}
	public void off_button_pushed(Light light) {
		System.out.println("Light Off!");
		light.setState(new OFF());
	}
}
