//***************************

// 파일명: ToppingsDecorator.java

// 작성자: 서형완

// 작성일: 2025/11/12

// 내용: AbstractPizza을 상속받으면서, 하위 토핑피자들의 부모입니다.

//***************************

package hw10_1;

public abstract class ToppingDecorator extends AbstractPizza {
    protected AbstractPizza pizza; 

    // 생성자를 통해 장식할 피자 객체를 받습니다.
    public ToppingDecorator(AbstractPizza pizza) {
        this.pizza = pizza;
    }

    // 사이즈는 토핑과 상관 없으니 그대로 둡니다.
    @Override
    public int getSize() {
        return pizza.getSize();
    }

    // getName과 getPrice는 하위 객체에서 구체적으로 구현해야 합니다.
    @Override
    public abstract String getName();

    @Override
    public abstract int getPrice();
}
