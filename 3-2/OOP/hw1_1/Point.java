//***************************
// 파일명: Point.java
// 작성자: 서형완
// 작성일: 2025/09/24
// 내용: 좌표를 설정하고 이동하는 클래스다. reset를 통해 처음 입력값으로 돌아갈 수 있다.
//***************************



package hw1_1;

public class Point implements Resettable {
	private double x;
	private double y;
	private double init_x;
	private double init_y;
	
	public Point(double x, double y) {
		this.x = x;
		this.y = y;
		init_x = x;
		init_y = y;
	}
	
	public void reset() {
		x = init_x;
		y = init_y;
	}
	
	public void move(double mx, double my) {
		x += mx;
		y += my;
	}
	
	public String toString() {
		String sx = String.valueOf(x);
		String sy = String.valueOf(y);
		return sx +", " + sy;
	}
}
