//***************************

// 파일명: Car.java

// 작성자: 서형완

// 작성일: 2025/10/14

// 내용: carNum을 받은 후 getCarNum으로 차 번호를 반환하는 함수를 만든다.

//***************************


package hw6_1;

public class Car {
    private int carNum;

    Car(int carNum) {
        this.carNum = carNum;
    }
    public int getCarNum() {
        return carNum;
    }
}
