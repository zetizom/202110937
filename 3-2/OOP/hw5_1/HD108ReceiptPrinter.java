//***************************

// 파일명: HD108ReceiptPrinter.java

// 작성자: 서형완

// 작성일: 2025/10/01

// 내용: 실제 프린터의 실행 메서드를 구현했다. 
//***************************


package hw5_1;

public class HD108ReceiptPrinter implements ReceiptPrinter {
	public void print(StringBuffer buf) {
        System.out.println("Real Print");	
		System.out.println(buf);	
	}
}


