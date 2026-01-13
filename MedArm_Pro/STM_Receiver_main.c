/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software
  * is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t uart_rx_byte;

#define PACKET_SIZE 15

// 패킷 수신 상태 머신
typedef enum {
    PKT_STATE_WAIT_HEADER1 = 0,
    PKT_STATE_WAIT_HEADER2,
    PKT_STATE_PAYLOAD,
    PKT_STATE_CHECKSUM
} PacketState_t;

volatile PacketState_t pkt_state = PKT_STATE_WAIT_HEADER1;
uint8_t pkt_buf[PACKET_SIZE];
volatile uint8_t pkt_index = 0;
volatile uint8_t pkt_ready = 0;  // 1이면 새 패킷 도착

// 서보 PWM CCR 범위 (0.5ms ~ 2.4ms 에 대응)
#define SERVO_CCR_MIN  47     // ≈ 0.5ms
#define SERVO_CCR_MAX  227    // ≈ 2.4ms

#define ANGLE1_START 90.0f		// 팔 모터 1의 시작 각도
#define ANGLE2_START 10.0f		// 팔 모터 2의 시작 각도
#define ANGLE3_START 170.0f		// 팔 모터 3의 시작 각도

#define FINGER_ANGLE1_START 90.0f	// 손가락 모터 1의 시작 각도
#define FINGER_ANGLE2_START 0.0f	// 손가락 모터 2의 시작 각도
#define FINGER_ANGLE3_START 0.0f	// 손가락 모터 3의 시작 각도
#define FINGER_ANGLE4_START 90.0f	// 손가락 모터 4의 시작 각도

// 현재 팔 서보 각도
volatile float servo1_angle_deg = ANGLE1_START;
volatile float servo2_angle_deg = ANGLE2_START;
volatile float servo3_angle_deg = ANGLE3_START;

// 현재 손가락 서보 각도
volatile float finger1_angle_deg = FINGER_ANGLE1_START;
volatile float finger2_angle_deg = FINGER_ANGLE2_START;
volatile float finger3_angle_deg = FINGER_ANGLE3_START;
volatile float finger4_angle_deg = FINGER_ANGLE4_START;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 10);
  return ch;
}

// 0 ~ 180 범위로 클램프
static float clamp_angle(float a, float start)
{
	float min = start * -1;
	float max = 180.0f - start;

    if (a < min) a = min;
    if (a > max) a = max;
    return a;
}

// 0도 ~ 180도를 CCR_MIN ~ CCR_MAX 로 선형 매핑
static uint16_t angle_to_ccr(float angle_deg)
{
    float a = angle_deg;	// clamp 함수 삭제

    // 0도 -> 0.0, 180도 -> 1.0
    float ratio = a / 180.0f;

    float ccr_f = SERVO_CCR_MIN + ratio * (SERVO_CCR_MAX - SERVO_CCR_MIN);

    if (ccr_f < SERVO_CCR_MIN) ccr_f = SERVO_CCR_MIN;
    if (ccr_f > SERVO_CCR_MAX) ccr_f = SERVO_CCR_MAX;

    return (uint16_t)(ccr_f + 0.5f); // 반올림
}

// 수신된 패킷 처리: 헤더/체크섬 검증 후 서보 각도 업데이트
void ProcessPacket(void)
{
    if (!pkt_ready)
        return;

    pkt_ready = 0;

    // 1) 헤더 확인
    if (pkt_buf[0] != 0xAA || pkt_buf[1] != 0x55)
    {
    	printf("ERR: Header Wrong! [0]=%02X, [1]=%02X\r\n", pkt_buf[0], pkt_buf[1]);
        return;
    }

    // 2) 체크섬 검증 (Byte 2~11 합의 LSB가 Byte 13)
    uint8_t calc_checksum = 0;
    for (int i = 2; i < PACKET_SIZE-1; i++)
    {
        calc_checksum += pkt_buf[i];
    }
    uint8_t recv_checksum = pkt_buf[PACKET_SIZE-1];

    if (calc_checksum != recv_checksum)
    {
        // 체크섬 오류 → 폐기
    	printf("ERR: Checksum Fail! Calc=%02X, Recv=%02X\r\n", calc_checksum, recv_checksum);
        return;
    }

    // 3) payload 파싱 (big-endian)
    // 여기서 angle*_int 는 "상대각도(Δθ) * 10" 이라고 가정
    int16_t angle1_int = (int16_t)((pkt_buf[2] << 8) | pkt_buf[3]);
    int16_t angle2_int = (int16_t)((pkt_buf[4] << 8) | pkt_buf[5]);
    int16_t angle3_int = (int16_t)((pkt_buf[6] << 8) | pkt_buf[7]);

    uint8_t finger1 = (uint8_t)pkt_buf[8];
    uint8_t finger2 = (uint8_t)pkt_buf[9];
    uint8_t finger3 = (uint8_t)pkt_buf[10];
    uint8_t finger4 = (uint8_t)pkt_buf[11];

    // 4) 상대각도(Δθ) 복원 (deg)
    float angle_rel1 = angle1_int / 10.0f;
    float angle_rel2 = angle2_int / 10.0f;
    float angle_rel3 = angle3_int / 10.0f;
    angle_rel3 = -angle_rel3;
    //angle_rel2 = -angle_rel2;

    // 센서 범위 제한
    float angle1_deg = clamp_angle(angle_rel1, ANGLE1_START);
    float angle2_deg = clamp_angle(angle_rel2, ANGLE2_START);
    float angle3_deg = clamp_angle(angle_rel3, ANGLE3_START);

    // 6) 현재 각도 상태 갱신
    servo1_angle_deg = angle1_deg + ANGLE1_START;
    servo2_angle_deg = angle2_deg + ANGLE2_START;
    servo3_angle_deg = angle3_deg + ANGLE3_START;

    // 7) 각도 → CCR 변환
    uint16_t ccr1 = angle_to_ccr(servo1_angle_deg);
    uint16_t ccr2 = angle_to_ccr(servo2_angle_deg);
    uint16_t ccr3 = angle_to_ccr(servo3_angle_deg);


//    static uint32_t last_servo_update = 0;
//    uint32_t now = HAL_GetTick();

//    if (now - last_servo_update >= 20) {
//    	last_servo_update = now;
    // 8) 서보 PWM 업데이트 (TIM3 CH1~CH3)
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr1);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccr2);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, ccr3);

    // === 손가락 서보 제어 (스위치 모멘터리 방식) ===
    // fingerX == 1 → "스위치/센서 눌림 = 접힘"
    // fingerX == 0 → "스위치/센서 안 눌림 = 초기자세"

    // 손가락 1 (엄지) : 초기 90도, 눌리면 0도
    if (finger1)
        finger1_angle_deg = 0.0f;     // 접힘
    else
        finger1_angle_deg = 90.0f;    // 초기

    uint16_t finger_ccr1 = angle_to_ccr(finger1_angle_deg);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, finger_ccr1);

    // 손가락 2 (검지) : 초기 0도, 눌리면 90도
    if (finger2)
        finger2_angle_deg = 90.0f;    // 접힘
    else
        finger2_angle_deg = 0.0f;     // 초기

    uint16_t finger_ccr2 = angle_to_ccr(finger2_angle_deg);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, finger_ccr2);

    // 손가락 3 (중지) : 초기 0도, 눌리면 90도
    if (finger3)
        finger3_angle_deg = 90.0f;    // 접힘
    else
        finger3_angle_deg = 0.0f;     // 초기

    uint16_t finger_ccr3 = angle_to_ccr(finger3_angle_deg);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, finger_ccr3);

    // 손가락 4 (약지+새끼) : 초기 90도, 눌리면 0도
    if (finger4)
        finger4_angle_deg = 0.0f;     // 접힘
    else
        finger4_angle_deg = 90.0f;    // 초기

    uint16_t finger_ccr4 = angle_to_ccr(finger4_angle_deg);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, finger_ccr4);
    //}

    // === [Full E2E] ACK 전송 (센서 STM32로 RTT 측정용) ===
    uint8_t ack = 0x06;  // ACK 문자
    HAL_UART_Transmit(&huart2, &ack, 1, 5);  // 5ms 타임아웃

//    printf("RX_DEBUG: S1=%.1f, S2=%.1f, S3=%.1f\r\n",
//               servo1_angle_deg, servo2_angle_deg, servo3_angle_deg);


}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  // TIM3 PWM 시작 (CH1, CH2, CH3)
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  // TIM4 PWM 시작 (CH1, CH2, CH3, CH4)
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  // 현재 각도(절대각도)에 맞춰 초기 CCR 세팅
  uint16_t ccr1_init = angle_to_ccr(servo1_angle_deg);  // 90도
  uint16_t ccr2_init = angle_to_ccr(servo2_angle_deg);  // 10도
  uint16_t ccr3_init = angle_to_ccr(servo3_angle_deg);  // 170도

  uint16_t finger_ccr1_init = angle_to_ccr(finger1_angle_deg);  // 90도
  uint16_t finger_ccr2_init = angle_to_ccr(finger2_angle_deg);  // 0도
  uint16_t finger_ccr3_init = angle_to_ccr(finger3_angle_deg);  // 0도
  uint16_t finger_ccr4_init = angle_to_ccr(finger4_angle_deg);  // 90도

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccr1_init);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccr2_init);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, ccr3_init);

  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, finger_ccr1_init);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, finger_ccr2_init);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, finger_ccr3_init);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, finger_ccr4_init);

  // UART1 인터럽트 수신 시작 (1바이트씩)
  HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	 ProcessPacket();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 887-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1894-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 887-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1894-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// UART 수신 완료 콜백: USART1으로 들어온 바이트를 패킷 상태 머신에 투입
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
    	// 디버깅용 -> USART2에 데이터 수신 시 LED TOGGLE
    	 HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

        uint8_t b = uart_rx_byte;

        switch (pkt_state)
        {
        case PKT_STATE_WAIT_HEADER1:
            if (b == 0xAA)
            {
                pkt_buf[0] = b;
                pkt_state  = PKT_STATE_WAIT_HEADER2;
            }
            break;

        case PKT_STATE_WAIT_HEADER2:
            if (b == 0x55)
            {
                pkt_buf[1] = b;
                pkt_index  = 2;
                pkt_state  = PKT_STATE_PAYLOAD;
            }
            else
            {
                // 첫 바이트만 0xAA 맞고 두 번째가 아니면 다시 처음부터
                pkt_state = PKT_STATE_WAIT_HEADER1;
            }
            break;

        case PKT_STATE_PAYLOAD:
            pkt_buf[pkt_index++] = b;
            if (pkt_index >= PACKET_SIZE)
            {
                pkt_ready = 1;
                pkt_state = PKT_STATE_WAIT_HEADER1;
            }
            break;

        default:
            pkt_state = PKT_STATE_WAIT_HEADER1;
            break;
        }

        // 다음 바이트 수신 예약
        HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
