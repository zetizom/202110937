//***************************
// 파일명: Driver.java
// 작성자: 서형완
// 작성일: 2025/09/24
// 내용: 메인 메서드로써 BankAccount, Point 객체를 생성하고 기능을 확인한다.
//***************************


package hw1_1;

public class Driver {
	public static void main(String[] args) {
		BankAccount bc = new BankAccount(1000);
		bc.input(500);
		bc.output(200);
		System.out.println(bc.toString());
		
		bc.reset();
		System.out.println(bc.toString());
		
		Point p = new Point(1.5, 2.5);
		p.move(2.1, 2.1);
		p.move(10.0, 10.0);
		System.out.println(p.toString());
		
		p.reset();
		System.out.println(p.toString());
	}
}
