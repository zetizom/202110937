//***************************

// 파일명: Pizza.java

// 작성자: 서형완

// 작성일: 2025/11/12

// 내용: 생성자로 기본 피자를 정의하고 각 함수들을 정의합니다.

//***************************

package hw10_1;


class Pizza extends AbstractPizza{
    private int size;
    private String name;
    private int price;
    
    public Pizza(int size) {
        this.size = size;
        name = "피자";
        price = 20000;
    }
    public int getSize() { return size; }
    public String getName() { return name; }
    public int getPrice() { return price; }
}
