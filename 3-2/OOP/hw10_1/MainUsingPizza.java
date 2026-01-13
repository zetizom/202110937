//***************************

// 파일명: MainUsingPizza.java

// 작성자: 서형완

// 작성일: 2025/11/12

// 내용: list를 통해서 필요한 토핑을 추가하고 토핑에 따라 필요한 문구를 출력합니다.

//***************************


package hw10_1;


import java.util.ArrayList;
import java.util.List;

class MainUsingPizza {
    public static void main(String[] args) {
    	
		System.out.println("hw10_1: 서형완\n\n");
    	ArrayList<String> list = new ArrayList<String>();

        list.add("Pepperoni");
        list.add("Cheese");
    
       // SelectToppingPizza pizza = new SelectToppingPizza(15); // 완성 후 아래 코드 실행
        AbstractPizza pizza = new Pizza(15);

        
        for (String topping : list) {
            // pizza.addTopping(topping);  // 완성 후 아래 코드 실행
        	
            if (topping.equals("Cheese")) {
                pizza = new CheeseToppingDecorator(pizza);
            }
            else if (topping.equals("Pepperoni")) {
                pizza = new PepperoniToppingDecorator(pizza);
            }
            
        	

        }
        System.out.printf("피자: %s, 크기: %d, 가격: %d\n", pizza.getName(), pizza.getSize(), pizza.getPrice());
    }
}
