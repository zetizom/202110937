//***************************

// 파일명: Directory.java

// 작성자: 서형완

// 작성일: 2025/11/26

// 내용: entries 리스트가 Object가 아닌 Entry를 담고, instanceof 검사가 대신 다형성으로 대체

//***************************


package hw14_1;

import java.util.ArrayList;
import java.util.List;

import java.util.ArrayList;
import java.util.List;

public class Directory extends Entry {
    // 수정: Object -> Entry (제네릭을 사용하여 타입 안정성 확보)
    private List<Entry> entries = new ArrayList<>();

    public Directory(String name) {
        super(name);
    }

    // 수정: instanceof 제거 및 다형성 활용
    @Override
    public void addEntry(Entry entry) {
        entries.add(entry);
        // 들어오는 요소가 File이든 Directory든 Entry의 setDepth를 호출하면 됨
        entry.setDepth(depth + 1); 
    }

    // 수정: instanceof 제거
    @Override
    public void removeEntry(Entry entry) {
        entries.remove(entry);
    }

    // 수정: 재귀적 호출 단순화
    @Override
    public int getSize() {
        int size = 0;
        for (Entry entry : entries) {
            // File이면 자신의 size, Directory면 자식들의 size 합을 반환 (다형성)
            size += entry.getSize(); 
        }
        return size;
    }

    @Override
    public void print() {
        for (int i = 0; i < depth; i++)
            System.out.print("\t");
        System.out.println("[Directory] " + name + ", Size: " + getSize());

        for (Entry entry : entries) {
            entry.print(); // 각 객체의 print() 위임
        }
    }
}
