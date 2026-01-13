//***************************

// 파일명: Client.java

// 작성자: 서형완

// 작성일: 2025/10/15

// 내용: 라이트 객체를 생성하고 버튼을 누르면서 상태를 변화 시켰다. 

//***************************

package hw7_1;

public class Client {

	public static void main(String[] args) {
    	System.out.println("hw6_1: 서형완\n\n");

		Light light = new Light();
		light.on_button_pushed();
		light.on_button_pushed();
		light.on_button_pushed();
		light.off_button_pushed();
		light.on_button_pushed();

	}

}
