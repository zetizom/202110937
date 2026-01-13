//***************************
// 파일명: GraphObserver.java
// 작성자: 서형완
// 작성일: 2025-11-10
// 내용: Observer 인터페이스를 구현한 구체적인 관찰자.
//       관찰한 수를 '*' 문자를 사용한 '그래프'로 콘솔에 출력합니다.
//***************************
package hw9_1;

public class GraphObserver implements Observer {
      /**
        Subject의 상태가 업데이트되었을 때 호출됩니다.
        generator로부터 숫자를 받아 '*'의 개수로 시각화하여 콘솔에 출력합니다.
      */ 
      public void update(Subject generator) {
        System.out.print("GraphObserver:");
        int count = generator.getNumber();
        for (int i=0; i<count; i++) {
          System.out.print("*");
        }
        System.out.println("");
      }
    }

