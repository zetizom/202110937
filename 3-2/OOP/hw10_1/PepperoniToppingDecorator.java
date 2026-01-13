//***************************

// 파일명: PepperoniToppingDecorator.java

// 작성자: 서형완

// 작성일: 2025-11-12

// 내용:  페퍼로니 토핑을 추가하는 데코레이터입니다.

//***************************
package hw10_1;

public class PepperoniToppingDecorator extends ToppingDecorator {
    
    public PepperoniToppingDecorator(AbstractPizza pizza) {
        super(pizza); 
    }

    // 기존 이름에 페퍼로니를 덧붙여 반환합니다.
    @Override
    public String getName() {
        return "페퍼로니 " + pizza.getName();
    }

    // 기존 가격에 페퍼로니 가격을 더해 반환합니다.
    @Override
    public int getPrice() {
        return pizza.getPrice() + ToppingsPrice.PEPPERONI;
    }
}
