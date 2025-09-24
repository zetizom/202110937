//***************************
// 파일명: BankAccout.java
// 작성자: 서형완
// 작성일: 2025/09/24
// 내용: 금액을 입금하고 출금하는 클래스다. reset를 통해 처음 입력값으로 돌아갈 수 있다.
//***************************



package hw1_1;

public class BankAccount implements Resettable {	
	private int account;
	private int init_ac;
	
	public BankAccount(int account) {
		this.account = account;
		init_ac = account;
	}
	
	public void reset() {
		account = init_ac;
	}
	
	public void input(int money) {
		account += money;
	}
	
	public void output(int money) {
		account -= money;
	}
	
	public String toString() {
		String ac = String.valueOf(account);
		return ac;
	}
	
}
