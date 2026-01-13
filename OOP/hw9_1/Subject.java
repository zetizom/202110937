//***************************
// 파일명: Subject.java
// 작성자: 서형완
// 작성일: 2025-11-10
// 내용: 옵저버들을 관리하고 알림을 보내는 주체의 추상 클래스
//***************************
package hw9_1;

import java.util.ArrayList;
import java.util.Iterator;

public abstract class Subject {
  // Observer들을 저장할 리스트 
  private ArrayList<Observer> observers = new ArrayList<>();

  // 옵저버를 리스트에 추가합니다.

  public void addObserver(Observer observer) {
    observers.add(observer);
  }


   // 옵저버를 리스트에서 제거합니다.
  public void deleteObserver(Observer observer) {
    observers.remove(observer);
  }

  // 등록된 모든 옵저버에게 변경 사항을 알립니다.
 
  public void notifyObservers() {
    Iterator<Observer> it = observers.iterator();
    while (it.hasNext()) {
      Observer o = it.next();
      o.update(this);
    }
  }

  
  // 현재 숫자를 반환하는 추상 메소드 
  public abstract int getNumber();

  
  // 숫자 생성 및 알림 로직을 실행하는 추상 메소드   
  public abstract void execute();
}


