
// 파일명: SubtractValueCommand.java

// 작성자: 서형완

// 작성일: 2025/11/06

// 내용: 뺄샘 연산을 직접 수행하는 부분 

//***************************

package hw8_1;

public class SubtractValueCommand implements Command {
    private int a;
    private int b;

    public SubtractValueCommand(int a, int b) {
        this.a = a;
        this.b = b;
    }

 // 뺄샘 수행후 stdout 실행
    @Override
    public void execute() {
        int result = a - b;
        System.out.println(a + " - " + b + " = " + result);
    }
}