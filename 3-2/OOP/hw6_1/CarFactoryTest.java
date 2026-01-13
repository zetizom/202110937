//***************************

// 파일명: CarFactoryTest.java

// 작성자: 서형완

// 작성일: 2025/10/14

// 내용: 싱글톤 패턴을 넣고 하나 뿐인 객체에서 차를 생성하고, 그 차의 번호를 출력하는 메서드를 넣는다. 
//***************************

package hw6_1;

public class CarFactoryTest{

    public static void main(String[] args) {
    	System.out.println("hw6_1: 서형완\n\n");
		CarFactory factory = CarFactory.getInstance();	// 싱글톤 패턴

		Car myCar = factory.createCar();				// 메서드에서 Car 생성
		Car yourCar = factory.createCar();

		System.out.println(myCar.getCarNum());		// 10001 출력
		System.out.println(yourCar.getCarNum());		// 10002 출력	

  }

}