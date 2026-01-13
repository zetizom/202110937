//***************************

// 파일명: Entry.java

// 작성자: 서형완

// 작성일: 2025/11/26

// 내용: File과 Directory의 공통 부모 클래스로써, MenuComponent 역할을 수행

//***************************

package hw14_1;

public abstract class Entry {
    protected String name;
    protected int depth = 0;

    public Entry(String name) {
        this.name = name;
    }

    // 공통 메서드 정의
    public abstract int getSize();
    public abstract void print();

    public String getName() { return name; }
    
    // 계층 구조 표현을 위한 depth 설정 (공통)
    public void setDepth(int depth) {
        this.depth = depth;
    }

    // Composite(Directory)에서만 쓰이지만 투명성을 위해 여기에 정의
    // 문제의 요구사항: "사용하지 않는 메소드는 예외 처리 안해도 됨"에 따라 빈 구현으로 둠
    public void addEntry(Entry entry) { }
    public void removeEntry(Entry entry) { }
}