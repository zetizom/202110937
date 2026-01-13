"""
main.py - MedArm 통합 제어 시스템

구조:
  [ESP32 BT] → COM_IN → Receiver Process
                              ↓
                       Shared Memory (제로카피)
                              ↓
                    ├─ Robot Sender Process → COM_OUT → [Robot STM32]
                    └─ Dashboard (별도 실행: streamlit run app.py)

실행 방법:
  python main.py

옵션:
  --port-in COM8      : 블루투스 수신 포트 (기본: COM8)
  --port-out COM6     : 로봇 전송 포트 (기본: COM6)
  --baud 115200       : Baudrate (기본: 115200)
  --no-robot          : 로봇 전송 비활성화 (테스트용)
"""

import serial
import struct
import time
import argparse
from multiprocessing import Process, Event, Queue, shared_memory
from shared_data import SharedMemoryWriter, SharedMemoryReader, SharedData, STRUCT_SIZE

# === 설정 ===
PACKET_SIZE = 15  # [Full E2E] 15바이트로 변경
HEADER = bytes([0xAA, 0x55])


def find_packet(ser) -> bytes | None:
    """
    시리얼 스트림에서 유효한 패킷 찾기
    헤더(0xAA, 0x55)를 찾고 나머지 13바이트를 읽어 15바이트 패킷 반환
    """
    while True:
        b = ser.read(1)
        if not b:
            return None  # timeout
        
        if b[0] == 0xAA:
            b2 = ser.read(1)
            if not b2:
                return None
            
            if b2[0] == 0x55:
                rest = ser.read(PACKET_SIZE - 2)
                if len(rest) != PACKET_SIZE - 2:
                    return None
                return b + b2 + rest


def validate_and_parse(pkt: bytes) -> dict | None:
    """
    패킷 검증 및 파싱
    - 체크섬 확인
    - 각도/손가락/RTT 값 추출
    
    패킷 구조 (15바이트):
    [0xAA][0x55][A1_H][A1_L][A2_H][A2_L][A3_H][A3_L][F1][F2][F3][F4][RTT_H][RTT_L][Checksum]
       0     1     2     3     4     5     6     7    8   9  10  11   12     13      14
    """
    if len(pkt) != PACKET_SIZE:
        return None
    
    if pkt[0] != 0xAA or pkt[1] != 0x55:
        return None
    
    payload = pkt[2:PACKET_SIZE-1]  # 12바이트 (인덱스 2~13)
    recv_checksum = pkt[PACKET_SIZE-1]
    
    calc_checksum = sum(payload) & 0xFF
    if calc_checksum != recv_checksum:
        return None
    
    # Big-endian: 3개의 signed int16 + 4개의 uint8 + 1개의 unsigned int16 (RTT)
    angle1_int, angle2_int, angle3_int, f1, f2, f3, f4, rtt = struct.unpack(">hhhBBBBH", payload)
    
    return {
        "angle1": angle1_int / 10.0,
        "angle2": angle2_int / 10.0,
        "angle3": angle3_int / 10.0,
        "finger1": f1,
        "finger2": f2,
        "finger3": f3,
        "finger4": f4,
        "rtt_ms": rtt,  # [Full E2E] RTT 값 (ms)
        "raw_packet": pkt
    }


def receiver_process(port_in: str, baud: int, ack_queue, stop_event: Event):
    """
    수신 프로세스: BT로부터 패킷 수신 → 공유 메모리에 쓰기
    + ACK 큐에서 받은 ACK를 BT로 역전송 (Full E2E용)
    """
    print(f"[Receiver] Starting on {port_in} @ {baud}bps")
    
    # 공유 메모리 Writer 생성
    shm_writer = SharedMemoryWriter()
    
    ser = None
    try:
        ser = serial.Serial(port_in, baud, timeout=0.05)  # 50ms 타임아웃 (빠른 ACK 전송)
        time.sleep(1.0)  # 포트 안정화
        print(f"[Receiver] Port opened: {port_in}")
        
        # Hz 계산용 변수
        packet_times = []
        last_hz_update = time.time()
        
        while not stop_event.is_set():
            # === ACK 역전송 (큐에 있으면) ===
            try:
                while not ack_queue.empty():
                    ack_data = ack_queue.get_nowait()
                    if ack_data:
                        ser.write(ack_data)  # BT로 ACK 전송
            except:
                pass
            
            recv_start = time.perf_counter()
            
            pkt = find_packet(ser)
            if pkt is None:
                continue
            
            recv_end = time.perf_counter()
            
            data = validate_and_parse(pkt)
            if data is None:
                print("[Receiver] Invalid packet (checksum error)")
                continue
            
            # 지연시간 계산 (마이크로초)
            latency_us = int((recv_end - recv_start) * 1_000_000)
            
            # [Full E2E] RTT에서 E2E 계산 (편도 = RTT / 2)
            rtt_ms = data.get("rtt_ms", 0)
            e2e_ms = rtt_ms / 2.0 if rtt_ms > 0 else 0.0
            
            # 공유 메모리에 쓰기
            shm_writer.write(
                data["angle1"], data["angle2"], data["angle3"],
                data["finger1"], data["finger2"], data["finger3"], data["finger4"],
                latency_us=latency_us
            )
            
            # [Full E2E] E2E 지연시간 업데이트
            if e2e_ms > 0:
                shm_writer.update_e2e_latency(e2e_ms)
            
            # Hz 계산 (1초마다 업데이트)
            now = time.time()
            packet_times.append(now)
            packet_times = [t for t in packet_times if now - t < 1.0]  # 최근 1초만 유지
            
            if now - last_hz_update > 0.2:  # 200ms마다 Hz 업데이트
                hz = len(packet_times)
                shm_writer.update_hz(hz)
                last_hz_update = now
            
            # 디버그 출력 (10패킷마다)
            if shm_writer.data.packet_count % 10 == 0:
                print(f"[Receiver] #{shm_writer.data.packet_count:5d} | "
                      f"A=({data['angle1']:6.1f}, {data['angle2']:6.1f}, {data['angle3']:6.1f}) | "
                      f"F=[{data['finger1']},{data['finger2']},{data['finger3']},{data['finger4']}] | "
                      f"{len(packet_times):.0f}Hz | RTT={rtt_ms}ms E2E={e2e_ms:.1f}ms")
    
    except serial.SerialException as e:
        print(f"[Receiver] Serial error: {e}")
        shm_writer.set_disconnected()
    except KeyboardInterrupt:
        pass
    finally:
        if ser and ser.is_open:
            ser.close()
        shm_writer.set_disconnected()
        time.sleep(0.1)
        shm_writer.close()
        print("[Receiver] Stopped")


def robot_sender_process(port_out: str, baud: int, ser_in_queue, stop_event: Event):
    """
    로봇 전송 프로세스: 공유 메모리에서 읽기 → 로봇 STM32로 전송
    + Robot에서 받은 ACK를 ser_in_queue에 넣어서 BT로 역전송
    """
    print(f"[RobotSender] Starting on {port_out} @ {baud}bps")
    
    # 공유 메모리 Reader 연결 (최대 10초 대기)
    try:
        shm_reader = SharedMemoryReader(timeout=10.0)
    except RuntimeError as e:
        print(f"[RobotSender] {e}")
        return
    
    ser = None
    try:
        ser = serial.Serial(port_out, baud, timeout=0.02)  # 20ms 타임아웃
        time.sleep(1.0)
        print(f"[RobotSender] Port opened: {port_out}")
        
        last_packet_count = 0
        
        while not stop_event.is_set():
            data = shm_reader.read()
            
            # 새 패킷이 있을 때만 전송
            if data.packet_count > last_packet_count and data.is_connected:
                last_packet_count = data.packet_count
                
                # 패킷 재구성
                angle1_int = int(data.angle1 * 10)
                angle2_int = int(data.angle2 * 10)
                angle3_int = int(data.angle3 * 10)
                rtt_dummy = 0
                
                payload = struct.pack(">hhhBBBBH",
                    angle1_int, angle2_int, angle3_int,
                    data.finger1, data.finger2, data.finger3, data.finger4, rtt_dummy
                )
                
                checksum = sum(payload) & 0xFF
                packet = bytes([0xAA, 0x55]) + payload + bytes([checksum])
                
                try:
                    ser.write(packet)
                    ser.flush()

                    # 디버그 출력 (10패킷마다)
                    if last_packet_count % 10 == 0:
                        print(f"[RobotSender] TX #{last_packet_count} | "
                              f"A=({data.angle1:.1f}, {data.angle2:.1f}, {data.angle3:.1f}) | "
                              f"F=[{data.finger1},{data.finger2},{data.finger3},{data.finger4}]")

                except serial.SerialException as e:
                    print(f"[RobotSender] TX error: {e}")
                    continue
                
                # Robot에서 ACK(0x06) 수신 대기
                try:
                    ack_byte = ser.read(1)
                    if ack_byte and ack_byte[0] == 0x06:
                        # ACK를 큐에 넣어서 BT로 역전송
                        try:
                            ser_in_queue.put_nowait(ack_byte)
                        except:
                            pass
                    
                    # 나머지 응답(printf 등) 버퍼 비우기
                    if ser.in_waiting > 0:
                        ser.read(ser.in_waiting)
                        
                except serial.SerialException:
                    pass
            
            time.sleep(0.002)  # 2ms 폴링
    
    except serial.SerialException as e:
        print(f"[RobotSender] Serial error: {e}")
    except KeyboardInterrupt:
        pass
    finally:
        if ser and ser.is_open:
            ser.close()
        shm_reader.close()
        print("[RobotSender] Stopped")


def main():
    parser = argparse.ArgumentParser(description="MedArm Control System")
    parser.add_argument("--port-in", default="COM8", help="BT 수신 포트 (기본: COM8)")
    parser.add_argument("--port-out", default="COM6", help="로봇 전송 포트 (기본: COM6)")
    parser.add_argument("--baud", type=int, default=115200, help="Baudrate (기본: 115200)")
    parser.add_argument("--no-robot", action="store_true", help="로봇 전송 비활성화")
    args = parser.parse_args()
    
    print("=" * 60)
    print("  MedArm Control System (Full E2E)")
    print("=" * 60)
    print(f"  BT Input  : {args.port_in} @ {args.baud}bps")
    print(f"  Robot Out : {args.port_out} @ {args.baud}bps" + (" (disabled)" if args.no_robot else ""))
    print(f"  Dashboard : Run separately with 'streamlit run app.py'")
    print("=" * 60)
    print()
    
    stop_event = Event()
    
    # ACK 역전송용 큐 (Robot → PC → BT → Sensor)
    ack_queue = Queue()
    
    # 프로세스 생성
    processes = []
    
    # 수신 프로세스 (ACK 역전송 포함)
    recv_proc = Process(
        target=receiver_process,
        args=(args.port_in, args.baud, ack_queue, stop_event),
        name="Receiver"
    )
    processes.append(recv_proc)
    
    # 로봇 전송 프로세스 (옵션)
    if not args.no_robot:
        robot_proc = Process(
            target=robot_sender_process,
            args=(args.port_out, args.baud, ack_queue, stop_event),
            name="RobotSender"
        )
        processes.append(robot_proc)
    
    # 프로세스 시작
    for p in processes:
        p.start()
        print(f"[Main] Process started: {p.name}")
    
    print()
    print("[Main] Press Ctrl+C to stop...")
    print()
    
    try:
        # 메인 프로세스는 대기
        while True:
            time.sleep(1.0)
            
            # 프로세스 상태 확인
            for p in processes:
                if not p.is_alive():
                    print(f"[Main] Process died: {p.name}")
    
    except KeyboardInterrupt:
        print()
        print("[Main] Shutting down...")
    
    # 정리
    stop_event.set()
    
    for p in processes:
        p.join(timeout=3.0)
        if p.is_alive():
            p.terminate()
            p.join(timeout=1.0)
    
    print("[Main] All processes stopped")
    print("[Main] Goodbye!")


if __name__ == "__main__":
    main()
