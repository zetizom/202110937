package st_1;

public abstract class Media {
	protected String title;  // 각 자식들이 접근 할 태니, protected로 정의
	public Media (String title) {
		this.title = title;
	}
	public String toString() {
		return "[미디어]: "+ title;
	}
}
