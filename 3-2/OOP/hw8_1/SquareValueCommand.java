// 파일명: SquareValueCommand.java

// 작성자: 서형완

// 작성일: 2025/11/06

// 내용: 제곱 연산을 직접 수행하는 부분 

//***************************


package hw8_1;

public class SquareValueCommand implements Command {
    private int value;

    public SquareValueCommand(int value) {
        this.value = value;
    }

 // 제곱 수행후 stdout 실행
    @Override
    public void execute() {
        int result = (int) Math.pow(value, 2);
        System.out.println("pow(" + value + ", 2) = " + result);
    }
}
