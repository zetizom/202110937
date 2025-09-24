package st_1;

public class Movie extends Media implements Playable{
	private String director;
	
	
	public Movie(String title, String director) {
		super(title);
		this.director = director;
	}
	
	@Override
	public void play() {
		System.out.println(director +"감독의 영화  " + title +"이(가) 상영됩니다.");
	}
	
	@Override
	public String toString() {
		return super.toString().replace("[미디어]", "영화") + " (감독: " + director + ")";
	}
}
