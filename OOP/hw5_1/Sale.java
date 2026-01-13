//***************************

// 파일명: Sale.java

// 작성자: 서형완

// 작성일: 2025/10/01

// 내용: add로 item 객체의 값을 items array에 메다는 메서드와 
//		printReceipt으로 items에 담긴 값을 string으로 가져와 출력하는 메서드와
//		setReceiptPrinter로 어떤 프린터를 사용할지 결정하는 부분으로 구성되어있다. 
//***************************


package hw5_1;

import java.util.ArrayList;
import java.util.Iterator;

public class Sale {
	private ArrayList<Item> items = new ArrayList<Item>();
	private ReceiptPrinter printer = new HD108ReceiptPrinter();


	public void printReceipt() {
		Iterator<Item> itr = items.iterator();
		StringBuffer buf = new StringBuffer();
		while (itr.hasNext()) {
			Item item = itr.next();
			buf.append(item.getName());
			buf.append(item.getPrice());
		}
		printer.print(buf);
	}


	public void add(Item item) {
		items.add(item);
	}
	
	public void setReceiptPrinter(ReceiptPrinter printer) {
		this.printer = printer;
	}
}
