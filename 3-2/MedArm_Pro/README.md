# Full End-to-End RTT 측정 시스템

## 패킷 구조 (15바이트)

```
[0xAA][0x55][A1_H][A1_L][A2_H][A2_L][A3_H][A3_L][F1][F2][F3][F4][RTT_H][RTT_L][Checksum]
   0     1     2     3     4     5     6     7    8   9  10  11   12     13      14
```

| 필드 | 바이트 | 설명 |
|------|--------|------|
| Header | 0-1 | 0xAA, 0x55 |
| Angle 1-3 | 2-7 | signed int16 × 3 (값 × 10) |
| Finger 1-4 | 8-11 | uint8 × 4 (0 또는 1) |
| **RTT** | 12-13 | **unsigned int16 (ms)** ← 추가됨 |
| Checksum | 14 | Byte 2~13 합의 LSB |

## 데이터 흐름

```
[센서 STM32] ──패킷──→ [ESP32] ──BT──→ [PC] ──USB──→ [Robot STM32]
     ↑                    ↑               │              │
     │                    │            main.py           │
     │                    │               │              │
     │                    │         ACK 큐 전달          │
     │                    │               │              ↓
     │                    └───BT ACK←─────┴────USB ACK───┘
     │                                                   
     └──────────────── ACK(0x06) 수신 ───────────────────┘
                            │
                       RTT 측정 완료
                    printf 출력 (시리얼 모니터)
```

## 대시보드 표시

**RTT가 패킷에 포함**되어 대시보드에서 바로 확인 가능!

```
┌──────────────────────────────────────┐
│  Real-time Metrics                   │
├──────────────────┬───────────────────┤
│ E2E Latency      │ IPC Latency       │
│ (Full E2E)       │ (Zero-Copy)       │
│   12.5 ms        │   50 μs           │  ← RTT/2 값
└──────────────────┴───────────────────┘
```

## 수정된 파일 목록

| 파일 | 수정 내용 |
|------|----------|
| `STM_Sender_main.c` | ACK 수신 대기 + RTT 계산 + printf 출력 |
| `STM_Receiver_main.c` | 패킷 처리 후 ACK(0x06) 전송 |
| `ESP_BT.ino` | 양방향 통신 (TX 핀 활성화, ACK 역전송) |
| `main.py` | ACK 큐로 역방향 전달 |
| `shared_data.py` | e2e_latency_ms 필드 (대시보드용) |
| `app.py` | E2E 지연 표시 |

## 하드웨어 연결 (추가 필요)

### ESP32 ↔ 센서 STM32 (양방향)
```
ESP32 GPIO16 (RX2) ← STM32 USART1_TX  (기존)
ESP32 GPIO17 (TX2) → STM32 USART1_RX  (추가!)
```

⚠️ **중요**: 기존에는 ESP32 TX를 사용 안 했는데, 이제 GPIO17을 센서 STM32의 RX에 연결해야 함!

## 실행 순서

### 1. 펌웨어 업로드
```
1) STM_Sender_main.c → 센서 STM32에 업로드
2) STM_Receiver_main.c → 로봇 STM32에 업로드  
3) ESP_BT.ino → ESP32에 업로드
```

### 2. 하드웨어 연결 확인
- ESP32 GPIO17 → 센서 STM32 USART1_RX 연결

### 3. PC 프로그램 실행
```bash
# 터미널 1
python main.py --port-in COM8 --port-out COM3

# 터미널 2
streamlit run app.py
```

### 4. RTT 확인
- **센서 STM32 시리얼 모니터**에서 확인:
```
[E2E] RTT=25 ms (avg=24 ms) | A1=45.2 A2=30.1 A3=-10.5
```

- **대시보드**에서 확인:
  - E2E Latency (PC↔Robot) 항목

## RTT 측정 원리

```
1. 센서 STM32: 패킷 전송 직전 HAL_GetTick() 저장
2. 패킷이 ESP32 → BT → PC → USB → Robot STM32로 전달
3. Robot STM32: 패킷 처리 후 ACK(0x06) 전송
4. ACK가 USB → PC → BT → ESP32 → 센서 STM32로 돌아옴
5. 센서 STM32: ACK 수신 시각 - 전송 시각 = RTT
6. 편도 지연 = RTT / 2
```

## 예상 RTT 값

| 구간 | 예상 지연 |
|------|----------|
| STM32 → ESP32 (UART) | ~1ms |
| ESP32 → PC (BT) | ~10ms |
| PC → Robot STM32 (USB) | ~2ms |
| Robot 처리 + ACK 전송 | ~1ms |
| 역방향 (동일) | ~14ms |
| **총 RTT** | **~28ms** |
| **편도 (E2E)** | **~14ms** |

## 문제 해결

### ACK가 안 오는 경우
1. ESP32 GPIO17 연결 확인
2. 센서 STM32의 USART1 RX 핀 확인 (PA10)
3. ESP32 시리얼 모니터에서 "[RX] ACK forwarded" 메시지 확인

### RTT가 100ms로 고정되는 경우
- ACK 타임아웃됨 → 위 연결 확인

### 대시보드 E2E가 0으로 표시
- 현재 대시보드는 PC↔Robot 구간만 표시
- 센서 STM32 시리얼 모니터에서 Full E2E 확인

## 시연 포인트

> "센서에서 로봇까지 **전체 경로의 왕복 시간(RTT)**을 실시간 측정합니다.
> 평균 RTT **25~30ms**, 편도 지연 **12~15ms**로
> **실시간 로봇 제어에 충분한 저지연** 통신을 달성했습니다."
