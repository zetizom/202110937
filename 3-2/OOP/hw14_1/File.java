//***************************

// 파일명: File.java

// 작성자: 서형완

// 작성일: 2025/11/26

// 내용: 원래 형태와 달리 Entry를 상속받습니다. 구현이 필요한 메서드만 작성하며, 불필요한 필드 중복을 제거

//***************************

package hw14_1;

public class File extends Entry {
    private int size;

    public File(String name, int size) {
        super(name); // 부모(Entry) 생성자 호출
        this.size = size;
    }

    @Override
    public int getSize() {
        return size;
    }

    @Override
    public void print() {
        for (int i = 0; i < depth; i++)
            System.out.print("\t");
        System.out.println("[File] " + name + ", Size: " + size);
    }
}
