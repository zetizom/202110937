//***************************

// 파일명: Main.java

// 작성자: 서형완

// 작성일: 2025/11/06

// 내용: main에서 invorker를 생성하고 그 invorker에 객체들을 등록한다. 

//***************************

package hw8_1;

public class Main {

    public static void main(String[] args) {

    	System.out.println("서형완, hw8_1\n");
        CommandManager mgr = new CommandManager();
        
        // Command 객체를 생성하여 invoker에게 전달
        mgr.addOperation(new AddValueCommand(2, 3));
        mgr.addOperation(new SubtractValueCommand(2, 3));
        mgr.addOperation(new SquareValueCommand(3));
        
        //  invoker에게 일괄 실행을 요청한다.
        mgr.performOperations();
    }
}