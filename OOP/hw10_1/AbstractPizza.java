//***************************

// 파일명: AbstractPizza.java

// 작성자: 서형완

// 작성일: 2025-11-12

// 내용: 모든 피자(기본 피자, 토핑된 피자)가 구현해야 할 공통 인터페이스를 정의합니다.

//***************************
package hw10_1;

public abstract class AbstractPizza {
    // 크기를 반환하는 메소드
    public abstract int getSize();
    
    // 이름을 반환하는 메소드
    public abstract String getName();
    
    // 가격을 반환하는 메소드
    public abstract int getPrice();
}
