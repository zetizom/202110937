//***************************
// 파일명: RandomGenerator.java
// 작성자: 서형완
// 작성일: 2025-11-10
// 내용: Subject 클래스를 상속받아, 0~49 범위의 난수를 20개 생성하며
//       옵저버에게 알리는 클래스 
//***************************
package hw9_1;

import java.util.Random;

public class RandomGenerator extends Subject  {
  private Random random = new Random();
  private int number;

  // 현재 숫자를 반환합니다.
   
  public int getNumber() {
    return number;
  }

  
   // 49 사이의 난수를 20번 생성하며, 각 단계마다 옵저버에게 알립니다.
  
  public void execute() {
    for (int i=0; i<20; i++) {
      number = random.nextInt(50);
      notifyObservers();
    }
  }
}

