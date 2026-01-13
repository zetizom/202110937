//***************************

// 파일명: CommandManager.java

// 작성자: 서형완

// 작성일: 2025/11/06

// 내용: invoker 클래스로써 커맨드 객체들을 저장, 실행을 요청하는 기능을 수행한다.
//***************************


package hw8_1;

import java.util.ArrayList;
import java.util.List;


public class CommandManager {
    
    // Command 객체들을 저장할 리스트
    private List<Command> operations = new ArrayList<>();

    // 실행할 커맨드를 리스트에 추가한다.
    public void addOperation(Command command) {
        operations.add(command);
    }

    // 리스트에 저장된 모든 커맨드를 순서대로 실행한다.
    public void performOperations() {
        for (Command command : operations) {
            command.execute();
        }
    }
}
