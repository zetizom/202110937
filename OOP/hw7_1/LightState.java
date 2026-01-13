//***************************

// 파일명: LightState.java

// 작성자: 서형완

// 작성일: 2025/10/15

// 내용: ON과 OFF, SLEEPING에서 사용할 두 메서드를 정의했다. 

//***************************



package hw7_1;

interface LightState {
	 public void on_button_pushed(Light light);
	 public void off_button_pushed(Light light);
}
