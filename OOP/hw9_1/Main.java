//***************************
// 파일명: Main.java
// 작성자: 서형완
// 작성일: 2025-11-10
// 내용: 옵저버 패턴 테스트를 위한 메인 클래스.
//       IncrementGenerator를 생성하여 테스트합니다.
//***************************
package hw9_1;

public class Main {
      public static void main(String[] args) {
        System.out.println("hw9_1, 서형완"); 

        Subject generator = new IncrementGenerator(10, 40, 4);
        
        Observer observer1 = new DigitObserver();
        Observer observer2 = new GraphObserver();
        
        // 옵저버 등록
        generator.addObserver(observer1);
        generator.addObserver(observer2);
        
        // 숫자 생성 및 옵저버에게 알림 실행
        generator.execute();
      }
}