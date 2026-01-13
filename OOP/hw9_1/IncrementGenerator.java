//***************************
// 파일명: IncrementGenerator.java
// 작성자: 서형완
// 작성일: 2025-11-10
// 내용: Subject 클래스를 상속받아, 생성자에서 받은 초기값부터 최종값 "미만"까지
//       증가값만큼 숫자를 증가시키며 옵저버에게 알리는 클래스
//***************************
package hw9_1;

public class IncrementGenerator extends Subject {
    private int number;       // 현재 숫자
    private int startValue;   // 초기값
    private int endValue;     // 최종값 (이 값 미만까지 실행)
    private int stepValue;    // 증가값

    // 생성자: 숫자를 생성할 범위를 초기화합니다.     
    public IncrementGenerator(int start, int end, int step) {
        this.startValue = start;
        this.endValue = end;
        this.stepValue = step;
        this.number = start; // 현재 숫자를 초기값으로 설정
    }


    // 현재 숫자를 반환합니다.     
    @Override
    public int getNumber() {
        return number;
    }


    //  숫자를 초기값부터 최종값 미만까지 증가값만큼 증가시키며. 각 단계마다 옵저버에게 알립니다.
    @Override
    public void execute() {
        for (int i = startValue; i < endValue; i += stepValue) {
            this.number = i;    // 현재 숫자를 업데이트
            notifyObservers();  // 옵저버들에게 변경 사항 알림
        }
    }
}