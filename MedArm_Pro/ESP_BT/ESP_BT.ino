#include <BluetoothSerial.h>

// ====== Bluetooth SPP 객체 ======
BluetoothSerial SerialBT;

// ====== STM32: STM32 → ESP32 하드웨어 UART 수신 코드 ======

// STM32 USART1_TX → ESP32 GPIO16 (RX2)
#define STM_RX_PIN 16   // ESP32가 STM32로부터 받는 RX 핀
#define STM_TX_PIN 17   // ESP32가 STM32로 보내는 TX 핀 (Full E2E용)

const uint8_t PACKET_SIZE = 15;

// UART2(Serial2) 사용
HardwareSerial stmSerial(2);  // UART2 인스턴스

void setup() {
  // PC와 USB로 연결된 기본 시리얼 (디버그용)
  Serial.begin(115200);
  delay(500);

  // 블루투스 시리얼 시작 (PC에서 보일 이름 설정)
  SerialBT.begin("ESP32_Encoder");  // Windows에서 이 이름으로 검색됨

  // STM32와 동일 Baudrate, 8N1, RX/TX 핀 지정
  stmSerial.begin(115200, SERIAL_8N1, STM_RX_PIN, STM_TX_PIN);

  Serial.println("ESP32: Full E2E mode - Bidirectional communication enabled");
  SerialBT.println("ESP32: Full E2E mode started");
}

void loop() {
  static uint8_t  state = 0;
  static uint8_t  buf[PACKET_SIZE];
  static uint8_t  index = 0;

  while (stmSerial.available()) {
    uint8_t b = stmSerial.read();

    switch (state) {
      case 0: // 헤더 0xAA 기다리는 상태
        if (b == 0xAA) {
          buf[0] = b;
          state = 1;
        }
        break;

      case 1: // 헤더 0x55 기다리는 상태
        if (b == 0x55) {
          buf[1] = b;
          state = 2;
          index = 2;
        } else {
          state = 0; // 다시 처음부터
        }
        break;

      case 2: // 나머지 13바이트 (Payload, Checksum)
        buf[index++] = b;

        if (index >= PACKET_SIZE) {
          uint8_t calc_cs = 0;
          for (int i = 2; i < PACKET_SIZE -1; i++) {
            calc_cs += buf[i];
          }
          uint8_t recv_cs = buf[PACKET_SIZE - 1];

          if (calc_cs == recv_cs) {
            // 체크섬 OK → BT로 패킷 전송
            SerialBT.write(buf, PACKET_SIZE);
            
            // 디버그 (간략하게)
            // Serial.print("[TX] Packet sent to BT, size=");
            // Serial.println(PACKET_SIZE);
          
          } else {
            Serial.print("Checksum error (calc=");
            Serial.print(calc_cs, HEX);
            Serial.print(", recv=");
            Serial.print(recv_cs, HEX);
            Serial.println(")");
          }

          // 다음 패킷 기다리기 위해 상태 초기화
          state = 0;
        }
        break;
    }
  }

  // === 2. BT → ESP32 → STM32 (ACK 역전송) ===
  // PC에서 ACK(0x06)를 보내면 STM32로 전달
  while (SerialBT.available()) {
    uint8_t ack = SerialBT.read();
    
    // ACK 바이트(0x06)면 STM32로 전달
    if (ack == 0x06) {
      stmSerial.write(ack);
      // Serial.println("[RX] ACK forwarded to STM32");
    }
    // 다른 데이터는 무시 (또는 필요시 처리)
  }
}
