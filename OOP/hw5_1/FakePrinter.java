//***************************

// 파일명: FakePrinter.java

// 작성자: 서형완

// 작성일: 2025/10/01

// 내용: 가짜 프린터의 실행 메서드를 구현했다. 
//***************************



package hw5_1;

public class FakePrinter implements ReceiptPrinter{
	public void print(StringBuffer buf) {
		System.out.println("fake Print");	
		System.out.println(buf);
	}
}
