//***************************

// 파일명: CarFactory.java

// 작성자: 서형완

// 작성일: 2025/10/14

// 내용: 우선 유일한 인스턴스를 저장할 정적변수와 고유번호를 관리할 변수를 저장한다.
// 그 후 메서드로만 생성자를 생성할 수 있고 멀티스레드문제를 해결할 수 있도록 싱크로나이즈 getInstance 메서드를 만든다.
// 마지막으로 자동차 생성 메서드를 만들어서 자동차를 만들고 그 후 고유번호를 증가시키도록 한다. 

//***************************



package hw6_1;

public class CarFactory {
   private static CarFactory instance;
        private static int carNumber = 10001;

    private CarFactory() {
    }

    public static synchronized CarFactory getInstance() {
        if (instance == null) {
            instance = new CarFactory();
        }
        return instance;
    }

    public Car createCar() {
        Car car = new Car(carNumber);
        carNumber++; 
        return car;
    }
}
