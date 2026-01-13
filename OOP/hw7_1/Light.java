//***************************

// 파일명: Light.java

// 작성자: 서형완

// 작성일: 2025/10/15

// 내용: 라이트 객체를 정의하고 버튼을 on하거나 off함에 따라서 상태를 변화시키는 메서드를 만들었다.  

//***************************


package hw7_1;

public class Light {
	 private LightState state;
	 public Light() {
		 state = new OFF(); // 기본적으로는 off로 시작 
	 }
	 public void setState(LightState state) { // light 상태 설정 
		 this.state = state;  
	 }
	 
	 // light 상태에 따라서 같은 구조이지만 다른 메서드가 실행 
	 public void on_button_pushed() {
		 state.on_button_pushed(this);
	 }
	 public void off_button_pushed() {
		 state.off_button_pushed(this);
	 }
}
