//***************************

// 파일명: CheeseToppingDecorator.java

// 작성자: 서형완

// 작성일: 2025-11-12

// 내용: 치즈 토핑을 추가하는 데코레이터입니다.
//***************************
package hw10_1;

public class CheeseToppingDecorator extends ToppingDecorator {
    
    public CheeseToppingDecorator(AbstractPizza pizza) {
        super(pizza); 
    }

    // 기존 이름에 치즈를 덧붙여 반환합니다.
    @Override
    public String getName() {
        return "치즈 " + pizza.getName();
    }

    // 기존 가격에 치즈 가격을 더해 반환합니다.
    @Override
    public int getPrice() {
        return pizza.getPrice() + ToppingsPrice.CHEESE;
    }
}
