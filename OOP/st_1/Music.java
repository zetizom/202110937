package st_1;

public class Music extends Media implements Playable {
	private String artist;

	
	public Music(String title, String artist) {
		super(title);
		this.artist = artist;		
	}
	
	@Override
	public void play() {
		System.out.println(artist +"의 " + title +"이(가) 재생됩니다.");
	}
	
	@Override
	public String toString() {
		return super.toString().replace("[미디어]", "음악") + " 작곡가: " + artist;
		
	}
}
