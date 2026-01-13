//***************************
// 파일명: Observer.java
// 작성자: 서형완
// 작성일: 2025-11-10
// 내용: Subject의 변경 사항을 통보받는 관찰자의 인터페이스
//***************************
package hw9_1;

public interface Observer {

    // Subject로부터 변경 사항을 알림받을 때 호출되는 메소드     
    public abstract void update(Subject generator);
}


