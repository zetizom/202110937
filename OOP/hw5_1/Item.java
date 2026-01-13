//***************************

// 파일명: Item.java

// 작성자: 서형완

// 작성일: 2025/10/01

// 내용: 이름과 가격이 저장하는 클레스를 구현하는 부분이다.
//***************************


package hw5_1;

public class Item {
	private String name;
	private int price;


	public String getName() {
		return name;
	}


	public int getPrice() {
		return price;
	}


	public Item(String name, int price) {
		this.name = name;
		this.price = price;
	}



}
