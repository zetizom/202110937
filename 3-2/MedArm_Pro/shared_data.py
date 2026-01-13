"""
shared_data.py - 공유 메모리 구조 정의

프로세스 간 제로카피 실시간 데이터 공유를 위한 모듈
"""

import struct
from multiprocessing import shared_memory
import time
import atexit

# 공유 메모리 이름
SHM_NAME = "medarm_data_v2"

# 시작할 때 자동 정리
def _cleanup_shm():
    try:
        from multiprocessing.shared_memory import SharedMemory
        s = SharedMemory(SHM_NAME)
        s.close()
        s.unlink()
    except:
        pass

# 프로그램 종료 시 자동 정리
atexit.register(_cleanup_shm)

# 데이터 구조 (총 44바이트)
# - angle1, angle2, angle3: float (4바이트 × 3 = 12바이트)
# - finger1~4: uint8 (1바이트 × 4 = 4바이트)
# - timestamp: double (8바이트) - 패킷 수신 시각
# - latency_us: uint32 (4바이트) - 내부 처리 지연시간 (마이크로초)
# - e2e_latency_ms: float (4바이트) - End-to-End 지연시간 (밀리초)
# - packet_count: uint32 (4바이트) - 총 수신 패킷 수
# - hz: float (4바이트) - 현재 수신율
# - is_connected: uint8 (1바이트) - 연결 상태
# - padding: 3바이트 (정렬용)

STRUCT_FORMAT = "=3f4BdIfIfB3x"
STRUCT_SIZE = struct.calcsize(STRUCT_FORMAT)  # 44바이트


class SharedData:
    """공유 메모리 데이터 클래스"""
    
    def __init__(self):
        self.angle1 = 0.0
        self.angle2 = 0.0
        self.angle3 = 0.0
        self.finger1 = 0
        self.finger2 = 0
        self.finger3 = 0
        self.finger4 = 0
        self.timestamp = 0.0
        self.latency_us = 0
        self.e2e_latency_ms = 0.0
        self.packet_count = 0
        self.hz = 0.0
        self.is_connected = 0
    
    def pack(self) -> bytes:
        """구조체를 바이트로 변환"""
        return struct.pack(
            STRUCT_FORMAT,
            self.angle1, self.angle2, self.angle3,
            self.finger1, self.finger2, self.finger3, self.finger4,
            self.timestamp,
            self.latency_us,
            self.e2e_latency_ms,
            self.packet_count,
            self.hz,
            self.is_connected
        )
    
    @classmethod
    def unpack(cls, data: bytes) -> 'SharedData':
        """바이트를 구조체로 변환"""
        values = struct.unpack(STRUCT_FORMAT, data)
        obj = cls()
        obj.angle1, obj.angle2, obj.angle3 = values[0:3]
        obj.finger1, obj.finger2, obj.finger3, obj.finger4 = values[3:7]
        obj.timestamp = values[7]
        obj.latency_us = values[8]
        obj.e2e_latency_ms = values[9]
        obj.packet_count = values[10]
        obj.hz = values[11]
        obj.is_connected = values[12]
        return obj


class SharedMemoryWriter:
    """공유 메모리 쓰기 클래스 (수신 프로세스용)"""
    
    def __init__(self):
        # 기존 공유 메모리 정리 (Windows 호환)
        try:
            old_shm = shared_memory.SharedMemory(name=SHM_NAME)
            old_shm.close()
            old_shm.unlink()
            time.sleep(0.1)
        except:
            pass
        
        # 새로 생성 (재시도 포함)
        for _ in range(3):
            try:
                self.shm = shared_memory.SharedMemory(
                    name=SHM_NAME,
                    create=True,
                    size=STRUCT_SIZE
                )
                break
            except FileExistsError:
                time.sleep(0.2)
        else:
            # 그래도 안 되면 기존 것 사용
            self.shm = shared_memory.SharedMemory(name=SHM_NAME)
        
        self.data = SharedData()
        print(f"[SharedMemory] Writer created: {SHM_NAME} ({STRUCT_SIZE} bytes)")
    
    def write(self, angle1, angle2, angle3, f1, f2, f3, f4, latency_us=0):
        """데이터 쓰기"""
        self.data.angle1 = angle1
        self.data.angle2 = angle2
        self.data.angle3 = angle3
        self.data.finger1 = f1
        self.data.finger2 = f2
        self.data.finger3 = f3
        self.data.finger4 = f4
        self.data.timestamp = time.time()
        self.data.latency_us = latency_us
        self.data.packet_count += 1
        self.data.is_connected = 1
        
        self.shm.buf[:STRUCT_SIZE] = self.data.pack()
    
    def update_hz(self, hz: float):
        """수신율 업데이트"""
        self.data.hz = hz
        self.shm.buf[:STRUCT_SIZE] = self.data.pack()
    
    def update_e2e_latency(self, e2e_ms: float):
        """End-to-End 지연시간 업데이트"""
        self.data.e2e_latency_ms = e2e_ms
        self.shm.buf[:STRUCT_SIZE] = self.data.pack()
    
    def set_disconnected(self):
        """연결 끊김 상태 설정"""
        self.data.is_connected = 0
        self.shm.buf[:STRUCT_SIZE] = self.data.pack()
    
    def close(self):
        """정리"""
        self.shm.close()
        try:
            self.shm.unlink()
        except:
            pass


class SharedMemoryReader:
    """공유 메모리 읽기 클래스 (대시보드/로봇 전송용)"""
    
    def __init__(self, timeout=5.0):
        self.shm = None
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            try:
                self.shm = shared_memory.SharedMemory(name=SHM_NAME)
                print(f"[SharedMemory] Reader connected: {SHM_NAME}")
                break
            except FileNotFoundError:
                time.sleep(0.1)
        
        if self.shm is None:
            raise RuntimeError(f"공유 메모리 '{SHM_NAME}' 연결 실패 (timeout: {timeout}s)")
    
    def read(self) -> SharedData:
        """데이터 읽기"""
        data = bytes(self.shm.buf[:STRUCT_SIZE])
        return SharedData.unpack(data)
    
    def close(self):
        """정리"""
        if self.shm:
            self.shm.close()


if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1 and sys.argv[1] == "read":
        reader = SharedMemoryReader()
        try:
            while True:
                data = reader.read()
                print(f"A=({data.angle1:6.1f}, {data.angle2:6.1f}, {data.angle3:6.1f}) | "
                      f"F=[{data.finger1},{data.finger2},{data.finger3},{data.finger4}] | "
                      f"E2E={data.e2e_latency_ms:.1f}ms | {data.hz:.0f}Hz | #{data.packet_count}")
                time.sleep(0.1)
        except KeyboardInterrupt:
            reader.close()
    else:
        writer = SharedMemoryWriter()
        try:
            import math
            t = 0
            while True:
                angle1 = 45 * math.sin(t)
                angle2 = 30 * math.cos(t)
                angle3 = 20 * math.sin(t * 0.5)
                f1 = 1 if math.sin(t * 2) > 0 else 0
                f2 = 1 if math.cos(t * 2) > 0 else 0
                
                writer.write(angle1, angle2, angle3, f1, f2, 0, 1, latency_us=50)
                writer.update_hz(50.0)
                writer.update_e2e_latency(12.5)
                
                t += 0.1
                time.sleep(0.02)
        except KeyboardInterrupt:
            writer.close()
            print("\n[Test] 종료")