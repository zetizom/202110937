package st_1;

public class Driver {
	public static void main(String[] args) {
				
		
		// playable 인터페이스를 구현한 클래스는 Playable 배열에 담을 수 있음
		// 리스트는 안됨 
		Playable[] medialist = new Playable[3];
		
		medialist[0] = new Music("dd","nj");
		medialist[1] = new Movie("aa","sw");
		medialist[2] = new Movie("bb","at");
		
		for(Playable media: medialist) {
			System.out.println(media.toString());
			media.play();
		}
	}
}
