//***************************

// 파일명: Client.java

// 작성자: 서형완

// 작성일: 2025/10/01

// 내용: 기본 코드에 페이크프린터 객체를 추가한 후 실행시키는 부분을 추가했다. 

//***************************



package hw5_1;

public class Client {
	public static void main(String[] args) {
		System.out.println("hw5_1: 서형완\n\n");
		Item item1 = new Item("Shampoo", 3000);
		Item item2 = new Item("Cookie", 1000);
		Sale sale = new Sale();
		sale.add(item1);
		sale.add(item2);
		sale.setReceiptPrinter(new FakePrinter());
		sale.printReceipt();
		sale.setReceiptPrinter(new HD108ReceiptPrinter());
		sale.printReceipt();
	}
}